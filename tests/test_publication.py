#!/usr/bin/env python3
"""Publication regressions using synthetic temporary files and Git repositories."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent
SPEC = importlib.util.spec_from_file_location('audit', ROOT / 'tools/audit_public.py')
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


def snapshot(contents):
    result = dict(contents)
    result[AUDIT.MANIFEST] = json.dumps({'schema': 1, 'files': {
        n: hashlib.sha256(b).hexdigest() for n, b in contents.items()}}).encode()
    return result


class Tests(unittest.TestCase):
    def test_clean_synthetic_text_and_binary_metadata_rejection(self):
        self.assertFalse(AUDIT.verify_snapshot(snapshot({'README.md': b'Synthetic research only\n'})))
        for name, raw in (('picture.jpg', b'\xff\xd8Exif\x00'), ('module.so', b'\x7fELF\x00'),
                          ('OEM.java', b'package de.' + b'vw.example;\n'),
                          ('README.md', b'\x7fELF\x00')):
            self.assertTrue(AUDIT.verify_snapshot(snapshot({name: raw})), name)

    def test_identifiers_credentials_and_coordinates_are_not_printed(self):
        cases = [b'AA' + b':BB:CC:DD:EE:FF', b'/Us' + b'ers/example/private',
                 b'ghp_' + b'a' * 36, b'48.' + b'123456, 2.654321',
                 b'person' + b'@example.invalid']
        for raw in cases:
            findings = AUDIT.inspect('README.md', raw)
            self.assertTrue(findings)
            self.assertNotIn(raw.decode(), json.dumps(findings))

    def test_extra_files_mutation_and_traversal_cannot_bypass_manifest(self):
        valid = snapshot({'README.md': b'clean'})
        self.assertTrue(AUDIT.verify_snapshot(dict(valid, extra=b'private')))
        mutated = dict(valid); mutated['README.md'] = b'changed'
        self.assertTrue(AUDIT.verify_snapshot(mutated))
        self.assertTrue(AUDIT.verify_snapshot(snapshot({'../outside.md': b'clean'})))

    def test_removed_private_blob_still_fails_history_audit(self):
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as directory:
            repo = Path(directory)
            subprocess.run(['git', 'init', '-q', str(repo)], check=True)

            def commit(contents, message):
                for name, raw in snapshot(contents).items():
                    (repo / name).write_bytes(raw)
                subprocess.run(['git', '-C', str(repo), 'add', '.'], check=True)
                subprocess.run(['git', '-C', str(repo), '-c', 'user.name=Publication Test',
                                '-c', 'user.email=publication@users.noreply.github.com',
                                'commit', '-qm', message], check=True)

            commit({'README.md': b'first ' + b'AA' + b':BB:CC:DD:EE:FF'}, 'fixture before')
            commit({'README.md': b'clean current'}, 'fixture after')
            self.assertFalse(AUDIT.worktree(repo))
            findings, count, _ = AUDIT.history(repo)
            self.assertEqual(count, 2)
            self.assertTrue(any(row['rule'] == 'hardware-address' for row in findings))

    def test_synthetic_observations_do_not_claim_a_live_fix(self):
        data = json.loads((ROOT / 'examples/synthetic-observations.json').read_text())
        self.assertFalse(data['live_carplay_mapping_fully_validated'])
        self.assertFalse(data['phone_overlay_fix_validated'])
        self.assertFalse(data['stream111_on_p0468_demonstrated'])
        by_id = {row['id']: row for row in data['cases']}
        self.assertEqual(by_id['four_lanes_two_right']['recommendations_by_position'], [0, 0, 2, 2])
        self.assertEqual(by_id['four_lanes_two_left']['recommendations_by_position'], [2, 2, 0, 0])

    def test_full_catalogue_preserves_exact_synthetic_inputs_and_study_limits(self):
        data = json.loads((ROOT / 'examples/catalogue-23.json').read_text())
        self.assertFalse(data['raw_vehicle_evidence_included'])
        self.assertFalse(data['dwell_is_protocol_requirement'])
        cases = data['cases']
        self.assertEqual([c['slot'] for c in cases], list(range(23)))
        self.assertTrue(all(c['synthetic'] and not c['live_mapping_qualified'] for c in cases))
        self.assertEqual(cases[14]['side_codes'], [96, 32, 64, 16])
        self.assertEqual(cases[14]['observation_status'], 'partial_branches')
        self.assertEqual([c['main'] for c in cases[17:22]], [19, 19, 20, 20, 20])
        self.assertEqual([c['direction'] for c in cases[17:22]], [64, 192, 64, 0, 192])
        self.assertTrue(all(c['study_only'] for c in cases[10:22]))
        self.assertEqual(cases[7]['lanes'], [])


if __name__ == '__main__':
    unittest.main()
