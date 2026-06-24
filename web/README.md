# DOOM HTML launcher

This folder contains a browser-facing HTML canvas launcher for the Doom source tree.

## What is already here

- `index.html` – launcher UI and `<canvas>` viewport
- `style.css` – layout and Doom-ish styling
- `main.js` – IWAD upload handling, start button logic, fullscreen support, boot log
- `doom-loader.js` – placeholder runtime bootstrap that should later be replaced with a real WebAssembly loader

## Current status

This is **not the full browser port yet**. It is the launcher + canvas frontend scaffold for one.

The original Doom release in this repository is native C code. To make it run in the browser, the engine still needs to be compiled to **WebAssembly** (normally with Emscripten), then connected to the launcher.

## Intended flow

1. Open `web/index.html`
2. Upload a local IWAD such as `DOOM.WAD`
3. Press **Start Doom**
4. The browser loader mounts the IWAD in a virtual filesystem
5. The compiled Doom runtime starts and renders into the canvas

## Hooking in the real engine

The simplest approach is:

1. Compile the Doom source with Emscripten
2. Produce a JS/WASM runtime pair
3. Replace the placeholder code in `doom-loader.js` so it:
   - creates the Emscripten module
   - mounts the uploaded IWAD into MEMFS
   - passes the correct command-line arguments to Doom
   - binds rendering/audio/input for the browser

## Notes

- The IWAD is intentionally user-supplied instead of bundled.
- The launcher does not ship copyrighted Doom game data.
- You can later extend this with PWAD support, settings, save persistence, and touch controls.
