CLANG ?= clang
EXPORTS = arcade_reset arcade_tick arcade_tile arcade_value arcade_actor arcade_audio
.PHONY: all test arm native package serve
all: cat-scenes space-invaders basslines equalizarr asteroids dj-wars Games/Native/Pacman/pacman.wasm
Games/Native/Pacman/pacman_core.c: vendor/pacman-upstream.c tools/extract_core.py
	python3 tools/extract_core.py
Games/Native/Pacman/pacman.wasm: Games/Native/Pacman/pacman_core.c Games/Native/Pacman/pacman_sound_data.h
	$(CLANG) -O2 -ffreestanding -fno-builtin --target=wasm32 -nostdlib -Wl,--no-entry $(foreach e,$(EXPORTS),-Wl,--export=$(e)) $< -o $@
work/pacman-core.so: Games/Native/Pacman/pacman_core.c Games/Native/Pacman/pacman_sound_data.h
	mkdir -p work
	$(CLANG) -O2 -shared -fPIC -fno-builtin $< -o $@
test: work/pacman-core.so
	python3 tests/test_core.py
arm: Games/Native/Pacman/pacman_core.c
	mkdir -p work
	$(CLANG) -O2 -ffreestanding -fno-builtin --target=arm-linux-gnueabi -march=armv7-a -mfloat-abi=softfp -c $< -o work/pacman-armv7.o
serve:
	python3 -m http.server 8765 --bind 127.0.0.1

native: cat-scenes space-invaders basslines equalizarr asteroids credits-scene debug-artwork music-animation farewell-artwork welcome-sound Games/Native/Pacman/pacman_core.c Games/Native/Pacman/pacman_sound_data.h
	python3 tools/build_help.py
	sh tools/build_native.sh
package: native
	python3 tools/package_native.py

test-native:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_native.c Games/Native/Pacman/pacman_core.c -o work/test-native
	work/test-native

test-launcher:
	$(CLANG) -O2 tests/test_launcher.c -o work/test-launcher
	work/test-launcher

test-banner:
	$(CLANG) -O2 -ffunction-sections -Wl,--gc-sections tests/test_banner.c -o work/test-banner
	work/test-banner

test-overlay:
	$(CLANG) -O2 tests/test_overlay.c -o work/test-overlay
	work/test-overlay

.PHONY: test-panel
test-panel:
	$(CLANG) -O2 tests/test_panel.c -o work/test-panel
	work/test-panel

.PHONY: test-session test-usb
test-session:
	python3 tests/test_session.py
test-usb:
	python3 tests/test_usb_cleanup.py

.PHONY: test-eject
test-eject:
	python3 tests/test_eject.py

.PHONY: test-nes-controls
test-nes-controls:
	$(CLANG) -O2 tests/test_nes_controls.c -o work/test-nes-controls
	work/test-nes-controls

.PHONY: test-bootstrap
test-bootstrap:
	python3 tests/test_bootstrap.py

.PHONY: test-debug
test-debug:
	$(CLANG) -O2 tests/test_debug.c -o work/test-debug
	work/test-debug

.PHONY: test-ssh
test-ssh:
	$(CLANG) -O2 tests/test_ssh_root_permissions.c -o work/test-ssh-root-permissions
	work/test-ssh-root-permissions
	python3 tests/test_ssh.py

.PHONY: test-menu-sound menu-music
test-menu-sound:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_menu_sound.c -pthread -o work/test-menu-sound
	work/test-menu-sound
menu-music:
	python3 tools/build_menu_music.py

.PHONY: test-music-player
test-music-player:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_music_player.c -o work/test-music-player
	work/test-music-player

.PHONY: farewell-sound test-farewell
farewell-sound:
	python3 tools/build_farewell_sound.py
test-farewell:
	$(CLANG) -O2 tests/test_farewell.c -o work/test-farewell
	work/test-farewell

.PHONY: welcome-sound
welcome-sound:
	python3 tools/build_welcome_sound.py

.PHONY: farewell-artwork
farewell-artwork:
	python3 tools/build_farewell_artwork.py

.PHONY: music-animation
music-animation:
	python3 tools/build_music_animation.py

.PHONY: debug-artwork
debug-artwork:
	python3 tools/build_debug_artwork.py

.PHONY: credits-scene
credits-scene: shared/cafe_chibi_atlas.h shared/cafe_dragon_atlas.h
	python3 tools/build_credits.py

.PHONY: test-credits
test-credits:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_credits.c -o work/test-credits
	work/test-credits

.PHONY: asteroids test-asteroids
asteroids:
	python3 tools/build_asteroids.py
test-asteroids:
	mkdir -p work
	$(CLANG) -O2 tests/test_asteroids.c -o work/test-asteroids
	work/test-asteroids

.PHONY: warmux warmux-prepare test-warmux
warmux-prepare:
	python3 tools/prepare_warmux.py
warmux:
	python3 tools/build_warmux.py
test-warmux:
	python3 tools/test_warmux.py

.PHONY: dj-wars test-dj-wars
dj-wars:
	python3 tools/build_dj_wars.py
test-dj-wars:
	mkdir -p work
	$(CLANG) -O2 tests/test_dj_wars.c -o work/test-dj-wars
	work/test-dj-wars

