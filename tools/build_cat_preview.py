"""Build the native cafe scene for browser previews (fixed memory, no imports)."""
import subprocess
exports=['cats_open','cats_cafe','cats_rescue','cats_seen','cats_buffer','cats_show','cats_vinyl_hit','cats_leds']
subprocess.run(['clang','--target=wasm32','-O2','-ffreestanding','-fno-builtin','-nostdlib','-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=6291456','-Wl,--max-memory=6291456',*[f'-Wl,--export={x}' for x in exports],'src/cat_preview.c','-o','assets/cat-scenes.wasm'],check=True)
