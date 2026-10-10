# RX3 Toolkit components — MPL-2.0

Upstream: https://github.com/Tratosca/rx3-toolkit
Files: app/firmware/firmware_image.py, key_source.py and key_source.json.
The exact corresponding source and full MPL-2.0 license accompany this installer
in vendor/rx3_toolkit (also included in every packaged distribution).

Changes on 2026-10-10: key_source.py uses a standalone error message instead of
the toolkit localization subsystem; finds metadata alongside its source; accepts
already-unzipped source parts. Input sizes and SHA-256 are checked by our wrapper
before it calls the extractor. No manufacturer package or key is bundled.
firmware_image.py and key_source.json are unchanged from our upstream snapshot.

MPL-2.0 sections 3.1–3.4 permit this distribution with notices and corresponding
source. This is an independent installer, not an official RX3 Toolkit release.
