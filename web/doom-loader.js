window.DoomLoader = {
  async start({ canvas, iwadName, iwadBytes, onStatus }) {
    onStatus?.(`Received IWAD ${iwadName} (${iwadBytes.length} bytes).`);

    if (typeof window.DoomModuleFactory !== 'function') {
      throw new Error('doom.js / DoomModuleFactory not found. Run web/build-wasm.sh first to generate web/dist/doom.js and doom.wasm.');
    }

    const module = await window.DoomModuleFactory({
      canvas,
      print: (text) => onStatus?.(String(text)),
      printErr: (text) => onStatus?.(`[stderr] ${text}`),
      locateFile: (path) => `./dist/${path}`,
      preRun: [function () {
        try {
          if (!module.FS.analyzePath('/doom1.wad').exists) {
            module.FS.writeFile(`/${iwadName}`, iwadBytes);
          } else {
            module.FS.unlink('/doom1.wad');
            module.FS.writeFile(`/${iwadName}`, iwadBytes);
          }
        } catch (error) {
          console.error(error);
        }
      }],
    });

    onStatus?.('Starting Doom main()...');
    module.callMain(['-iwad', `/${iwadName}`]);

    return {
      stop() {
        onStatus?.('Stopping Doom runtime is not implemented yet; reload the page to reset the module.');
      },
      module,
    };
  },
};
