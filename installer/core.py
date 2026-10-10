"""USB installer backend. No firmware/key downloads, formatting or shell execution."""
from __future__ import annotations
import base64, dataclasses, hashlib, io, json, os, pathlib, shutil, ssl, stat
import subprocess, sys, tempfile, time, urllib.request, zipfile
from .vendor.rx3_toolkit import key_source, firmware_image

ROOT = pathlib.Path(__file__).resolve().parent
REPOSITORY = 'DRAGON-EX/XDJ-RX3-ARCADE-GAMES-COLLECTION'
API = 'https://api.github.com/repos/' + REPOSITORY
VERSION = '0.4.7'
MAX_PACKAGE = 120 * 1024 * 1024
MAX_PAYLOAD = 95 * 1024 * 1024

class InstallError(ValueError): pass

@dataclasses.dataclass(frozen=True)
class Drive:
    root: str
    identity: str
    label: str
    size: int

def sha(path):
    with pathlib.Path(path).open('rb') as f: return hashlib.file_digest(f, 'sha256').hexdigest()

def drives():
    result = []
    if sys.platform == 'win32':
        # Get-Disk BusType distinguishes USB SSDs from internal fixed disks.
        script = '''$ErrorActionPreference='Stop'; @((Get-Disk | Where-Object {$_.BusType -eq 'USB' -and -not $_.IsBoot -and -not $_.IsSystem}) | ForEach-Object { $d=$_; Get-Partition -DiskNumber $d.Number | Where-Object {$_.DriveLetter} | ForEach-Object { $p=$_; $v=$p | Get-Volume; [pscustomobject]@{root=([string]$p.DriveLetter+':\\'); identity=([string]$d.UniqueId+':'+[string]$p.Offset+':'+[string]$v.UniqueId); label=$v.FileSystemLabel; fs=$v.FileSystem; size=$v.Size} }}) | ConvertTo-Json -Compress'''
        flags = getattr(subprocess, 'CREATE_NO_WINDOW', 0)
        output = subprocess.check_output(['powershell.exe','-NoProfile','-NonInteractive','-Command',script], text=True, creationflags=flags, timeout=20)
        rows = json.loads(output or '[]')
        if isinstance(rows, dict): rows=[rows]
        for row in rows or []:
            if row.get('fs','').lower() in ('fat32','exfat'):
                result.append(Drive(row['root'],row['identity'],row.get('label') or 'USB',int(row['size'])))
    elif sys.platform.startswith('linux'):
        data=json.loads(subprocess.check_output(['lsblk','--json','--bytes','--output','NAME,PATH,TYPE,TRAN,RM,SIZE,MOUNTPOINTS,FSTYPE,UUID,SERIAL,RO'],text=True,timeout=10))
        for disk in data['blockdevices']:
            if disk.get('tran')!='usb' or disk.get('ro'): continue
            def visit(node):
                mounts=node.get('mountpoints') or []
                for mount in mounts:
                    if mount and mount not in ('/','/home','/boot','/boot/efi') and node.get('fstype') in ('vfat','exfat') and not node.get('ro'):
                        identity='|'.join(str(x or '') for x in (disk.get('serial'),disk['path'],node['path'],node.get('uuid'),node['size']))
                        result.append(Drive(mount,identity,pathlib.Path(mount).name,int(node['size'])))
                for child in node.get('children',[]): visit(child)
            visit(disk)
    return result

def validate_drive(drive):
    if drive not in drives(): raise InstallError('USB stick changed or was removed. Select it again.')
    root=pathlib.Path(drive.root)
    if root.is_symlink() or not root.is_dir(): raise InstallError('Invalid USB root.')
    return root

def validate_parts(paths, progress=lambda x:None):
    if len(paths)!=2 or pathlib.Path(paths[0]).resolve()==pathlib.Path(paths[1]).resolve():
        raise InstallError('Select two different source parts.')
    source=key_source.load_source()
    raw=json.loads((ROOT/'source-parts.json').read_text())
    for i,(path,part) in enumerate(zip(map(pathlib.Path,paths),source.parts)):
        progress('Checking Pioneer part '+str(i+1)+' / 2…')
        allowed={(part.size,part.sha256),(raw[i]['size'],raw[i]['sha256'])}
        if not path.is_file() or not any(path.stat().st_size==size for size,_ in allowed):
            raise InstallError(f'Part {i+1}: wrong file size. Use the XDJ-RX3 source package (ZIP or .0{i}).')
        if (path.stat().st_size,sha(path)) not in allowed:
            raise InstallError(f'Part {i+1}: checksum differs from the verified Pioneer package.')
    progress('Checking the contents of both source parts…')
    return key_source.effective(key_source.key_from_parts(map(pathlib.Path,paths),source))

