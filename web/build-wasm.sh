#!/usr/bin/env bash
set -euo pipefail

# Build script for compiling the original linuxdoom-1.10 source to WebAssembly.
# Requires Emscripten in PATH (emcc / emmake).

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SRC_DIR="$ROOT_DIR/linuxdoom-1.10"
OUT_DIR="$ROOT_DIR/web/dist"
mkdir -p "$OUT_DIR"

COMMON_FLAGS=(
  -O2
  -sALLOW_MEMORY_GROWTH=1
  -sASSERTIONS=1
  -sUSE_SDL=2
  -sWASM=1
  -sEXPORTED_RUNTIME_METHODS=FS,callMain
  -sEXPORTED_FUNCTIONS=_main
  -sENVIRONMENT=web
  -sMODULARIZE=1
  -sEXPORT_NAME=DoomModuleFactory
  -sNO_EXIT_RUNTIME=1
)

# Core Doom sources from the original Makefile, minus the native Linux platform layer.
DOOM_SRCS=(
  doomdef.c doomstat.c dstrings.c tables.c f_finale.c f_wipe.c d_main.c d_net.c d_items.c
  g_game.c m_menu.c m_misc.c m_argv.c m_bbox.c m_fixed.c m_swap.c m_cheat.c m_random.c
  am_map.c p_ceilng.c p_doors.c p_enemy.c p_floor.c p_inter.c p_lights.c p_map.c p_maputl.c
  p_plats.c p_pspr.c p_setup.c p_sight.c p_spec.c p_switch.c p_mobj.c p_telept.c p_tick.c
  p_saveg.c p_user.c r_bsp.c r_data.c r_draw.c r_main.c r_plane.c r_segs.c r_sky.c r_things.c
  w_wad.c wi_stuff.c v_video.c st_lib.c st_stuff.c hu_stuff.c hu_lib.c s_sound.c z_zone.c
  info.c sounds.c
)

pushd "$SRC_DIR" >/dev/null
emcc \
  "${DOOM_SRCS[@]}" \
  i_main.c \
  i_emscripten.c \
  -I. \
  "${COMMON_FLAGS[@]}" \
  --js-library ../web/doom-browser.js \
  -o "$OUT_DIR/doom.js"
popd >/dev/null

echo "Built browser runtime to $OUT_DIR/doom.js and $OUT_DIR/doom.wasm"
