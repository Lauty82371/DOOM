const canvas = document.getElementById('doomCanvas');
const ctx = canvas.getContext('2d');
const iwadInput = document.getElementById('iwadInput');
const wadStatus = document.getElementById('wadStatus');
const startBtn = document.getElementById('startBtn');
const fullscreenBtn = document.getElementById('fullscreenBtn');
const overlay = document.getElementById('overlay');
const logEl = document.getElementById('log');

let iwadFile = null;
let doomRuntime = null;

function log(message) {
  const timestamp = new Date().toLocaleTimeString();
  logEl.textContent += `[${timestamp}] ${message}\n`;
  logEl.scrollTop = logEl.scrollHeight;
}

function drawBootScreen(message = 'DOOM HTML') {
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  ctx.fillStyle = '#8d0d0d';
  ctx.fillRect(0, canvas.height - 72, canvas.width, 72);

  ctx.fillStyle = '#fff';
  ctx.font = 'bold 42px Arial';
  ctx.textAlign = 'center';
  ctx.fillText(message, canvas.width / 2, canvas.height / 2 - 10);

  ctx.font = '18px Arial';
  ctx.fillStyle = '#ffcf7a';
  ctx.fillText('Select an IWAD to start the engine', canvas.width / 2, canvas.height / 2 + 28);
}

async function readFileAsUint8Array(file) {
  const buffer = await file.arrayBuffer();
  return new Uint8Array(buffer);
}

async function handleIwadSelection(file) {
  if (!file) return;
  iwadFile = file;
  wadStatus.textContent = `Selected: ${file.name} (${Math.round(file.size / 1024)} KB)`;
  startBtn.disabled = false;
  log(`IWAD selected: ${file.name}`);
  drawBootScreen(file.name);
}

iwadInput.addEventListener('change', async (event) => {
  const file = event.target.files?.[0];
  await handleIwadSelection(file);
});

fullscreenBtn.addEventListener('click', async () => {
  if (document.fullscreenElement) {
    await document.exitFullscreen();
  } else {
    await canvas.requestFullscreen();
  }
});

startBtn.addEventListener('click', async () => {
  if (!iwadFile) {
    log('Start blocked: no IWAD selected.');
    return;
  }

  startBtn.disabled = true;
  overlay.classList.add('hidden');
  log('Preparing IWAD in memory...');

  const iwadBytes = await readFileAsUint8Array(iwadFile);

  if (!window.DoomLoader || typeof window.DoomLoader.start !== 'function') {
    log('doom-loader.js not found or missing start().');
    log('The launcher is ready, but you still need to compile the Doom engine to WebAssembly and expose DoomLoader.start().');
    fullscreenBtn.disabled = false;
    return;
  }

  try {
    doomRuntime = await window.DoomLoader.start({
      canvas,
      iwadName: iwadFile.name,
      iwadBytes,
      onStatus: (message) => log(message),
    });

    fullscreenBtn.disabled = false;
    log('Doom engine started.');
  } catch (error) {
    console.error(error);
    log(`Engine failed to start: ${error?.message || error}`);
    startBtn.disabled = false;
  }
});

drawBootScreen();
log('Launcher ready. Waiting for IWAD selection.');
