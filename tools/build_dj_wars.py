"""Build the allocation-free DJ Wars C simulation and renderer for browser preview."""
import subprocess
exports=['game_demo','game_reset','game_tick','game_frame','game_buffer','game_weapon','game_target','game_value','game_camera','game_audio_buffer','game_audio_render']
subprocess.run(['clang','--target=wasm32','-O2','-ffreestanding','-fno-builtin','-nostdlib','-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=16777216','-Wl,--max-memory=16777216',*[f'-Wl,--export={x}' for x in exports],'Games/Native/DJ Wars/preview.c','-o','Games/Native/DJ Wars/dj-wars.wasm'],check=True)
