'use strict';

// El servidor en C++ es la unica fuente de verdad; el navegador solo representa su estado.
const $ = id => document.getElementById(id);
const form = $('configForm');
const notice = $('notice');
const chart = $('throughputChart');
const state = { history: [], lastEvent: '', lastList: '', loading: false, lastSnapshot: null };

function message(text = '') {
  notice.textContent = text;
  notice.classList.toggle('hidden', !text);
}

const scenarios = {
  calm:     {vehicles: 3, arrival: 300, crossing: 160, green: 1100, red: 250, mode: 'fixed'},
  balanced: {vehicles: 6, arrival: 90, crossing: 130, green: 1000, red: 220, mode: 'fixed'},
  rush:     {vehicles: 16, arrival: 35, crossing: 100, green: 850, red: 180, mode: 'adaptive'}
};

function choosePreset(name) {
  const preset = scenarios[name];
  if (!preset || state.lastSnapshot?.running) return;
  for (const [key, value] of Object.entries(preset)) {
    if (key === 'mode') {
      const radio = form.querySelector(`input[name="mode"][value="${value}"]`);
      if (radio) radio.checked = true;
    } else {
      $(key).value = value;
    }
  }
  document.querySelectorAll('.preset').forEach(button =>
    button.classList.toggle('selected', button.dataset.preset === name));
}

for (const button of document.querySelectorAll('.preset')) {
  button.addEventListener('click', () => choosePreset(button.dataset.preset));
}

async function request(path, fields) {
  const opts = {method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}};
  if (fields) opts.body = new URLSearchParams(fields).toString();
  const response = await fetch(path, opts);
  let data = {};
  try { data = await response.json(); } catch { /* respuesta no JSON */ }
  if (!response.ok || data.ok === false) throw new Error(data.error || `Error HTTP ${response.status}`);
  return data;
}

form.addEventListener('submit', async event => {
  event.preventDefault();
  if (state.loading || !form.reportValidity()) return;
  state.loading = true;
  message();
  $('startButton').disabled = true;
  try {
    const data = Object.fromEntries(new FormData(form).entries());
    await request('/api/start', data);
    state.history = [];
    state.lastEvent = '';
    state.lastList = '';
    await refresh();
  } catch (error) { message(`No se pudo iniciar: ${error.message}`); }
  finally {
    state.loading = false;
    if (!state.lastSnapshot?.running) $('startButton').disabled = false;
  }
});

$('stopButton').addEventListener('click', async () => {
  $('stopButton').disabled = true;
  message();
  try { await request('/api/stop'); await refresh(); }
  catch (error) { message(`No se pudo detener: ${error.message}`); }
});

$('resetButton').addEventListener('click', async () => {
  message();
  try {
    await request('/api/reset');
    state.history = [];
    state.lastEvent = '';
    state.lastList = '';
    await refresh();
  } catch (error) { message(error.message); }
});

function fraction(done, total) {
  return total > 0 ? Math.min(100, Math.max(0, 100 * done / total)) : 0;
}

function displayTime(ms) {
  const total = Math.floor(Math.max(0, ms) / 1000);
  const minutes = String(Math.floor(total / 60)).padStart(2, '0');
  const seconds = String(total % 60).padStart(2, '0');
  const tenths = Math.floor((ms % 1000) / 100);
  return `${minutes}:${seconds}.${tenths}`;
}

function renderCars(containerId, number, max = 3) {
  const container = $(containerId);
  const limit = Math.min(max, Math.max(0, number));
  if (container.children.length === limit) return;
  container.replaceChildren();
  for (let i = 0; i < limit; i++) {
    const car = document.createElement('span');
    car.className = 'car';
    container.appendChild(car);
  }
}

function animateCrossing(event) {
  if (!event || event.type !== 'paso') return;
  const car = $('crossingCar');
  car.className = '';
  // Reinicia la animacion CSS entre vehiculos sucesivos.
  void car.offsetHeight;
  car.className = event.axis === 'ns' ? 'crossing-ns' : 'crossing-eo';
}

