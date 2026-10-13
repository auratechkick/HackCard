/** HackCard REST API client — matches firmware WebStatusServer routes */
const HackCardApi = (() => {
  const base = () => localStorage.getItem('hackcard_base') || '';

  function labSessionToken() {
    const fromUrl = new URLSearchParams(window.location.search).get('st');
    if (fromUrl) {
      sessionStorage.setItem('hackcard_lab_st', fromUrl);
      localStorage.setItem('hackcard_lab_st', fromUrl);
      return fromUrl;
    }
    return sessionStorage.getItem('hackcard_lab_st') || localStorage.getItem('hackcard_lab_st') || '';
  }

  function withLabSession(path, body = '') {
    const st = labSessionToken();
    if (!st) {
      return { path, body };
    }
    const sep = path.includes('?') ? '&' : '?';
    const pathWithSt = `${path}${sep}st=${encodeURIComponent(st)}`;
    const stField = `st=${encodeURIComponent(st)}`;
    const bodyWithSt = body ? (body.includes('st=') ? body : `${body}&${stField}`) : stField;
    return { path: pathWithSt, body: bodyWithSt };
  }

  async function post(path, body = '') {
    const auth = withLabSession(path, body);
    const res = await fetch(base() + auth.path, {
      method: 'POST',
      credentials: 'include',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: auth.body
    });
    const text = await res.text();
    try {
      const data = JSON.parse(text);
      data.ok = res.ok;
      data.httpStatus = res.status;
      return data;
    } catch {
      return { raw: text, ok: res.ok, httpStatus: res.status };
    }
  }

  /** POST raw text body; pin/name/etc. go in query string (reliable for multiline DuckyScript). */
  async function postTextQuery(path, queryObj, textBody = '') {
    const q = new URLSearchParams(queryObj).toString();
    const pathWithQ = q ? `${path}?${q}` : path;
    const auth = withLabSession(pathWithQ, '');
    const res = await fetch(base() + auth.path, {
      method: 'POST',
      credentials: 'include',
      headers: { 'Content-Type': 'text/plain; charset=utf-8' },
      body: textBody
    });
    const text = await res.text();
    try { return JSON.parse(text); } catch { return { raw: text, ok: res.ok }; }
  }

  async function get(path, asText = false) {
    const auth = withLabSession(path, '');
    const res = await fetch(base() + auth.path, { credentials: 'include' });
    if (asText) return res.text();
    const text = await res.text();
    try {
      return JSON.parse(text);
    } catch {
      try {
        return JSON.parse(text.trim());
      } catch {
        return { raw: text, ok: res.ok };
      }
    }
  }

  const enc = (obj) => Object.entries(obj).map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v ?? '')}`).join('&');

  return {
    base,
    setBase(url) { localStorage.setItem('hackcard_base', url.replace(/\/$/, '')); },

    status: () => get('/api/status'),
    setup: (p) => post('/api/setup', enc(p)),
    profile: (p) => post('/api/profile', enc(p)),
    cli: (cmd) => post('/api/cli', `cmd=${encodeURIComponent(cmd)}`),
    cliBanner: () => get('/api/cli/banner', true),
    serialLog: async (since = 0) => {
      const res = await fetch(`${base()}/api/log/serial?since=${since}`);
      const cursor = parseInt(res.headers.get('X-Log-Cursor') || since, 10);
      return { text: await res.text(), cursor: Number.isFinite(cursor) ? cursor : since };
    },

    // Wi-Fi
    wifiScan: () => post('/api/wifi/scan'),
    wifiScanClear: () => post('/api/wifi/scan/clear'),
    wifiLast: () => get('/api/wifi/last'),
    wifiStaGet: (pin) => post('/api/wifi/sta', `pin=${encodeURIComponent(pin)}`),
    wifiApGet: () => get('/api/wifi/ap'),
    wifiApSet: (p) => post('/api/wifi/ap', enc(p)),
    wifiBeaconStatus: () => get('/api/wifi/beacon'),
    wifiBeaconStart: (p) => post('/api/wifi/beacon/start', enc(p)),
    wifiBeaconStop: () => post('/api/wifi/beacon/stop'),
    wifiTwinStatus: () => get('/api/wifi/twin'),
    wifiTwinStart: (p) => post('/api/wifi/twin/start', enc(p)),
    wifiTwinStop: () => post('/api/wifi/twin/stop'),
    wifiTrainingStatus: () => get('/api/wifi/training'),
    wifiTrainingConfig: (p) => post('/api/wifi/training/config', enc(p)),
    wifiTrainingStart: (pin) => post('/api/wifi/training/start', `pin=${encodeURIComponent(pin)}`),
    wifiTrainingStop: () => post('/api/wifi/training/stop'),
    wifiTrainingCaptures: () => get('/api/wifi/training/captures', true),
    wifiTrainingCapturesClear: () => post('/api/wifi/training/captures/clear'),
    wifiTrainingExportSd: () => post('/api/wifi/training/export/sd'),
    wifiMonitorStatus: () => get('/api/wifi/monitor'),
    wifiMonitorProbes: () => get('/api/wifi/monitor/probes'),
    wifiMonitorSniffStart: (p) => post('/api/wifi/monitor/sniff/start', enc(p)),
    wifiMonitorSniffStop: () => post('/api/wifi/monitor/sniff/stop'),
    wifiMonitorDeauthStart: (p) => post('/api/wifi/monitor/deauth/start', enc(p)),
    wifiMonitorDeauthStop: () => post('/api/wifi/monitor/deauth/stop'),
    wifiMonitorProbesClear: () => post('/api/wifi/monitor/probes/clear'),
    wifiMonitorProbesExportSd: () => post('/api/wifi/monitor/probes/export/sd'),
    wifiMonitorAps: () => get('/api/wifi/monitor/aps'),
    wifiMonitorLive: () => get('/api/wifi/monitor/live'),
    wifiKarmaStatus: () => get('/api/wifi/karma'),
    wifiKarmaStart: (p) => post('/api/wifi/karma/start', enc(p)),
    wifiKarmaStop: () => post('/api/wifi/karma/stop'),
    wifiCaptureStatus: () => get('/api/wifi/capture'),
    wifiCaptureStart: (p) => post('/api/wifi/capture/start', enc(p)),
    wifiCaptureStop: () => post('/api/wifi/capture/stop'),
    wifiCaptureVerify: (password) => post('/api/wifi/capture/verify', `password=${encodeURIComponent(password)}`),
    wifiCaptureExportSd: () => post('/api/wifi/capture/export/sd'),

    // NFC
    nfcStatus: () => get('/api/nfc/status'),
    nfcRead: (p) => post('/api/nfc/read', p ? enc(p) : ''),
    nfcInfo: () => post('/api/nfc/info'),
    nfcDump: (p) => post('/api/nfc/dump', p ? enc(p) : ''),
    nfcWrite: (p) => post('/api/nfc/write', enc(p)),
    nfcHistory: () => get('/api/nfc/history', true),
    nfcLastDump: () => get('/api/nfc/last-dump', true),
    nfcErase: (pin) => post('/api/nfc/erase', `pin=${encodeURIComponent(pin)}`),
    nfcClone: (p) => post('/api/nfc/clone', enc(p)),
    nfcKill: (pin) => post('/api/nfc/kill', `pin=${encodeURIComponent(pin)}`),
    nfcClassicDump: (p) => post('/api/nfc/classic/dump', typeof p === 'string' ? `pin=${encodeURIComponent(p)}` : enc(p)),
    nfcClassicClone: (pin) => post('/api/nfc/classic/clone', `pin=${encodeURIComponent(pin)}`),
    nfcKeysScan: (pin) => post('/api/nfc/keys/scan', `pin=${encodeURIComponent(pin)}`),
    nfcStoragePrefs: (p) => post('/api/nfc/storage/prefs', enc(p)),
    nfcHistoryExportSd: () => post('/api/nfc/history/export/sd'),
    nfcDumpExportSd: () => post('/api/nfc/dump/export/sd'),
    nfcClassicExportSd: () => post('/api/nfc/classic/export/sd'),

    // Peripherals
    apps: (p) => post('/api/apps', enc(p)),
    settings: (p) => post('/api/settings', enc(p)),
    logs: () => get('/api/logs', true),
    logsClear: () => post('/api/logs/clear'),
    diagRun: () => post('/api/diag/run'),
    modeSet: (mode) => post('/api/mode', `mode=${mode}`),
    modes: () => get('/api/modes'),
    labUnlock: (pin) => post('/api/lab/unlock', `pin=${encodeURIComponent(pin)}`),

    // BLE
    bleStatus: () => get('/api/ble/status'),
    bleScan: () => post('/api/ble/scan'),
    bleScanLast: () => get('/api/ble/scan/last'),
    bleAdvertiseStart: (p) => post('/api/ble/advertise/start', enc(p)),
    bleAdvertiseStop: () => post('/api/ble/advertise/stop'),
    bleContactStart: () => post('/api/ble/contact/start'),
    bleContactStop: () => post('/api/ble/contact/stop'),
    bleContactRefresh: () => post('/api/ble/contact/refresh'),

    // HID
    hidStatus: () => get('/api/hid/status'),
    hidDevice: (p) => post('/api/hid/device', enc(p)),
    hidOs: (os) => post('/api/hid/os', `os=${os}`),
    hidRun: (p) => post('/api/hid/run', enc(p)),
    hidPayloads: () => get('/api/hid/payloads'),
    hidPayloadRead: (name) => get(`/api/hid/payload?name=${encodeURIComponent(name)}`),
    hidPayloadSave: (p) => postTextQuery('/api/hid/payload/save', { pin: p.pin, name: p.name }, p.content ?? ''),
    hidPayloadSaveSd: (p) => postTextQuery('/api/hid/payload/save/sd', { pin: p.pin, name: p.name }, p.content ?? ''),
    hidPayloadDelete: (p) => post('/api/hid/payload/delete', enc(p)),
    hidRunScript: (p) => postTextQuery('/api/hid/run-script', { pin: p.pin, name: p.name || '', os: p.os || '' }, p.content ?? ''),
    hidArm: (p) => post('/api/hid/arm', enc(p)),
    hidDisarm: () => post('/api/hid/arm', 'disarm=1'),
  };
})();
