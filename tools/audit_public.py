#!/usr/bin/env python3
"""Audit the approved publication tree and optionally its reachable Git history.

Diagnostics identify rules and paths, never matched private values.
This is a content check, not an intellectual-property determination.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = 'PUBLIC-MANIFEST.json'
MAX_BYTES = 256 * 1024
SUFFIXES = {'.md', '.json', '.c', '.h', '.java', '.py', '.yml'}
NAMED = {'LICENSE', '.gitignore', '.gitattributes'}
RULES = {
    'personal-filesystem-path': re.compile(r'/(?:Users|home|private|Volumes)/|[A-Za-z]:\\Users\\'),
    'vehicle-identifier': re.compile(r'(?<![A-Za-z0-9])[A-HJ-NPR-Z0-9]{17}(?![A-Za-z0-9])'),
    'vehicle-serial': re.compile(r'\b(?:VWZ|SKZ|AUZ)[A-Z0-9]{7,20}\b'),
    'hardware-address': re.compile(r'(?i)\b(?:[0-9a-f]{2}[:-]){5}[0-9a-f]{2}\b'),
    'coordinate-pair': re.compile(r'(?<![\w.])[+-]?(?:[0-8]?\d)\.\d{4,}\s*,\s*[+-]?(?:1[0-7]\d|[0-9]?\d)\.\d{4,}(?![\w.])'),
    'credential-token': re.compile(r'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}|AKIA[A-Z0-9]{16})\b'),
    'private-key': re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
    'embedded-credential': re.compile(r'''(?i)\b(?:password|api_key|access_token|secret)\s*[:=]\s*["'][^"'\n]{8,}["']'''),
    'vendor-java-package': re.compile(r'(?m)^\s*package\s+(?:de\.vw|org\.dsi)\.'),
    'decompiled-listing': re.compile(r'(?m)^\s*\d+:\s+(?:aload_\d|invokevirtual|invokeinterface|invokespecial|bipush|sipush)\b'),
    'decompiler-banner': re.compile(r'(?i)decompiled (?:by|with) (?:CFR|Procyon|FernFlower)'),
}
EMAIL = re.compile(r'\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b')


def path_ok(name):
    path = PurePosixPath(name)
    return (not path.is_absolute() and '..' not in path.parts and
            not any(p in {'.git', 'build', 'results', 'captures', 'dump', 'decompiled', 'required-files', 'pics'}
                    for p in path.parts) and
            (path.suffix in SUFFIXES or path.name in NAMED))


def inspect(name, raw, check_path=True):
    findings = []
    if check_path and not path_ok(name):
        findings.append({'path': name, 'rule': 'unapproved-path-or-file-type'})
    if len(raw) > MAX_BYTES:
        findings.append({'path': name, 'rule': 'oversized-file'})
        return findings
    try:
        text = raw.decode('utf-8')
    except UnicodeError:
        return findings + [{'path': name, 'rule': 'binary-or-non-utf8'}]
    if any(ord(c) < 32 and c not in '\t\r\n' for c in text):
        findings.append({'path': name, 'rule': 'binary-control-bytes'})
    for rule, pattern in RULES.items():
        matches = pattern.finditer(text)
        if rule == 'vehicle-identifier':
            matches = (m for m in matches if any(c.isdigit() for c in m.group()))
        for match in matches:
            findings.append({'path': name, 'rule': rule, 'line': text[:match.start()].count('\n') + 1})
    for match in EMAIL.finditer(text):
        if not match.group().endswith('@users.noreply.github.com'):
            findings.append({'path': name, 'rule': 'non-public-commit-email',
                             'line': text[:match.start()].count('\n') + 1})
    return findings


def verify_snapshot(contents):
    findings = []
    try:
        manifest = json.loads(contents[MANIFEST])
        approved = manifest['files']
        if manifest['schema'] != 1 or not isinstance(approved, dict):
            raise ValueError('schema')
        if any(not path_ok(name) or not re.fullmatch(r'[0-9a-f]{64}', sha)
               for name, sha in approved.items()):
            raise ValueError('entries')
    except (KeyError, ValueError, TypeError, UnicodeError):
        return [{'path': MANIFEST, 'rule': 'missing-or-invalid-manifest'}]
    for name in sorted(set(contents) ^ (set(approved) | {MANIFEST})):
        findings.append({'path': name, 'rule': 'file-set-drift'})
    for name, raw in contents.items():
        findings.extend(inspect(name, raw))
        if name in approved and hashlib.sha256(raw).hexdigest() != approved[name]:
            findings.append({'path': name, 'rule': 'content-hash-drift'})
    return findings


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.DEVNULL)


def worktree(root):
    # Ignored build products stay outside the publication set. A force-added
    # ignored file is tracked and therefore checked, rather than silently skipped.
    names = git(root, 'ls-files', '-z', '--cached', '--others', '--exclude-standard').decode().split('\0')
    contents, findings = {}, []
    for name in sorted(set(filter(None, names))):
        path = root / name
        if path.is_symlink() or not path.is_file():
            findings.append({'path': name, 'rule': 'non-regular-file'})
        else:
            contents[name] = path.read_bytes()
    return findings + verify_snapshot(contents)


def history(root):
    commits = git(root, 'rev-list', 'HEAD').decode().splitlines()
    known = set(commits)
    findings, objects = [], set()
    for commit in commits:
        raw = git(root, 'cat-file', 'commit', commit)
        findings.extend(inspect('commit:' + commit[:12], raw, check_path=False))
        for parent in re.findall(rb'^parent ([0-9a-f]+)$', raw, re.M):
            if parent.decode() not in known:
                findings.append({'path': commit[:12], 'rule': 'incomplete-history'})
        contents = {}
        for entry in git(root, 'ls-tree', '-rz', '--full-tree', commit).split(b'\0'):
            if not entry:
                continue
            meta, encoded = entry.split(b'\t', 1)
            mode, kind, oid = meta.split()
            name = encoded.decode('utf-8')
            if mode not in (b'100644', b'100755') or kind != b'blob':
                findings.append({'path': name, 'rule': 'historical-non-regular-file'})
                continue
            contents[name] = git(root, 'cat-file', 'blob', oid.decode())
            objects.add(oid)
        findings.extend(dict(row, commit=commit[:12]) for row in verify_snapshot(contents))
    return findings, len(commits), len(objects)


def all_objects(root):
    findings, count = [], 0
    for line in git(root, 'cat-file', '--batch-all-objects', '--batch-check=%(objectname) %(objecttype)').splitlines():
        oid, kind = line.decode().split()
        if kind not in ('blob', 'commit', 'tag'):
            continue
        count += 1
        findings.extend(inspect('object:' + oid[:12], git(root, 'cat-file', kind, oid), check_path=False))
    return findings, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--history', action='store_true')
    parser.add_argument('--all-objects', action='store_true', help='also scan unreachable objects in this local database')
    args = parser.parse_args()
    try:
        findings = worktree(ROOT)
        result = {'branch': 'HEAD', 'history_checked': args.history, 'all_objects_checked': args.all_objects}
        if args.history:
            rows, result['commits'], result['historical_blobs'] = history(ROOT)
            findings.extend(rows)
        if args.all_objects:
            rows, result['database_objects_checked'] = all_objects(ROOT)
            findings.extend(rows)
        result.update(ok=not findings, findings=findings)
    except (subprocess.CalledProcessError, OSError, ValueError, UnicodeError):
        result = {'ok': False, 'findings': [{'rule': 'audit-input-or-git-error'}]}
    print(json.dumps(result, indent=2))
    return 0 if result['ok'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