function renderEvents(events) {
  const signature = events.map(e => `${e.ms}:${e.id}:${e.type}`).join('|');
  if (signature === state.lastList) return;
  state.lastList = signature;
  $('eventCount').textContent = `${events.length} evento${events.length === 1 ? '' : 's'}`;
  const list = $('eventList');
  if (!events.length) {
    list.innerHTML = '<div class="empty-events"><span>◎</span><strong>Sin actividad registrada</strong><p>Inicia una simulación para visualizar los eventos en tiempo real.</p></div>';
    return;
  }
  list.replaceChildren();
  for (const event of events.slice(0, 35)) {
    const row = document.createElement('div'); row.className = 'event-row';
    const icon = document.createElement('div');
    icon.className = `event-icon ${['paso', 'fase', 'fin', 'detenido'].includes(event.type) ? event.type : ''}`;
    icon.textContent = ({paso: '↗', llegada: '↓', fase: '●', inicio: '▶', fin: '✓', detenido: '■'})[event.type] || '•';
    const desc = document.createElement('div'); desc.className = 'event-text';
    desc.textContent = event.text;
    const timer = document.createElement('div'); timer.className = 'event-time';
    timer.textContent = `+${(event.ms / 1000).toFixed(2)} s`;
    row.append(icon, desc, timer);
    list.appendChild(row);
  }
}

function drawChart() {
  const ctx = chart.getContext('2d');
  if (!ctx) return;
  const bounds = chart.getBoundingClientRect();
  const width = Math.max(170, bounds.width), height = 140;
  const ratio = window.devicePixelRatio || 1;
  chart.width = Math.floor(width * ratio);
  chart.height = Math.floor(height * ratio);
  ctx.scale(ratio, ratio);
  ctx.clearRect(0, 0, width, height);
  const pad = {left: 23, top: 13, right: 8, bottom: 26};
  const innerW = width - pad.left - pad.right, innerH = height - pad.top - pad.bottom;
  const max = Math.max(4, Math.ceil(Math.max(...state.history.map(p => p.total), 0) / 4) * 4);
  ctx.strokeStyle = '#2b3b50'; ctx.lineWidth = 1;
  ctx.fillStyle = '#7890ac'; ctx.font = '10px ui-sans-serif, sans-serif';
  for (let i = 0; i <= 4; i++) {
    const y = pad.top + innerH * i / 4;
    ctx.beginPath(); ctx.moveTo(pad.left, y); ctx.lineTo(width - pad.right, y); ctx.stroke();
    ctx.fillText(String(Math.round(max * (4 - i) / 4)), 0, y + 3);
  }
  if (!state.history.length) return;
  const h = state.history;
  const minT = h[0].time, maxT = Math.max(h.at(-1).time, minT + 1);
  const pt = p => ({
    x: pad.left + (p.time - minT) / (maxT - minT) * innerW,
    y: pad.top + innerH * (1 - p.total / max)
  });
  ctx.lineJoin = 'round'; ctx.lineCap = 'round'; ctx.lineWidth = 2.4;
  ctx.strokeStyle = '#34d5b3'; ctx.beginPath();
  h.forEach((item, i) => {
    const p = pt(item);
    if (i === 0) ctx.moveTo(p.x, p.y);
    else ctx.lineTo(p.x, p.y);
  });
  ctx.stroke();
  const last = pt(h.at(-1));
  ctx.beginPath(); ctx.arc(last.x, last.y, 4, 0, Math.PI * 2);
  ctx.fillStyle = '#2ddcba'; ctx.fill();
  ctx.fillStyle = '#7186a0';
  ctx.fillText('0 s', pad.left, height - 6);
  ctx.fillText(`${(maxT / 1000).toFixed(1)} s`, Math.max(pad.left, width - 43), height - 6);
}

function recordChart(snapshot) {
  if (!snapshot.running && !snapshot.finished && !snapshot.served) {
    if (state.history.length) { state.history = []; drawChart(); }
    return;
  }
  const last = state.history.at(-1);
  const point = {time: snapshot.elapsed_ms, total: snapshot.served};
  if (!last || point.time - last.time >= 240 || point.total !== last.total) {
    state.history.push(point);
    if (state.history.length > 90) state.history.shift();
    drawChart();
  }
}

