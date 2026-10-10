import hashlib, io, json, pathlib, tempfile, unittest, zipfile
from unittest.mock import patch
from installer import core
from installer.vendor.rx3_toolkit import firmware_image

class InstallerTests(unittest.TestCase):
 def test_persistent_save_is_opt_in(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);marker=p/'persistent-save.enabled';(p/'autoexec.sh').write_text('#!/bin/sh\n')
   core.configure_save(p);self.assertFalse(marker.exists())
   core.configure_save(p,True);self.assertEqual(marker.read_text(),'1\n')
   plain=firmware_image.build_autoexec_iso(p)
   self.assertIn(b'persistent-save.enabled',plain)
   core.configure_save(p,False);self.assertFalse(marker.exists())
 def test_crypto_iso_roundtrip(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);(p/'autoexec.sh').write_text('#!/bin/sh\necho hello\n');(p/'test').write_bytes(b'data')
   plain=firmware_image.build_autoexec_iso(p);key=bytes(range(32))
   encoded=firmware_image.crypt(plain,key,False)
   self.assertNotEqual(plain,encoded);self.assertEqual(firmware_image.crypt(encoded,key,True),plain)
   self.assertEqual(firmware_image.autoexec_iso_metadata(plain),'UsbAuto')
 def test_wrong_parts(self):
  with tempfile.TemporaryDirectory() as t:
   a=pathlib.Path(t)/'a';b=pathlib.Path(t)/'b';a.write_bytes(b'a');b.write_bytes(b'b')
   for paths in ((a,a),(a,b)):
    with self.assertRaises(core.InstallError):core.validate_parts(paths)
 def test_reject_path_traversal(self):
  for name in ('../outside','/outside','C:/outside','dir\\outside'):
   with tempfile.TemporaryDirectory() as t:
    p=pathlib.Path(t);z=p/'bad.zip'
    with zipfile.ZipFile(z,'w') as f:f.writestr(name,'bad')
    with self.assertRaises(core.InstallError):core.unpack(z,p/'payload')
 def test_usb_backup_and_replacement(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);d=core.Drive(t,'one','Test USB',2**30)
   (p/'autoexec.bin').write_bytes(b'old');(p/'playlist').write_bytes(b'keep')
   with patch.object(core,'drives',return_value=[d]):core.install_image(b'new',d)
   self.assertEqual((p/'autoexec.bin').read_bytes(),b'new')
   self.assertEqual(next(p.glob('autoexec-backup-*.bin')).read_bytes(),b'old')
   self.assertEqual((p/'playlist').read_bytes(),b'keep');self.assertFalse(list(p.glob('.rx3-install-*')))
 def test_removed_drive(self):
  with tempfile.TemporaryDirectory() as t:
   d=core.Drive(t,'one','Test',2**30)
   with patch.object(core,'drives',return_value=[]),self.assertRaises(core.InstallError):core.install_image(b'x',d)
   self.assertEqual(list(pathlib.Path(t).iterdir()),[])
 def test_changed_drive_preserves_old_image(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);d=core.Drive(t,'one','Test',2**30);(p/'autoexec.bin').write_bytes(b'old')
   with patch.object(core,'drives',side_effect=[[d],[]]),self.assertRaises(core.InstallError):core.install_image(b'new',d)
   self.assertEqual((p/'autoexec.bin').read_bytes(),b'old');self.assertFalse(list(p.glob('.rx3-install-*')))
 def test_symlink_target_rejected(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);d=core.Drive(t,'one','Test',2**30);(p/'real').write_bytes(b'old')
   try:(p/'autoexec.bin').symlink_to(p/'real')
   except OSError:self.skipTest('symlinks not available')
   with patch.object(core,'drives',return_value=[d]),self.assertRaises(core.InstallError):core.install_image(b'new',d)
   self.assertEqual((p/'real').read_bytes(),b'old')
 def test_low_space_preserves_old(self):
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t);d=core.Drive(t,'one','Test',2**30);(p/'autoexec.bin').write_bytes(b'old')
   with patch.object(core,'drives',return_value=[d]),patch.object(core.shutil,'disk_usage',return_value=type('Space',(),{'free':0})()),self.assertRaises(core.InstallError):core.install_image(b'new',d)
   self.assertEqual((p/'autoexec.bin').read_bytes(),b'old')
 def test_token_cannot_leave_github(self):
  with self.assertRaises(core.InstallError):core.GitHub('example').get('https://example.com/')
 def test_public_files_do_not_consume_api_quota(self):
  from unittest.mock import MagicMock
  opener=MagicMock();opener.open.return_value.__enter__.return_value.read.return_value=b'chunk'
  with patch.object(core.urllib.request,'build_opener',return_value=opener):
   self.assertEqual(core.GitHub().file('release/chunks/000.part','a'*40),b'chunk')
  request=opener.open.call_args.args[0]
  self.assertEqual(request.full_url,'https://raw.githubusercontent.com/'+core.REPOSITORY+'/'+'a'*40+'/release/chunks/000.part')
  self.assertFalse(request.has_header('Authorization'))
 def test_authenticated_files_keep_token_on_api(self):
  client=core.GitHub('example')
  with patch.object(client,'get',return_value={'encoding':'base64','content':'YQ=='}) as get:
   self.assertEqual(client.file('release/package.json','a'*40),b'a')
   self.assertTrue(get.call_args.args[0].startswith(core.API+'/contents/'))
 def test_file_ref_and_path_validation(self):
  for path,commit in [('release/package.json','main'),('../other','a'*40),('/other','a'*40),('a\\b','a'*40)]:
   with self.assertRaises(core.InstallError):core.GitHub().file(path,commit)
if __name__=='__main__':unittest.main()