class GitHub:
    def __init__(self, token=''): self.token=token.strip()
    def get(self, url, limit=MAX_PACKAGE):
        # Credentials go only to the GitHub API, never to redirects or arbitrary manifest URLs.
        if not url.startswith(API+'/'): raise InstallError('Unexpected download location.')
        headers={'Accept':'application/vnd.github+json','User-Agent':'RX3-Arcade-Installer/'+VERSION}
        if self.token: headers['Authorization']='Bearer '+self.token
        import certifi
        class NoRedirect(urllib.request.HTTPRedirectHandler):
            def redirect_request(self,*args,**kwargs): raise InstallError('Unexpected GitHub redirect.')
        opener=urllib.request.build_opener(NoRedirect,urllib.request.HTTPSHandler(context=ssl.create_default_context(cafile=certifi.where())))
        try:
            with opener.open(urllib.request.Request(url,headers=headers),timeout=60) as response:
                data=response.read(limit+1)
        except urllib.error.HTTPError as e:
            if e.code in (401,403,404):
                raise InstallError('GitHub access denied or package missing. For this private repository, enter a token with read-only Contents access. API rate limits can also cause this message.') from None
            raise InstallError('GitHub download failed (HTTP '+str(e.code)+').') from None
        if len(data)>limit: raise InstallError('Download exceeds allowed size.')
        return json.loads(data)
    def file(self,path,commit):
        from urllib.parse import quote
        if len(commit)!=40 or any(c not in '0123456789abcdef' for c in commit): raise InstallError('Invalid GitHub revision.')
        if path.startswith('/') or '..' in path or '\\' in path: raise InstallError('Invalid package path.')
        if not self.token:
            # Public payloads use raw files; 94 chunks would exceed the anonymous API quota.
            import certifi
            url='https://raw.githubusercontent.com/'+REPOSITORY+'/'+commit+'/'+quote(path,safe='/')
            class NoRedirect(urllib.request.HTTPRedirectHandler):
                def redirect_request(self,*args,**kwargs): raise InstallError('Unexpected download redirect.')
            opener=urllib.request.build_opener(NoRedirect,urllib.request.HTTPSHandler(context=ssl.create_default_context(cafile=certifi.where())))
            try:
                with opener.open(urllib.request.Request(url,headers={'User-Agent':'RX3-Arcade-Installer/'+VERSION}),timeout=60) as response:
                    data=response.read(8*1024*1024+1)
            except urllib.error.HTTPError as e:
                raise InstallError('Game file download failed (HTTP '+str(e.code)+').') from None
            if len(data)>8*1024*1024: raise InstallError('Download exceeds allowed size.')
            return data
        obj=self.get(API+'/contents/'+quote(path,safe='/')+'?ref='+commit, 8*1024*1024)
        if obj.get('encoding')!='base64': raise InstallError('Unsupported GitHub file response.')
        return base64.b64decode(obj['content'])
    def download(self,folder,progress):
        head=self.get(API+'/commits/main',2*1024*1024)['sha']
        if len(head)!=40 or any(c not in '0123456789abcdef' for c in head): raise InstallError('Invalid GitHub revision.')
        manifest=json.loads(self.file('release/package.json',head))
        if manifest.get('format')!=1 or manifest.get('model')!='XDJ-RX3': raise InstallError('Incompatible game package.')
        pieces=manifest.get('chunks',[])
        if not pieces or len(pieces)>250: raise InstallError('Invalid package manifest.')
        total=0; output=pathlib.Path(folder)/'runtime.zip'
        with output.open('wb') as stream:
            for i,piece in enumerate(pieces):
                path=piece['path']
                if not path.startswith('release/chunks/') or '..' in path: raise InstallError('Invalid package path.')
                progress(f'Downloading version {manifest["version"]}: {i+1} / {len(pieces)}…')
                data=self.file(path,head)
                if len(data)!=piece['size'] or hashlib.sha256(data).hexdigest()!=piece['sha256']: raise InstallError('Download checksum mismatch.')
                total+=len(data)
                if total>MAX_PACKAGE: raise InstallError('Package too large.')
                stream.write(data)
        if sha(output)!=manifest['sha256']: raise InstallError('Package checksum mismatch.')
        return output,manifest

