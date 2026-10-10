# DJ DRAGON-X Cat Rescue Saga Installer 0.4.7

Windows and Linux desktop installer for the XDJ-RX3 Cat Rescue Saga.
Run `DJ-DRAGON-X-Cat-Rescue-Saga-Installer.exe` on Windows or `DJ-DRAGON-X-Cat-Rescue-Saga-Installer` on Linux
from the unpacked distribution. No Python installation is needed for a packaged build.
The Windows build must be produced on Windows; the Linux build on Linux.

1. Mount a FAT32/exFAT USB stick and select it by label, path and capacity.
2. Obtain both XDJ-RX3 source-code parts yourself from the
   [Pioneer DJ source-code page](https://www.pioneerdj.com/en/support/open-source-code-distribution/gnu-open-source-license/).
   Read the manufacturer's terms. The installer does not download these packages.
3. Select the original ZIP downloads or their extracted
   `pioneerdj_xdj_rx3.tar.bz2.00` and `.01` files, in that order.
4. Accept the terms and click **Check source packages**. Exact sizes and SHA-256
   digests are checked, followed by the contents. An unrelated firmware update
   or package for another device is rejected.
5. For a private project repository, enter a GitHub token with read-only Contents
   access to `DRAGON-EX/XDJ-RX3-ARCADE-GAMES-COLLECTION`. It is not stored.
6. Leave **PERSISTENT SAVE FILE** off for session-only play (recommended).
   If enabled, highscores, rescued cats, finale victory and song unlocks survive
   restarts. **Unplugging during saving can corrupt the USB filesystem.**
7. Click **Install to USB** and check the selected destination. The current main
   revision is resolved once; all game package files come from that same revision.
   The installer verifies checksums, builds the ISO in a temporary directory,
   encrypts it locally using the user-supplied data, verifies it and writes it.
8. Wait for success, then safely eject the stick through your operating system.

The installer never formats the drive. It backs up an existing `autoexec.bin`
and replaces only that file. Removal, drive identity changes, insufficient space,
invalid packages and failed read-back stop installation. Keep the drive attached
until completion. An unsigned application may require the OS's normal confirmation.

`good.mp3` and its RX3-ready `cafe-show.pcm` are in `good.zip`, protected with the
requested password `good`. Extraction is automatic. This public password is a
packaging convention, not confidentiality or DRM. The MP3 is verified temporarily;
the PCM goes into the device image. Keys and manufacturer files are never uploaded,
installed as separate files on the stick, or retained by the installer.

## Included toolkit code and licenses

The required image-building/encryption and source-package reader components of
[Tratosca/rx3-toolkit](https://github.com/Tratosca/rx3-toolkit) are included under
MPL-2.0. Exact corresponding source, the full license and modification notes are
in `vendor/rx3_toolkit`; packaged builds include `installer-source/vendor/rx3_toolkit`.
The original toolkit application, branding and unrelated modules are not required.
MPL-2.0 sections 3.1–3.4 permit bundling with notices and source availability.
Additional Python runtime/dependency licenses accompany packaged distributions.
No claim of affiliation with Pioneer DJ, AlphaTheta or RX3 Toolkit is made.

## Build and test

Use Python 3.12 with Tk (Windows's standard Python includes Tk; on Linux install
Tk support using the distribution's Python package manager).

```
python -m venv .venv
# Activate .venv using your operating system's normal command.
python -m pip install -r installer/requirements-build.txt
python -m unittest discover -s tests -p test_installer.py
python -m installer.gui
python tools/build_installer.py
```

Game maintainers build the native payload locally, then run
`python tools/build_release_package.py`. Firmware, device libraries, keys,
SSH credentials and Road Fighter ROMs are excluded by the explicit package list.
The source checkout remains the development source; the installer downloads the
prebuilt runtime in `release/package.json` and `release/chunks/`.

## Optional persistent progress

The installer adds an opt-in marker to the device image. Default installations
do not read or write save files. Enabled installations keep two checksummed
generations, `rx3-save-a.dat` and `rx3-save-b.dat`, in the USB root. Writes run
in the background, with score updates coalesced over up to five seconds; new
unlocks are queued immediately. A SAVING indicator warns while a write is active.
This reduces lost progress but cannot prevent FAT/exFAT corruption on removal
during a write. A corrupt newest generation falls back to the previous valid one.
Debug-unlocked mode never loads or saves progress. Reinstalling with the option
off leaves existing save files untouched and ignores them. It does not resume
a partially played level or match.

For browser previews in a fresh checkout, run
`python tools/unpack_preview_music.py`. The extracted MP3 remains ignored by Git;
only the password-protected ZIP is distributed.
