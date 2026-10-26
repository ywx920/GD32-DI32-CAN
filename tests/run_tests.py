"""Host-only tests; mocks replace peripherals and FRAM. No board is accessed."""
import os
import shutil
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
gcc = os.environ.get('DI_HOST_CC') or shutil.which('gcc')
if not gcc:
    raise SystemExit('Set DI_HOST_CC to the host GCC executable.')
commands = [
    [gcc, '-std=c99', '-Wall', '-Wextra', '-Werror', '-DDI_HOST_TEST', '-I', '.',
     'tests/test_di.c', 'task.c', 'di_protocol.c', '-o', 'tests/test_di.exe'],
    [gcc, '-std=c99', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
     '-DGD32F450', '-DUSE_STDPERIPH_DRIVER', '-I', '.',
     '-I', 'Library/Fimware/CMSIS', '-I', 'Library/Fimware/CMSIS/GD/GD32F4xx/Include',
     '-I', 'Library/Fimware/GD32F4xx_standard_peripheral/Include',
     '-I', 'Library/Utilities', 'tests/test_can.c', 'can.c', '-o', 'tests/test_can.exe']
]
log = []
for command, executable in zip(commands, ['tests/test_di.exe', 'tests/test_can.exe']):
    r = subprocess.run(command, cwd=root, capture_output=True, text=True, errors='replace')
    log.append(r.stdout + r.stderr)
    if r.returncode:
        (root/'tests/test_results.txt').write_text('\n'.join(log), encoding='utf-8')
        raise SystemExit(r.stdout + r.stderr)
    r = subprocess.run([str(root/executable)], cwd=root, capture_output=True, text=True)
    log.append(r.stdout + r.stderr)
    if r.returncode:
        (root/'tests/test_results.txt').write_text('\n'.join(log), encoding='utf-8')
        raise SystemExit(r.stdout + r.stderr)
(root/'tests/test_results.txt').write_text('\n'.join(log), encoding='utf-8')
print('\n'.join(x for x in log if x.strip()))