def unpack(package, folder):
    folder=pathlib.Path(folder);folder.mkdir()
    with zipfile.ZipFile(package) as z:
        infos=z.infolist()
        if len(infos)>500 or sum(x.file_size for x in infos)>MAX_PACKAGE: raise InstallError('Package too large.')
        names=set()
        for item in infos:
            p=pathlib.PurePosixPath(item.filename)
            if p.is_absolute() or '..' in p.parts or '\\' in item.filename or ':' in item.filename or item.filename in names:
                raise InstallError('Unsafe package path.')
            names.add(item.filename)
            if stat.S_ISLNK(item.external_attr>>16) or item.is_dir(): raise InstallError('Unexpected package entry.')
            if len(p.parts)!=1: raise InstallError('Unexpected package directory.')
            (folder/item.filename).write_bytes(z.read(item))
    metadata=json.loads((folder/'payload.json').read_text())
    expected=set(metadata['files'])|{'payload.json'}
    if set(p.name for p in folder.iterdir())!=expected: raise InstallError('Unexpected payload files.')
    for name,digest in metadata['files'].items():
        if sha(folder/name)!=digest: raise InstallError('Payload checksum mismatch: '+name)
    # The track is intentionally distributed in a password-protected archive.
    # The password is public by design; this is packaging, not access control.
    protected=folder/'good.zip'
    with zipfile.ZipFile(protected) as z:
        if set(z.namelist())!={'good.mp3','cafe-show.pcm'}: raise InstallError('Unexpected music archive.')
        if sum(i.file_size for i in z.infolist())>32*1024*1024: raise InstallError('Music archive too large.')
        for name in ('good.mp3','cafe-show.pcm'):
            raw=z.read(name,pwd=b'good')
            if hashlib.sha256(raw).hexdigest()!=metadata['music'][name]: raise InstallError('Music checksum mismatch.')
            if name=='cafe-show.pcm': (folder/name).write_bytes(raw)
    protected.unlink();(folder/'payload.json').unlink()
    for name in ('autoexec.sh','start.sh','stop-dj.sh','rx3-arcade','rx3-launcher'):
        p=folder/name
        if not p.is_file(): raise InstallError('Required runtime file missing: '+name)
        p.chmod(0o755)
    if sum(p.stat().st_size for p in folder.iterdir())>MAX_PAYLOAD: raise InstallError('RX3 RAM budget exceeded.')
    return metadata

def install_image(image, drive, progress=lambda x:None):
    root=validate_drive(drive)
    target=root/'autoexec.bin'
    if target.is_symlink() or (target.exists() and not target.is_file()): raise InstallError('Unsafe existing autoexec.bin.')
    old_digest=sha(target) if target.exists() else None
    needed=len(image)+(target.stat().st_size if target.exists() else 0)+16*1024*1024
    if shutil.disk_usage(root).free<needed: raise InstallError('Not enough free space on the USB stick.')
    staged=None
    try:
        fd,name=tempfile.mkstemp(prefix='.rx3-install-',suffix='.tmp',dir=root);staged=pathlib.Path(name)
        progress('Writing and verifying the new USB image…')
        with os.fdopen(fd,'wb') as f: f.write(image);f.flush();os.fsync(f.fileno())
        if sha(staged)!=hashlib.sha256(image).hexdigest(): raise InstallError('USB verification failed.')
        validate_drive(drive)
        if target.is_symlink() or (sha(target) if target.exists() else None)!=old_digest: raise InstallError('USB contents changed during installation.')
        if old_digest:
            # Exclusive creation avoids overwriting any earlier backup.
            backup=root/('autoexec-backup-'+time.strftime('%Y%m%d-%H%M%S')+'-'+os.urandom(3).hex()+'.bin')
            with target.open('rb') as src,backup.open('xb') as out:
                shutil.copyfileobj(src,out);out.flush();os.fsync(out.fileno())
            if sha(backup)!=old_digest: raise InstallError('Previous version backup failed.')
        os.replace(staged,target);staged=None
        if os.name=='posix':
            fd=os.open(root,os.O_RDONLY)
            try: os.fsync(fd)
            finally: os.close(fd)
        return target
    finally:
        if staged is not None: staged.unlink(missing_ok=True)

def configure_save(payload, persistent=False):
    marker=pathlib.Path(payload)/'persistent-save.enabled'
    marker.unlink(missing_ok=True)
    if persistent: marker.write_text('1\n',encoding='ascii')

def run(drive,key,token,progress,persistent=False):
    validate_drive(drive)
    with tempfile.TemporaryDirectory(prefix='rx3-installer-') as tmp:
        archive,manifest=GitHub(token).download(tmp,progress)
        progress('Unpacking the game and protected music archive…')
        payload=pathlib.Path(tmp)/'payload';unpack(archive,payload)
        configure_save(payload,persistent)
        progress('Building the RX3 USB image…')
        plain=firmware_image.build_autoexec_iso(payload)
        progress('Preparing and checking autoexec.bin…')
        encoded=firmware_image.crypt(plain,key,False)
        if firmware_image.crypt(encoded,key,True)!=plain: raise InstallError('Image verification failed.')
        firmware_image.autoexec_iso_metadata(plain)
        install_image(encoded,drive,progress)
    return manifest['version']