function render(snapshot) {
  state.lastSnapshot = snapshot;
  const working = snapshot.running;
  const finished = snapshot.finished;
  const badge = $('systemBadge');
  badge.className = `status-pill ${working ? 'running' : finished ? 'finished' : ''}`;
  $('systemText').textContent = working ? 'En ejecución' : snapshot.error ? 'Error de ejecución' :
    finished ? (snapshot.stopped ? 'Detenida' : 'Completada') : 'En espera';
  $('generated').textContent = snapshot.generated;
  $('served').textContent = snapshot.served;
  $('pending').textContent = snapshot.pending;
  $('avgWait').textContent = Math.round(snapshot.avg_wait_ms);
  const expected = (snapshot.config?.vehicles || 6) * 2;
  const progress = fraction(snapshot.served, expected);
  $('progressLabel').textContent = `${Math.round(progress)} % completado`;
  $('progressFill').style.width = `${progress}%`;
  $('nsCount').textContent = `${snapshot.ns.served} / ${snapshot.ns.generated}`;
  $('eoCount').textContent = `${snapshot.eo.served} / ${snapshot.eo.generated}`;
  $('nsServed').textContent = snapshot.ns.served;
  $('eoServed').textContent = snapshot.eo.served;
  $('nsWait').textContent = `${Math.round(snapshot.ns.wait_ms)} ms`;
  $('eoWait').textContent = `${Math.round(snapshot.eo.wait_ms)} ms`;
  $('nsBar').style.width = `${fraction(snapshot.ns.served, Math.max(snapshot.ns.generated, 1))}%`;
  $('eoBar').style.width = `${fraction(snapshot.eo.served, Math.max(snapshot.eo.generated, 1))}%`;
  $('elapsed').textContent = displayTime(snapshot.elapsed_ms);
  $('lightNS').classList.toggle('go', snapshot.phase === 'ns');
  $('lightEO').classList.toggle('go', snapshot.phase === 'eo');
  $('phaseText').textContent = snapshot.phase === 'ns' ? 'Norte – Sur tiene luz verde' :
    snapshot.phase === 'eo' ? 'Este – Oeste tiene luz verde' : 'Ambos semáforos en rojo';
  $('phaseBadge').textContent = snapshot.phase === 'ns' ? '● PASO N–S' :
    snapshot.phase === 'eo' ? '● PASO E–O' : '● EN ESPERA';
  $('phaseBadge').classList.toggle('active', snapshot.phase !== 'rojo');
  const waitingNS = Math.max(0, snapshot.ns.generated - snapshot.ns.served);
  const waitingEO = Math.max(0, snapshot.eo.generated - snapshot.eo.served);
  renderCars('carsNorth', Math.ceil(waitingNS / 2));
  renderCars('carsSouth', Math.floor(waitingNS / 2));
  renderCars('carsEast', Math.ceil(waitingEO / 2));
  renderCars('carsWest', Math.floor(waitingEO / 2));
  $('startButton').disabled = working || state.loading;
  $('stopButton').disabled = !working;
  $('resetButton').disabled = working;
  form.querySelectorAll('input').forEach(input => { input.disabled = working; });
  document.querySelectorAll('.preset').forEach(button => { button.disabled = working; });
  if (snapshot.error) message(snapshot.error);
  if (snapshot.events?.length) {
    const newestPass = snapshot.events.find(e => e.type === 'paso');
    if (newestPass) {
      const key = `${newestPass.id}:${newestPass.ms}`;
      if (state.lastEvent && state.lastEvent !== key) animateCrossing(newestPass);
      state.lastEvent = key;
    }
  }
  renderEvents(snapshot.events || []);
  recordChart(snapshot);
}

async function refresh() {
  try {
    const response = await fetch('/api/status', {cache: 'no-store'});
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    render(await response.json());
  } catch (error) {
    message(`Sin conexión con el motor C++: ${error.message}`);
    $('systemBadge').className = 'status-pill';
    $('systemText').textContent = 'Sin conexión';
  }
}

window.addEventListener('resize', drawChart);
function tickClock() { $('clockNow').textContent = new Date().toLocaleTimeString('es-PE', {hour12: false}); }
tickClock();
setInterval(tickClock, 1000);
refresh();
setInterval(refresh, 300);
