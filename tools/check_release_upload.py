"""Check GitHub's asset digests against the locally packaged release files."""
from pathlib import Path
import hashlib, json, sys

root = Path(__file__).resolve().parents[1]
remote = json.loads(Path(sys.argv[1]).read_text())
meta = json.loads((root/'release/current.json').read_text())
assert remote['tag_name'] == meta['tag']
assets = {asset['name']:asset for asset in remote['assets']}
for file in sorted((root/'dist').iterdir()):
    asset = assets[file.name]
    assert asset['state'] == 'uploaded'
    assert asset['size'] == file.stat().st_size
    assert asset['digest'] == 'sha256:'+hashlib.sha256(file.read_bytes()).hexdigest(), file.name
print('PASS all GitHub release asset sizes and SHA-256 digests')
