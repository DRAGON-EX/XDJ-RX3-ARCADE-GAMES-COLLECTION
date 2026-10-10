"""Compile Equalizarr's shared RX3/browser C implementation."""
import subprocess
exports=['game_rescue_begin','game_set_best','game_reset','game_tick','game_frame','game_buffer','game_sound','game_value']
subprocess.run(['clang','--target=wasm32','-O2','-ffreestanding','-fno-builtin','-nostdlib','-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=5242880','-Wl,--max-memory=5242880',*[f'-Wl,--export={x}' for x in exports],'Games/Native/Equalizarr/preview.c','-o','Games/Native/Equalizarr/equalizarr.wasm'],check=True)
