"""Build only. Never downloads/flashes firmware."""
import os
import re
import subprocess
from pathlib import Path
root = Path(__file__).resolve().parent
uv4 = Path(os.environ.get('KEIL_UV4', 'D:/Keil_v5/UV4/UV4.exe'))
if not uv4.exists():
    raise SystemExit('Set KEIL_UV4 to the uVision UV4.exe path.')
log = root/'MDK-ARM/output/di1014-build.log'
result = subprocess.run([str(uv4), '-b', str(root/'MDK-ARM/GD32F450ZE_DI1014.uvprojx'),
                         '-o', str(log), '-j0'],
                        creationflags=subprocess.CREATE_NO_WINDOW, timeout=180)
text = log.read_bytes().decode('gb18030', errors='replace') if log.exists() else ''
print(text)
if result.returncode != 0 or not re.search(r'0 Error\(s\)', text):
    raise SystemExit('Keil build failed; inspect MDK-ARM/output/di1014-build.log')
hexfile = root/'MDK-ARM/output/DIM-DI-1014-250k.hex'
if not hexfile.exists():
    raise SystemExit('Build did not produce expected HEX.')
print('HEX:', hexfile)
