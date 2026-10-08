#!/usr/bin/env python3
"""Build the native PE32 portable app; optionally create its release ZIP."""
import argparse
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent
VERSION = '0.1.0-rc.1'
BUNDLE = 'UNI2-Block-List-' + VERSION + '-win32'


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='i686-w64-mingw32-gcc')
    parser.add_argument('--cxx', default='i686-w64-mingw32-g++')
    parser.add_argument('--windres', default='i686-w64-mingw32-windres')
    parser.add_argument('--package', action='store_true', help='also create a ZIP and SHA256SUMS.txt')
    args = parser.parse_args()
    for compiler in (args.cc, args.cxx):
        if subprocess.check_output([compiler, '-dumpmachine'], text=True).strip() != 'i686-w64-mingw32':
            raise SystemExit('Use the i686-w64-mingw32 (32-bit Windows) compiler')
    protocol = (ROOT / 'include/protocol.h').read_text(encoding='utf-8')
    if not re.search(r'UB_VERSION\[\]\s*=\s*"' + re.escape(VERSION) + r'"', protocol):
        raise SystemExit('build.py and include/protocol.h versions must agree')
    vendor = ROOT / 'vendor/minhook'
    for item in json.loads((vendor / 'SOURCE.json').read_text(encoding='utf-8'))['files']:
        if sha256(vendor / item['path']) != item['sha256']:
            raise SystemExit('MinHook source fingerprint mismatch: ' + item['path'])
    out = ROOT / 'build' / BUNDLE
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    obj = ROOT / 'build/obj'
    obj.mkdir(exist_ok=True)
    common = ['-O2', '-Wall', '-Wextra', '-Werror', '-finput-charset=UTF-8',
              '-I' + str(ROOT / 'include'), '-I' + str(vendor / 'include')]

    def run(argv):
        subprocess.run([str(value) for value in argv], cwd=ROOT, check=True)

    objects = []
    for name in ['buffer', 'hook', 'trampoline', 'hde/hde32']:
        target = obj / (name.replace('/', '_') + '.o')
        objects.append(target)
        run([args.cc, *common, '-Wno-unused-parameter', '-c', vendor / 'src' / (name + '.c'), '-o', target])
    cpp = [args.cxx, *common, '-std=c++17', '-static', '-static-libgcc', '-static-libstdc++']
    run([*cpp, '-shared', '-Wl,--kill-at', ROOT / 'src/runtime.cpp', *objects,
         '-ladvapi32', '-o', out / 'uni2-block-list.dll'])
    resource = obj / 'gui_resource.o'
    run([args.windres, '--input', ROOT / 'resources/gui.rc', '--output', resource,
         '--output-format=coff', '--target=pe-i386', '--include-dir', ROOT / 'resources',
         '--preprocessor=' + args.cc, '--preprocessor-arg=-E', '--preprocessor-arg=-xc',
         '--preprocessor-arg=-DRC_INVOKED'])
    run([*cpp, '-mwindows', '-municode', ROOT / 'src/gui.cpp', ROOT / 'src/injector.cpp',
         ROOT / 'src/settings.cpp', ROOT / 'src/ui_language.cpp', resource,
         '-lcomctl32', '-lcomdlg32', '-lshell32', '-ladvapi32', '-luxtheme', '-lgdi32',
         '-o', out / 'UNI2 Block List.exe'])
    for name in ['README.md', 'README.zh-CN.md', 'LICENSE']:
        shutil.copy2(ROOT / name, out / name)
    shutil.copy2(vendor / 'LICENSE.txt', out / 'MinHook-LICENSE.txt')
    receipt = {path.name: sha256(path) for path in sorted(out.iterdir())}
    (out / 'SHA256.json').write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(out)
    if args.package:
        archive = ROOT / 'build' / (BUNDLE + '.zip')
        with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as package:
            for path in sorted(out.iterdir()):
                info = zipfile.ZipInfo(BUNDLE + '/' + path.name, date_time=(1980, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = 0o100644 << 16
                package.writestr(info, path.read_bytes(), compresslevel=9)
        digest = sha256(archive)
        (ROOT / 'build/SHA256SUMS.txt').write_text(digest + '  ' + archive.name + '\n', encoding='ascii')
        print(archive)
        print('SHA-256: ' + digest)


if __name__ == '__main__':
    main()
