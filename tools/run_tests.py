#!/usr/bin/env python3
"""Build original host tests only; no downloads, firmware, SD or vehicle access."""
import argparse
import os
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent


def run(args):
    subprocess.run([str(x) for x in args], cwd=ROOT, check=True, timeout=90)


def main():
    arguments = argparse.ArgumentParser(description=__doc__)
    arguments.add_argument('--examples', action='store_true',
                           help='also print the synthetic frames, decoded fields and JSON plans')
    options = arguments.parse_args()
    output = ROOT / 'build'
    output.mkdir(exist_ok=True)
    cc = shlex.split(os.environ.get('CC', 'cc'))
    home = os.environ.get('JAVA_HOME')
    javac = str(Path(home) / 'bin/javac') if home else shutil.which('javac')
    java = str(Path(home) / 'bin/java') if home else shutil.which('java')
    if not cc or not shutil.which(cc[0]) or not javac or not java:
        raise RuntimeError('C compiler and JDK required; set CC/JAVA_HOME if needed')
    with tempfile.TemporaryDirectory(prefix='offline-', dir=output) as directory:
        temp = Path(directory)
        parser = ROOT / 'native/p0468-rgi-parser/p0468_rgi_parser.c'
        audit = ROOT / 'native/p0468-c314-input/p0468_c314_input.c'
        envelope = ROOT / 'native/p0468-c314-envelope/p0468_c314_envelope.c'
        for sanitized in (False, True):
            flags = ['-std=c99', '-Wall', '-Wextra', '-Werror']
            if sanitized:
                flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
            for name, sources in (
                ('parser', [parser, ROOT / 'tests/test-b26-rgi-parser.c']),
                ('strict-input', [parser, audit, ROOT / 'tests/test-c314-input.c']),
                ('envelope', [envelope, ROOT / 'tests/test-c314-envelope.c']),
            ):
                binary = temp / name
                run(cc + flags + sources + ['-o', binary])
                run([binary])
        run(cc + ['-std=c99', '-Wall', '-Wextra', '-Werror', parser,
                  ROOT / 'native/p0468-rgi-parser/p0468_rgi_decode.c', '-o', temp / 'rgi-decode'])
        generated = subprocess.check_output([sys.executable, ROOT / 'examples/synthetic_rgi.py'],
                                            cwd=ROOT, timeout=10)
        synthetic_input = temp / 'synthetic-frames.txt'
        synthetic_input.write_bytes(generated)
        decoded = subprocess.check_output([str(temp / 'rgi-decode'), str(synthetic_input)],
                                          cwd=ROOT, text=True, timeout=10)
        assert 'decoded_frames=3' in decoded and 'lane3={position=3,status=2' in decoded
        assert 'Demo turn' not in decoded and 'Demo lanes' not in decoded
        print('Synthetic RGI generator -> host decoder PASS, text redacted')
        if options.examples:
            print(generated.decode('ascii'), end='')
            print(decoded, end='')
        sources = sorted((ROOT / 'java').rglob('*.java'))
        run([javac, '-encoding', 'UTF-8', '-d', temp, *sources])
        run([java, '-cp', temp, 'org.mst2research.guidance.C314RichPolicyTest'])
        run([java, '-cp', temp, 'org.mst2research.guidance.C314GuidanceAdapterTest'])
        demo = subprocess.check_output([java, '-cp', str(temp), 'org.mst2research.guidance.GuidanceDemo'],
                                       cwd=ROOT, text=True, timeout=10)
        plans = [json.loads(line) for line in demo.splitlines()]
        assert len(plans) == 4 and all(p['synthetic_only'] for p in plans)
        assert plans[0]['side_codes'] == [64, 0]
        assert [lane['recommendation'] for lane in plans[0]['lanes']] == [0, 0, 2, 2]
        assert plans[1]['guidance_active'] and not plans[1]['instruction_layers']
        assert all(p['replace_sides'] and p['replace_lanes'] and not p['lanes'] and not p['side_codes']
                   for p in plans[1:])
        assert not plans[2]['guidance_active'] and plans[3]['mode'] == 0
        print('Synthetic JSON demo: rich turn/lanes, recalculation, stop and refusal PASS')
        if options.examples:
            print(demo, end='')
        run([sys.executable, ROOT / 'tests/test_publication.py'])
    print('Offline C normal/ASan/UBSan, pure Java policy and publication tests PASS; no vehicle validation')


if __name__ == '__main__':
    main()
