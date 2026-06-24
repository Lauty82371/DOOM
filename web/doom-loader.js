window.DoomLoader = {
  async start({ canvas, iwadName, iwadBytes, onStatus }) {
    onStatus?.(`Received IWAD ${iwadName} (${iwadBytes.length} bytes).`);
    onStatus?.('This is a bootstrap stub. Replace web/doom-loader.js with the Emscripten-generated loader or make it call into your compiled Doom runtime.');

    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 36px Arial';
    ctx.textAlign = 'center';
    ctx.fillText('DOOM ENGINE PLACEHOLDER', canvas.width / 2, canvas.height / 2 - 10);

    ctx.font = '18px Arial';
    ctx.fillStyle = '#ffcf7a';
    ctx.fillText('Compile the C source to WebAssembly and mount the IWAD in MEMFS.', canvas.width / 2, canvas.height / 2 + 28);

    return {
      stop() {
        onStatus?.('Engine stopped.');
      },
    };
  },
};