.PHONY: test-dj-wars-render
test-dj-wars-render:
	$(CLANG) -O2 tests/test_dj_wars_render.c -o work/test-dj-wars-render
	work/test-dj-wars-render

.PHONY: equalizarr test-equalizarr
equalizarr:
	python3 tools/build_equalizarr.py
test-equalizarr:
	python3 tests/test_equalizarr_levels.py
	$(CLANG) -O2 tests/test_equalizarr.c -o work/test-equalizarr
	work/test-equalizarr

.PHONY: basslines test-basslines
basslines:
	python3 tools/build_basslines.py
test-basslines:
	mkdir -p work
	$(CLANG) -O2 tests/test_basslines.c -o work/test-basslines
	work/test-basslines

.PHONY: space-invaders test-space-invaders
space-invaders:
	python3 tools/build_space_invaders.py
test-space-invaders:
	mkdir -p work
	$(CLANG) -O2 tests/test_space_invaders.c -o work/test-space-invaders
	work/test-space-invaders

.PHONY: test-game-session test-dj-wars-audio
test-game-session:
	node tests/test_session_state.cjs
test-dj-wars-audio:
	$(CLANG) -O2 tests/test_dj_wars_audio.c -o work/test-dj-wars-audio
	work/test-dj-wars-audio

.PHONY: cat-scenes test-cat-scenes
shared/cat_atlas.h: tools/build_cat_scenes.py $(wildcard assets/cats/*.png) assets/music/no-cover-cat.gif
	python3 tools/build_cat_scenes.py
shared/cafe_chibi_atlas.h: tools/build_cafe_chibi.py $(wildcard assets/cats/chibi/*.png)
	python3 tools/build_cafe_chibi.py
shared/cafe_room_art.h: tools/build_cafe_room.py assets/cafe/midnight-room-v1.png
	python3 tools/build_cafe_room.py
shared/cafe_dragon_atlas.h: tools/build_cafe_dragon.py assets/cafe/dragon-chibi-ref-v2.png
	python3 tools/build_cafe_dragon.py
cat-scenes: shared/cat_atlas.h shared/cafe_chibi_atlas.h shared/cafe_room_art.h shared/cafe_dragon_atlas.h
	python3 tools/build_cat_preview.py

shared/cafe_show.h: tools/build_cafe_show.py assets/music/cafe-good.mp3
	python3 tools/build_cafe_show.py
.PHONY: cafe-show test-cafe-show
cafe-show: shared/cafe_show.h
	python3 tools/build_cafe_show_audio.py
cat-scenes native: shared/cafe_show.h
test-cafe-show: cafe-show
	$(CLANG) -O2 tests/test_cafe_show_audio.c -o work/test-cafe-show-audio
	work/test-cafe-show-audio
	node tests/test_cafe_show_audio.js
	$(CLANG) -O2 tests/test_cafe_show_leds.c -o work/test-cafe-show-leds
	work/test-cafe-show-leds
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_cafe_show_integration.c Games/Native/Pacman/pacman_core.c -o work/test-cafe-show-integration -lpthread
	work/test-cafe-show-integration
space-invaders basslines equalizarr asteroids: shared/cafe_chibi_atlas.h shared/cafe_dragon_atlas.h shared/cafe_room_art.h
test-cat-scenes: shared/cat_atlas.h shared/cafe_chibi_atlas.h shared/cafe_room_art.h shared/cafe_dragon_atlas.h
	$(CLANG) -O2 tests/test_cat_scenes.c -o work/test-cat-scenes
	node tests/test_rescue_wasm.cjs
	work/test-cat-scenes

.PHONY: rescue-audio
rescue-audio:
	python3 tools/build_rescue_audio.py

.PHONY: test-game-music
test-game-music: rescue-audio
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_game_music.c -o work/test-game-music
	work/test-game-music
	node tests/test_game_music.js

.PHONY: test-dj-wars-finale
test-dj-wars-finale:
	$(CLANG) -O2 tests/test_dj_wars_finale.c -o work/test-dj-wars-finale
	work/test-dj-wars-finale

shared/dj_master_atlas.h: tools/build_dj_master.py assets/dj-wars/guetta-chibi-v1.png
	python3 tools/build_dj_master.py
native dj-wars test-dj-wars-finale test-dj-wars-render test-native: shared/dj_master_atlas.h

.PHONY: test-dj-wars-fireworks test-cafe-music
test-dj-wars-fireworks:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_dj_wars_fireworks.c Games/Native/Pacman/pacman_core.c -o work/test-dj-wars-fireworks
	work/test-dj-wars-fireworks
test-cafe-music:
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_cafe_music.c Games/Native/Pacman/pacman_core.c -o work/test-cafe-music -lasound -lpthread
	work/test-cafe-music

.PHONY: test-persistent-save
test-persistent-save:
	$(CLANG) -O2 tests/test_persistent_save.c -pthread -o work/test-persistent-save
	work/test-persistent-save
	$(CLANG) -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections tests/test_persistent_integration.c Games/Native/Pacman/pacman_core.c -pthread -o work/test-persistent-integration
	work/test-persistent-integration
