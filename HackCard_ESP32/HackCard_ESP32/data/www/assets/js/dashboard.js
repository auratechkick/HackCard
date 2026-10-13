/** HackCard Lab Console — unified dashboard */
(() => {
  const $ = (sel, root = document) => root.querySelector(sel);
  const $$ = (sel) => [...document.querySelectorAll(sel)];
  const pin = () => ($('#set-lab-pin')?.value || $('#labPinMobile')?.value || $('#labPin')?.value || '1234').trim();
  const msg = (el, text, ok) => { if (el) { el.textContent = text; el.className = 'msg ' + (ok ? 'ok' : 'err'); } };

  const MORE_PAGES = ['training', 'monitor', 'peripherals', 'hid', 'ble', 'terminal', 'settings'];
  const PATH_TO_PAGE = {
    '/': 'dashboard',
    '/setup': 'settings',
    '/settings': 'settings',
    '/wifi': 'wifi',
    '/wifi/ap': 'settings',
    '/wifi/training': 'training',
    '/nfc': 'nfc',
    '/ble': 'ble',
    '/hid': 'hid',
    '/terminal': 'terminal',
    '/diag': 'settings',
    '/apps': 'peripherals',
    '/logs': 'settings',
    '/modes': 'settings',
    '/lab': 'dashboard',
    '/profile': 'settings',
    '/profile/edit': 'settings',
    '/monitor': 'monitor'
  };
  const SETTINGS_TAB_FROM_PATH = {
    '/setup': 'setup',
    '/settings': 'general',
    '/profile': 'profile',
    '/profile/edit': 'profile',
    '/wifi/ap': 'ap',
    '/modes': 'mode'
  };
  const PAGE_TITLES = {
    dashboard: 'Home', wifi: 'Wi-Fi Lab', training: 'Training', monitor: 'Monitor',
    nfc: 'NFC', peripherals: 'RGB & Buzzer', hid: 'USB HID', ble: 'Bluetooth',
    terminal: 'Terminal', settings: 'Settings'
  };

  function escapeHtml(text) {
    return String(text ?? '').replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  function renderMobileCards(container, rows, emptyMsg) {
    if (!container) return;
    if (!rows.length) {
      container.innerHTML = `<div class="list-card-empty">${emptyMsg}</div>`;
      return;
    }
    container.innerHTML = rows.map(r => `
      <div class="list-card">
        <div class="list-card-title">${escapeHtml(r.title)}</div>
        <div class="list-card-meta">${r.meta.map(m => `<span>${escapeHtml(m)}</span>`).join('')}</div>
      </div>`).join('');
  }

  function setConnDots(online) {
    document.querySelectorAll('#conn-dot, #conn-dot-mobile').forEach(dot => {
      dot.classList.toggle('online', online);
      dot.classList.toggle('off', !online);
    });
    document.querySelectorAll('.sidebar-conn, .mobile-topbar').forEach(el => {
      el.classList.toggle('is-offline', !online);
    });
    const label = online ? 'Online' : 'Offline';
    const side = $('#sidebar-status');
    if (side) side.textContent = label;
  }

  function showSettingsReconnectNotice(htmlText) {
    const box = $('#settings-reconnect-notice');
    if (!box) return;
    box.innerHTML = htmlText;
    box.hidden = false;
    box.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  }

  function wifiReconnectNoticeHtml(newSsid) {
    const ssid = escapeHtml(newSsid || 'your HackCard network');
    return `<strong>Wi‑Fi access point restarted.</strong> This page will disconnect. ` +
      `On your phone or laptop: open Wi‑Fi settings → join <strong>${ssid}</strong> → ` +
      `open <strong>http://192.168.4.1/</strong>. If the name did not change, toggle Wi‑Fi off/on and reconnect.`;
  }

  function openMoreSheet() {
    $('#more-sheet')?.classList.add('open');
    document.body.style.overflow = 'hidden';
  }

  function closeMoreSheet() {
    $('#more-sheet')?.classList.remove('open');
    document.body.style.overflow = '';
  }

  function updateBottomTabs(pageId) {
    $$('.bottom-tab').forEach(tab => {
      tab.classList.remove('active');
      const t = tab.dataset.tab;
      if (t === pageId) tab.classList.add('active');
      else if (t === 'more' && MORE_PAGES.includes(pageId)) tab.classList.add('active');
    });
  }

  // Navigation
  function showPage(id) {
    closeMoreSheet();
    $$('.page').forEach(p => p.classList.remove('active'));
    $$('.nav-link').forEach(a => a.classList.remove('active'));
    const page = $('#page-' + id);
    if (page) page.classList.add('active');
    $$(`.nav-link[data-page="${id}"]`).forEach(link => link.classList.add('active'));
    updateBottomTabs(id);
    const titleEl = $('#mobile-page-title');
    if (titleEl) titleEl.textContent = PAGE_TITLES[id] || id;
    const loaders = { dashboard: loadDashboard, wifi: loadWifi, training: loadTraining, monitor: loadMonitor, nfc: loadNfc, peripherals: loadPeripherals, hid: loadHid, ble: loadBle, terminal: loadTerminal, settings: loadSettings };
    if (loaders[id]) loaders[id]();
    if (id !== 'training' && window._trainPoll) {
      clearInterval(window._trainPoll);
      window._trainPoll = null;
    }
    if (id === 'settings') {
      const path = window.location.pathname.replace(/\/$/, '') || '/';
      const tab = SETTINGS_TAB_FROM_PATH[path] || 'general';
      showSettingsTab(tab);
    }
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  function showSettingsTab(tabId) {
    $$('.settings-tab').forEach(t => {
      const isActive = t.dataset.settingsTab === tabId;
      t.classList.toggle('active', isActive);
      t.classList.toggle('btn-primary', isActive);
      t.classList.toggle('btn-ghost', !isActive);
    });
    $$('.settings-panel').forEach(p => { p.hidden = true; });
    const panel = $(`#settings-${tabId}`);
    if (panel) panel.hidden = false;
    if (tabId === 'ap') loadApSettings();
    if (tabId === 'storage') loadStorageSettings();
    if (tabId === 'mode') loadDeviceMode();
    if (tabId === 'profile') loadProfileSettings();
  }

  $$('.settings-tab').forEach(tab => tab.addEventListener('click', () => showSettingsTab(tab.dataset.settingsTab)));

  $$('.nav-link').forEach(a => a.addEventListener('click', e => {
    e.preventDefault();
    showPage(a.dataset.page);
  }));

  $$('.bottom-tab').forEach(tab => {
    tab.addEventListener('click', () => {
      if (tab.dataset.tab === 'more') {
        openMoreSheet();
        $$('.bottom-tab').forEach(t => t.classList.remove('active'));
        tab.classList.add('active');
        return;
      }
      showPage(tab.dataset.page);
    });
  });

  $('#more-close')?.addEventListener('click', closeMoreSheet);
  $('#more-backdrop')?.addEventListener('click', closeMoreSheet);

  // Sync Lab PIN (sidebar, mobile header, settings, setup)
  const pinDesktop = $('#labPin');
  const pinMobile = $('#labPinMobile');

  function syncLabPinFields(value) {
    const v = (value ?? '').trim();
    if (!v) return;
    if (pinDesktop) pinDesktop.value = v;
    if (pinMobile) pinMobile.value = v;
    const setPin = $('#set-lab-pin');
    if (setPin) setPin.value = v;
    const setupPin = $('#setup-lab-pin');
    if (setupPin) setupPin.value = v;
  }

  function applyLabPinFromStatus(s) {
    if (s?.lab_pin) syncLabPinFields(s.lab_pin);
  }

  const SD_EXPORT_MSG = 'Insert an SD card (Full Mode). Export to SD is not available without a card.';

  function formatFlashFree(bytes) {
    const n = Number(bytes) || 0;
    if (n >= 1048576) return `${(n / 1048576).toFixed(1)} MB free on flash`;
    if (n >= 1024) return `${Math.round(n / 1024)} KB free on flash`;
    return `${n} bytes free on flash`;
  }

  function applyStorageFromStatus(s) {
    const ready = !!s?.sd_ready;
    window.__hackcardSdReady = ready;
    const tier = s?.storage || (ready ? 'Full Mode' : 'Flash Mode');
    const stat = $('#stat-storage');
    if (stat) stat.textContent = tier;
    const hint = $('#stat-storage-hint');
    if (hint) {
      hint.textContent = ready
        ? 'SD ready — use Export → SD for debrief files'
        : 'Flash Mode — lab works; exports need SD';
    }
    const noSdTitle = 'Insert SD card (Full Mode)';
    document.querySelectorAll('[data-requires-sd]').forEach(btn => {
      const extraDisabled = btn.id === 'btn-train-export-sd' && btn.dataset.trainCapturesOk !== '1';
      btn.disabled = !ready || extraDisabled;
      btn.title = ready ? (btn.dataset.titleSdOk || 'Copy to SD card') : noSdTitle;
    });
    document.querySelectorAll('[data-sd-only-option]').forEach(el => {
      el.disabled = !ready;
      if (!ready) el.checked = false;
      const label = el.closest('label');
      if (label) label.classList.toggle('disabled-hint', !ready);
    });
    const tierLine = $('#storage-tier-line');
    if (tierLine) {
      tierLine.className = ready ? 'status-line ok' : 'status-line';
      tierLine.textContent = s?.storage_status || (ready ? 'Storage: SD card ready (Full Mode)' : 'Storage: flash only (Flash Mode)');
    }
    const flashFree = $('#storage-flash-free');
    if (flashFree && s?.flash_free_bytes != null) flashFree.textContent = formatFlashFree(s.flash_free_bytes);
    const sdExports = $('#storage-sd-exports');
    if (sdExports) {
      sdExports.textContent = ready
        ? 'SD exports enabled — files go under /wifi/exports/, /nfc/exports/, /payloads/ on the card.'
        : SD_EXPORT_MSG;
    }
  }

  function requireSdCard(msgEl) {
    if (window.__hackcardSdReady) return true;
    if (msgEl) msg(msgEl, SD_EXPORT_MSG, false);
    return false;
  }

  pinDesktop?.addEventListener('input', e => {
    if (pinMobile) pinMobile.value = e.target.value;
    const setPin = $('#set-lab-pin');
    if (setPin) setPin.value = e.target.value;
  });
  pinMobile?.addEventListener('input', e => {
    if (pinDesktop) pinDesktop.value = e.target.value;
    const setPin = $('#set-lab-pin');
    if (setPin) setPin.value = e.target.value;
  });
  $('#set-lab-pin')?.addEventListener('input', e => {
    const v = e.target.value;
    if (pinDesktop) pinDesktop.value = v;
    if (pinMobile) pinMobile.value = v;
  });

  function pageFromPath() {
    const pageParam = new URLSearchParams(window.location.search).get('page');
    if (pageParam && $(`#page-${pageParam}`)) return pageParam;
    const path = window.location.pathname.replace(/\/$/, '') || '/';
    return PATH_TO_PAGE[path] || 'dashboard';
  }

  function showSetupBanner(visible) {
    const el = $('#setup-banner');
    if (el) el.hidden = !visible;
  }

  // Dashboard
  async function updateNetworkStatus() {
    try {
      const s = await HackCardApi.status();
      const apIp = s.ap_ip || '192.168.4.1';
      const apText = `${s.ap_ssid || 'HackCard'} @ http://${apIp}/lab`;
      if ($('#stat-ap-ip')) $('#stat-ap-ip').textContent = apIp;
      if ($('#wifi-ap-endpoint')) $('#wifi-ap-endpoint').textContent = apText;
      if (s.sta_connected && s.sta_ip) {
        if ($('#stat-sta-ip')) $('#stat-sta-ip').textContent = s.sta_ip;
      } else if ($('#stat-sta-ip')) {
        $('#stat-sta-ip').textContent = 'AP mode';
      }
    } catch { /* offline */ }
  }

  async function loadDashboard() {
    try {
      const s = await HackCardApi.status();
      $('#stat-fw').textContent = s.firmware || '—';
      applyStorageFromStatus(s);
      applyLabPinFromStatus(s);
      $('#stat-ap').textContent = s.ap_ssid || '—';
      $('#stat-clients').textContent = s.clients ?? '—';
      const upSec = s.uptime ?? (s.uptime_ms ? Math.floor(s.uptime_ms / 1000) : 0);
      $('#stat-uptime').textContent = upSec ? Math.floor(upSec / 60) + 'm ' + (upSec % 60) + 's' : '—';
      $('#stat-mode').textContent = s.mode_title || s.mode || '—';
      await updateNetworkStatus();
      showSetupBanner(s.first_boot_complete === false);
      setConnDots(true);
    } catch {
      $('#stat-fw').textContent = 'Offline';
      showSetupBanner(false);
      setConnDots(false);
    }
  }

  // Wi-Fi Lab
  const SCAN_KEY = 'hackcard_wifi_scan';

  async function loadWifi() {
    await updateNetworkStatus();
    await refreshScanList();
    if (localStorage.getItem(SCAN_KEY) === '1') {
      pollForWifiScan();
    }
    try {
      const [twin, beacon] = await Promise.all([HackCardApi.wifiTwinStatus(), HackCardApi.wifiBeaconStatus()]);
      const t = twin.status || twin;
      const target = t.target_ssid || twin.target_ssid || '?';
      const restore = t.restore_ap_ssid || twin.restore_ap_ssid || 'HackCard';
      const rem = t.remaining_sec ?? twin.remaining_sec;
      const labIp = t.lab_ip || '192.168.4.1';
      const twinStatusEl = $('#twin-status');
      const twinHintEl = $('#twin-reconnect-hint');
      if (twinStatusEl) {
        twinStatusEl.textContent = t.running || twin.running
          ? `ACTIVE — Wi‑Fi name is now “${target}” (${rem ?? '?'}s left) · HackCard “${restore}” returns when stopped`
          : 'Idle';
      }
      if (twinHintEl) {
        if (t.running || twin.running) {
          twinHintEl.hidden = false;
          twinHintEl.textContent =
            `If you lost the dashboard: join “${target}” on this laptop, then http://${labIp}/ — or wait for auto-restore.`;
        } else {
          twinHintEl.hidden = true;
        }
      }
      $('#beacon-status').textContent = beacon.running ? `ACTIVE — ${beacon.packets_sent ?? beacon.packet_count ?? 0} beacons sent` : 'Idle';
      try {
        const mon = await HackCardApi.wifiMonitorStatus();
        const stressEl = $('#wifi-stress-status');
        if (stressEl) {
          stressEl.textContent = mon.deauth_running
            ? `Demo active · ${mon.deauth_sent ?? 0}/${mon.deauth_limit ?? 5} frames · ${mon.deauth_remaining_sec ?? '?'}s left`
            : 'Idle — stop sniff/karma/capture/beacon/twin before starting';
        }
      } catch { /* monitor optional */ }
    } catch { /* ignore */ }
  }

  function showEmptyScanList(message) {
    const box = $('#wifi-scan-list');
    const cards = $('#wifi-scan-cards');
    const text = message || 'No results — run scan';
    if (box) box.innerHTML = `<tr><td colspan="3">${escapeHtml(text)}</td></tr>`;
    renderMobileCards(cards, [], text);
  }

  function renderScanResults(d) {
    const nets = (d.networks || d.results || []).slice().sort((a, b) => (b.rssi ?? b.RSSI ?? 0) - (a.rssi ?? a.RSSI ?? 0));
    const box = $('#wifi-scan-list');
    const cards = $('#wifi-scan-cards');
    if (!nets.length) {
      if (box) box.innerHTML = '<tr><td colspan="3">No networks found</td></tr>';
      renderMobileCards(cards, [], 'No networks found');
      return;
    }
    if (box) {
      box.innerHTML = nets.map(n => {
        const ssid = escapeHtml(n.ssid || n.SSID || '(hidden)');
        const auth = n.auth ? ` · ${escapeHtml(n.auth)}` : '';
        return `<tr><td>${ssid}${auth ? `<span class="hint">${auth}</span>` : ''}</td><td>${n.rssi ?? n.RSSI ?? '?'}</td><td>${n.channel ?? n.ch ?? '?'}</td></tr>`;
      }).join('');
    }
    if (cards) {
      cards.innerHTML = nets.map(n => {
        const ssid = escapeHtml(n.ssid || n.SSID || '(hidden)');
        const authMeta = n.auth ? `<span>${escapeHtml(n.auth)}</span>` : '';
        return `<div class="list-card"><div class="list-card-title">${ssid}</div><div class="list-card-meta"><span>RSSI ${n.rssi ?? n.RSSI ?? '?'} dBm</span><span>Ch ${n.channel ?? n.ch ?? '?'}</span>${authMeta}</div></div>`;
      }).join('');
    }
  }

  let wifiScanPollRunning = false;

  async function pollForWifiScan() {
    if (wifiScanPollRunning) return;
    wifiScanPollRunning = true;
    const el = $('#wifi-scan-msg');
    localStorage.setItem(SCAN_KEY, '1');
    let sawScanning = false;
    for (let i = 0; i < 60; i++) {
      if (i > 0) await new Promise(r => setTimeout(r, 2000));
      try {
        const d = await HackCardApi.wifiLast();
        if (d.scanning) {
          sawScanning = true;
          msg(el, 'Scanning… results will appear here when finished (~10 s)', true);
          if ((d.networks || []).length) renderScanResults(d);
          continue;
        }
        if (d.ready || sawScanning) {
          if (!d.scanning) {
            localStorage.removeItem(SCAN_KEY);
            renderScanResults(d);
            msg(el, scanStatusMessage(d), d.success !== false);
            wifiScanPollRunning = false;
            return;
          }
        }
      } catch {
        msg(el, 'Rejoin HackCard Wi‑Fi — scan continues on device; this page will update automatically', true);
      }
    }
    localStorage.removeItem(SCAN_KEY);
    msg(el, 'Scan timed out — tap Refresh list or Scan again', false);
    wifiScanPollRunning = false;
  }

  $('#btn-wifi-scan')?.addEventListener('click', async () => {
    const el = $('#wifi-scan-msg');
    localStorage.setItem(SCAN_KEY, '1');
    msg(el, 'Starting scan… stay on Wi‑Fi Lab if possible (~10 s)', true);
    try {
      const d = await HackCardApi.wifiScan();
      if (d.started || d.scanning) {
        await pollForWifiScan();
        return;
      }
      if (d.networks) {
        renderScanResults(d);
        msg(el, `Scan complete — ${d.count ?? d.networks.length} network(s)`, true);
        return;
      }
      localStorage.removeItem(SCAN_KEY);
      msg(el, d.error || 'Could not start scan', false);
    } catch {
      await pollForWifiScan();
    }
  });

  function scanStatusMessage(d) {
    const parts = [];
    if (d.error) parts.push(d.error);
    if (d.scanning) parts.push('Scan still running on device…');
    if (d.storage_flash_ready === false) parts.push('Flash storage not ready — check partition scheme');
    else if (d.storage_saved === false && d.ready) parts.push('Saved in device memory (flash write skipped)');
    if (d.ready && d.success !== false) {
      parts.push(`${d.count ?? d.networks?.length ?? 0} network(s)`);
    }
    return parts.join(' · ') || 'Status unknown';
  }

  async function refreshScanList() {
    const box = $('#wifi-scan-list');
    const cards = $('#wifi-scan-cards');
    const el = $('#wifi-scan-msg');
    try {
      const d = await HackCardApi.wifiLast();
      if (d.raw && !d.ready && !d.networks) {
        msg(el, `API error: ${String(d.raw).slice(0, 120)}`, false);
        return;
      }
      if (d.scanning) {
        msg(el, scanStatusMessage(d), true);
        pollForWifiScan();
        if ((d.networks || []).length) renderScanResults(d);
        return;
      }
      if (!d.ready) {
        const cleared = d.cleared || (d.networks || []).length === 0;
        msg(el, cleared ? 'No saved scan — tap Scan networks' : scanStatusMessage(d), !d.error);
        showEmptyScanList(cleared ? 'List cleared — run scan to find networks' : 'No results — run scan');
        return;
      }
      renderScanResults(d);
      msg(el, scanStatusMessage(d), d.success !== false);
    } catch (e) {
      msg(el, `Could not load scan (${e.message || 'offline'}) — rejoin HackCard Wi‑Fi`, false);
      if (box) box.innerHTML = '<tr><td colspan="3">Could not load scan</td></tr>';
      renderMobileCards(cards, [], 'Could not load scan');
    }
  }
  $('#btn-wifi-scan-refresh')?.addEventListener('click', refreshScanList);

  $('#btn-wifi-scan-clear')?.addEventListener('click', async () => {
    const el = $('#wifi-scan-msg');
    localStorage.removeItem(SCAN_KEY);
    wifiScanPollRunning = false;
    msg(el, 'Clearing saved scan…', true);
    try {
      const d = await HackCardApi.wifiScanClear();
      if (d.ok === false || d.success === false) {
        msg(el, d.error || d.raw || 'Could not clear — wait for scan to finish', false);
        return;
      }
      await refreshScanList();
      msg(el, 'Results cleared. Tap Scan networks for a fresh scan.', true);
    } catch (e) {
      msg(el, e.message || 'Clear failed — check connection', false);
    }
  });
  $('#btn-twin-start')?.addEventListener('click', async () => {
    const el = $('#twin-msg');
    const ssid = $('#twin-ssid').value.trim();
    const withPortal = $('#twin-with-portal').checked ? '1' : '0';
    const template = $('#twin-template').value;
    msg(el, 'Starting evil twin...', true);
    try {
      const mins = $('#twin-minutes')?.value || '5';
      const ch = $('#twin-channel')?.value?.trim();
      const p = { pin: pin(), ssid, minutes: mins, with_portal: withPortal, template, network: ssid };
      if (ch) p.channel = ch;
      const d = await HackCardApi.wifiTwinStart(p);
      if (d.success) {
        const tgt = d.status?.target_ssid || ssid;
        msg(el,
          `Twin active on “${tgt}”. Reconnect this PC to “${tgt}”, then open http://192.168.4.1/` +
          (withPortal === '1' ? ' (training portal on)' : ''),
          true);
      } else {
        msg(el, d.error || 'Failed', false);
      }
      loadWifi();
    } catch (e) { msg(el, e.message, false); }
  });
  $('#btn-twin-stop')?.addEventListener('click', async () => {
    await HackCardApi.wifiTwinStop();
    loadWifi();
    msg($('#twin-msg'), 'Twin stopped', true);
  });

  $('#btn-wifi-stress-start')?.addEventListener('click', async () => {
    const el = $('#wifi-stress-msg');
    const bssid = $('#wifi-stress-bssid')?.value?.trim();
    const channel = $('#wifi-stress-ch')?.value || '6';
    const mode = $('#wifi-stress-mode')?.value || '0';
    if (!bssid) {
      msg(el, 'Enter your lab AP BSSID (from scan list)', false);
      return;
    }
    const d = await HackCardApi.wifiMonitorDeauthStart({ pin: pin(), bssid, channel, count: 5, mode });
    msg(el, d.success ? 'Stress demo started (capped frames)' : (d.error || 'Failed — stop other Wi‑Fi lab tools first'), d.success);
    loadWifi();
  });
  $('#btn-wifi-stress-stop')?.addEventListener('click', async () => {
    await HackCardApi.wifiMonitorDeauthStop();
    msg($('#wifi-stress-msg'), 'Demo stopped', true);
    loadWifi();
  });

  $('#btn-beacon-start')?.addEventListener('click', async () => {
    const el = $('#beacon-msg');
    const ssids = $('#beacon-ssids').value.trim().replace(/\n/g, '%0A');
    msg(el, 'Starting beacon...', true);
    try {
      const d = await HackCardApi.wifiBeaconStart({ pin: pin(), ssids: $('#beacon-ssids').value.trim() });
      msg(el, d.success ? 'Beacon broadcasting' : (d.error || 'Failed'), d.success);
      loadWifi();
    } catch (e) { msg(el, e.message, false); }
  });
  $('#btn-beacon-stop')?.addEventListener('click', async () => { await HackCardApi.wifiBeaconStop(); loadWifi(); msg($('#beacon-msg'), 'Stopped', true); });

  // Training
  async function loadTraining() {
    try {
      const s = await HackCardApi.wifiTrainingStatus();
      const n = s.capture_count ?? 0;
      $('#train-status').textContent = s.active
        ? `ACTIVE — ${s.template} on "${s.network_name}" · ${n} capture(s)`
        : `Idle · ${n} capture(s) on device`;
      $('#train-captures').textContent = await HackCardApi.wifiTrainingCaptures();
      const sdBtn = $('#btn-train-export-sd');
      if (sdBtn) {
        sdBtn.dataset.trainCapturesOk = (s.capture_count ?? 0) >= 1 ? '1' : '0';
      }
      applyStorageFromStatus({ ...s, storage: s.sd_ready ? 'Full Mode' : 'Flash Mode' });
      if (s.active && !window._trainPoll) {
        window._trainPoll = setInterval(loadTraining, 3000);
      } else if (!s.active && window._trainPoll) {
        clearInterval(window._trainPoll);
        window._trainPoll = null;
      }
    } catch { /* ignore */ }
  }

  $('#btn-train-save')?.addEventListener('click', async () => {
    const d = await HackCardApi.wifiTrainingConfig({
      template: $('#train-template').value,
      network: $('#train-network').value,
      headline: $('#train-headline').value,
      subtitle: $('#train-subtitle').value
    });
    msg($('#train-msg'), d.success ? 'Template saved' : 'Save failed', d.success);
  });
  $('#btn-train-start')?.addEventListener('click', async () => {
    const d = await HackCardApi.wifiTrainingStart(pin());
    msg($('#train-msg'), d.success ? 'Portal active at /portal' : (d.error || 'Failed'), d.success);
    loadTraining();
  });
  $('#btn-train-stop')?.addEventListener('click', async () => { await HackCardApi.wifiTrainingStop(); loadTraining(); });
  $('#btn-train-refresh')?.addEventListener('click', () => loadTraining());
  $('#btn-train-clear')?.addEventListener('click', async () => { await HackCardApi.wifiTrainingCapturesClear(); loadTraining(); });
  $('#btn-train-export-sd')?.addEventListener('click', async () => {
    const el = $('#train-msg');
    const hint = $('#train-sd-hint');
    if (!requireSdCard(el)) return;
    msg(el, 'Writing to SD…', true);
    try {
      const d = await HackCardApi.wifiTrainingExportSd();
      if (d.success) {
        msg(el, `Saved on SD: ${d.path}`, true);
        if (hint) {
          hint.hidden = false;
          hint.textContent = `Saved on SD: ${d.path}`;
        }
      } else {
        msg(el, d.error || 'SD export failed', false);
        if (hint) hint.hidden = true;
      }
    } catch (e) {
      msg(el, e.message, false);
    }
  });

  $('#train-template')?.addEventListener('change', () => {
    const t = parseInt($('#train-template').value, 10);
    const presets = {
      0: ['Guest_WiFi', 'Sign in to Wi-Fi', 'Enter your details to access the internet.'],
      1: ['Corp_Network', 'Network Authentication', 'Your organization requires sign-in before internet access.'],
      2: ['Hotel_Guest', 'Welcome — Hotel Wi-Fi', 'Enter your room number and last name to connect.'],
      3: ['HomeRouter', 'Router Firmware Update', 'A critical security update is available. Enter admin credentials to continue.'],
      4: ['SecureBrowse', 'Browser Plugin Required', 'Install the security plugin to continue browsing safely.'],
      5: ['Sign-In', 'Sign in to continue', 'Use your account to access this network.'],
      6: ['ISP_Account', 'Internet service sign-in', 'Verify your account to restore internet access.']
    };
    const p = presets[t] || presets[0];
    $('#train-network').value = p[0];
    $('#train-headline').value = p[1];
    $('#train-subtitle').value = p[2];
  });

  // Monitor
  let monitorTimer;
  async function loadMonitor() {
    try {
      const live = await HackCardApi.wifiMonitorLive();
      const s = live.status || await HackCardApi.wifiMonitorStatus();
      const sniffOn = s.sniffing ?? s.sniff_active;
      const deauthOn = s.deauth_running ?? s.deauth_active;
      $('#mon-sniff-status').textContent = sniffOn ? `Sniffing (${s.sniff_remaining_sec || '?'}s left)` : 'Idle';
      $('#mon-deauth-status').textContent = deauthOn ? `Attack active (${s.deauth_remaining_sec || '?'}s)` : 'Idle';
      const ps = $('#mon-packet-stats');
      if (ps) ps.textContent = `Packets: ${s.packet_total ?? 0} (mgmt ${s.mgmt_packets ?? 0}, data ${s.data_packets ?? 0}) · Ch ${s.hop_channel ?? '—'} · APs ${s.ap_count ?? 0}`;
      const apBox = $('#ap-list');
      const aps = live.aps?.aps || [];
      if (apBox) {
        apBox.innerHTML = aps.length ? aps.map(a => `<tr><td>${escapeHtml(a.ssid)}</td><td>${escapeHtml(a.bssid)}</td><td>${a.rssi ?? '?'}</td><td>${a.channel ?? '?'}</td></tr>`).join('')
          : '<tr><td colspan="4">No beacons yet — start sniff</td></tr>';
      }
      const liveBox = $('#live-feed');
      const feed = live.live || [];
      if (liveBox) {
        liveBox.textContent = feed.length
          ? feed.slice(-8).map(e => `${e.type === 0 ? 'BEACON' : 'PROBE'} ch${e.channel} ${e.rssi}dBm ${e.summary}`).join('\n')
          : '—';
      }
      const ks = await HackCardApi.wifiKarmaStatus();
      $('#karma-status').textContent = ks.running ? `Karma active · ${ks.probes_seen ?? 0} probes · ${ks.responses_sent ?? 0} responses` : 'Idle';
      const cs = await HackCardApi.wifiCaptureStatus();
      $('#capture-status').textContent = cs.running ? `Capturing · ${cs.eapol_frames ?? 0} EAPOL · ${cs.pcap_bytes ?? 0} bytes` : (cs.has_handshake ? 'Handshake hint captured' : 'Idle');
      const sdHint = $('#mon-sd-hint');
      if (sdHint) {
        const sdOk = s.sd_ready ?? cs.sd_ready;
        sdHint.textContent = sdOk
          ? `SD card ready · exports under /wifi/exports/ on the card${s.log_sd_only ? ' · probe log SD-only' : ''}${cs.capture_sd_only ? ' · capture SD-only' : ''}`
          : 'No SD card — probe/capture data uses device flash (or enable SD-only when a card is inserted)';
      }
      const probes = await HackCardApi.wifiMonitorProbes();
      const list = probes.probes || probes || [];
      const box = $('#probe-list');
      const cards = $('#probe-cards');
      if (box) {
        box.innerHTML = list.length ? list.map(p => `<tr><td>${escapeHtml(p.ssid || p.ssid_probed || '?')}</td><td>${escapeHtml(p.mac || p.bssid || '?')}</td><td>${p.rssi ?? '?'}</td></tr>`).join('')
          : '<tr><td colspan="3">No probes yet</td></tr>';
      }
      renderMobileCards(cards, list.map(p => ({
        title: p.ssid || p.ssid_probed || '(any)',
        meta: [`MAC ${p.mac || p.bssid || '?'}`, `RSSI ${p.rssi ?? '?'}`]
      })), 'No probes yet — start sniff');
    } catch { /* ignore */ }
  }

  $('#btn-sniff-start')?.addEventListener('click', async () => {
    const p = { pin: pin(), minutes: 3 };
    if ($('#sniff-save-sd')?.checked) p.save_sd = 1;
    const d = await HackCardApi.wifiMonitorSniffStart(p);
    msg($('#mon-msg'), d.success ? 'Sniff started' : (d.error || 'Failed'), d.success);
    clearInterval(monitorTimer);
    monitorTimer = setInterval(loadMonitor, 3000);
    loadMonitor();
  });
  $('#btn-sniff-stop')?.addEventListener('click', async () => { await HackCardApi.wifiMonitorSniffStop(); loadMonitor(); });
  $('#btn-deauth-start')?.addEventListener('click', async () => {
    const bssid = $('#deauth-bssid').value.trim();
    const channel = $('#deauth-ch').value || '6';
    const mode = $('#deauth-mode')?.value || '0';
    const d = await HackCardApi.wifiMonitorDeauthStart({ pin: pin(), bssid, channel, count: 5, mode });
    msg($('#mon-msg'), d.success ? 'Deauth demo sent' : (d.error || 'Failed'), d.success);
    loadMonitor();
  });
  $('#btn-deauth-stop')?.addEventListener('click', async () => { await HackCardApi.wifiMonitorDeauthStop(); loadMonitor(); });
  $('#btn-probes-clear')?.addEventListener('click', async () => { await HackCardApi.wifiMonitorProbesClear(); loadMonitor(); });
  $('#btn-probes-export-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#mon-msg'))) return;
    const d = await HackCardApi.wifiMonitorProbesExportSd();
    msg($('#mon-msg'), d.success ? `Saved on SD: ${d.path || ''}` : (d.error || 'Export failed'), d.success);
  });

  $('#btn-karma-start')?.addEventListener('click', async () => {
    const d = await HackCardApi.wifiKarmaStart({ pin: pin(), ssids: $('#karma-ssids').value.trim() });
    msg($('#karma-msg'), d.success ? 'Karma active' : (d.error || 'Failed'), d.success);
    loadMonitor();
  });
  $('#btn-karma-stop')?.addEventListener('click', async () => { await HackCardApi.wifiKarmaStop(); loadMonitor(); });

  $('#btn-capture-start')?.addEventListener('click', async () => {
    const ch = $('#capture-ch').value?.trim();
    const p = { pin: pin(), minutes: 5 };
    if (ch) p.channel = ch;
    if ($('#capture-save-sd')?.checked) p.save_sd = 1;
    const d = await HackCardApi.wifiCaptureStart(p);
    msg($('#capture-msg'), d.success ? 'Capture started' : (d.error || 'Failed'), d.success);
    loadMonitor();
  });
  $('#btn-capture-stop')?.addEventListener('click', async () => { await HackCardApi.wifiCaptureStop(); loadMonitor(); });
  $('#btn-capture-verify')?.addEventListener('click', async () => {
    const pwd = $('#capture-verify-pass')?.value || '';
    const d = await HackCardApi.wifiCaptureVerify(pwd);
    msg($('#capture-msg'), d.message || (d.success ? 'Capture ready for offline crack' : 'Verify failed'), d.success);
  });
  $('#btn-capture-export-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#capture-msg'))) return;
    const d = await HackCardApi.wifiCaptureExportSd();
    msg($('#capture-msg'), d.success ? `Saved on SD: ${d.path || ''}` : (d.error || 'Export failed'), d.success);
  });

  // NFC
  function nfcSdDumpParams() {
    return $('#nfc-dumps-save-sd')?.checked ? { dumps_sd: 1 } : {};
  }

  async function loadNfc() {
    const chipEl = $('#nfc-chip-status');
    try {
      const s = await HackCardApi.nfcStatus();
      const ready = !!(s.chip_ready ?? s.ready);
      const info = s.chip_info || s.chip || '';
      const protocol = s.protocol || 'ISO14443A (NFC-A)';
      const sdHint = $('#nfc-sd-hint');
      if (sdHint) {
        sdHint.textContent = s.sd_ready
          ? `SD card ready · exports under /nfc/exports/ on the card${s.history_sd_only ? ' · history SD-only' : ''}${s.dumps_sd_only ? ' · dumps SD-only' : ''}`
          : 'No SD card — NFC files use device flash (enable SD-only when a card is inserted)';
      }
      const histCb = $('#nfc-history-save-sd');
      const dumpCb = $('#nfc-dumps-save-sd');
      if (histCb && s.history_sd_only != null) histCb.checked = !!s.history_sd_only;
      if (dumpCb && s.dumps_sd_only != null) dumpCb.checked = !!s.dumps_sd_only;
      if (!chipEl) return;
      if (ready) {
        chipEl.className = 'status-line ok';
        chipEl.innerHTML =
          `<strong>PN532 ready</strong> — ${escapeHtml(info || 'connected')} · ${escapeHtml(protocol)} · ` +
          `<span class="hint">reader/writer (no tag emulation)</span>`;
      } else {
        chipEl.className = 'status-line err';
        chipEl.innerHTML =
          '<strong>PN532 not detected</strong> — check I2C connection, power, and that NFC is enabled in firmware.';
      }
    } catch {
      if (chipEl) {
        chipEl.className = 'status-line err';
        chipEl.textContent = 'Could not reach NFC status API — rejoin HackCard Wi‑Fi and refresh.';
      }
    }
  }

  function formatNfcActionMessage(d) {
    if (d == null) return 'No response';
    if (typeof d !== 'object') return String(d);
    if (d.summary) return d.summary;
    if (d.text) return d.text;
    if (d.message) return d.message;
    if (d.success === false) return d.error || 'Action failed — hold a tag on the antenna and try again.';
    if (d.uid) {
      const parts = [`UID ${d.uid}`];
      if (d.tag_type) parts.push(d.tag_type);
      if (d.family) parts.push(d.family);
      if (d.pages != null) parts.push(`${d.pages} page(s)`);
      if (d.memory_hint) parts.push(d.memory_hint);
      if (d.ndef_preview) {
        const prev = String(d.ndef_preview);
        parts.push(`NDEF: ${prev.length > 72 ? `${prev.slice(0, 72)}…` : prev}`);
      }
      if (d.saved_path) parts.push(`Saved: ${d.saved_path}`);
      return parts.join(' · ');
    }
    return 'Done.';
  }
  const nfcOut = (el, d) => msg(el, formatNfcActionMessage(d), d.success !== false);

  function renderBleStatus(s) {
    const el = $('#ble-status');
    if (!el) return;
    const ready = !!(s.ble_ready ?? s.ready);
    if (!ready) {
      el.className = 'status-line err';
      el.innerHTML = '<strong>BLE not ready</strong> — stack unavailable or disabled in this firmware build.';
      return;
    }
    const modes = [];
    if (s.advertising) {
      const name = (s.advert_name || '').trim();
      modes.push(name ? `Advertising as “${escapeHtml(name)}”` : 'Advertising (custom name)');
    }
    if (s.contact_card) {
      const cname = (s.contact_name || '').trim();
      modes.push(cname ? `Contact card GATT — “${escapeHtml(cname)}”` : 'Contact card GATT active');
    }
    let headline;
    if (modes.length) headline = `<strong>BLE ready</strong> — ${modes.join(' · ')}`;
    else headline = '<strong>BLE ready</strong> — idle (not advertising, contact card off)';
    const hints = [];
    if (s.has_last_scan) hints.push('last scan saved on device');
    if (hints.length) headline += ` · <span class="hint">${escapeHtml(hints.join(' · '))}</span>`;
    el.className = 'status-line ok';
    el.innerHTML = headline;
  }

  $('#btn-nfc-read')?.addEventListener('click', async () => {
    await HackCardApi.nfcStoragePrefs({
      history_sd: $('#nfc-history-save-sd')?.checked ? 1 : 0,
      dumps_sd: $('#nfc-dumps-save-sd')?.checked ? 1 : 0
    });
    nfcOut($('#nfc-msg'), await HackCardApi.nfcRead(nfcSdDumpParams()));
  });
  $('#btn-nfc-info')?.addEventListener('click', async () => { nfcOut($('#nfc-msg'), await HackCardApi.nfcInfo()); });
  $('#btn-nfc-dump')?.addEventListener('click', async () => {
    await HackCardApi.nfcStoragePrefs({
      history_sd: $('#nfc-history-save-sd')?.checked ? 1 : 0,
      dumps_sd: $('#nfc-dumps-save-sd')?.checked ? 1 : 0
    });
    nfcOut($('#nfc-msg'), await HackCardApi.nfcDump(nfcSdDumpParams()));
  });
  $('#btn-nfc-write-url')?.addEventListener('click', async () => {
    nfcOut($('#nfc-msg'), await HackCardApi.nfcWrite({ type: 'url', url: $('#nfc-url').value }));
  });
  $('#btn-nfc-write-text')?.addEventListener('click', async () => {
    nfcOut($('#nfc-msg'), await HackCardApi.nfcWrite({ type: 'text', text: $('#nfc-text').value }));
  });
  $('#btn-nfc-write-vcard')?.addEventListener('click', async () => {
    nfcOut($('#nfc-msg'), await HackCardApi.nfcWrite({ type: 'vcard' }));
  });
  $('#btn-nfc-history')?.addEventListener('click', async () => {
    $('#nfc-history').textContent = await HackCardApi.nfcHistory();
  });
  $('#nfc-history-save-sd')?.addEventListener('change', async () => {
    await HackCardApi.nfcStoragePrefs({
      history_sd: $('#nfc-history-save-sd')?.checked ? 1 : 0,
      dumps_sd: $('#nfc-dumps-save-sd')?.checked ? 1 : 0
    });
    loadNfc();
  });
  $('#nfc-dumps-save-sd')?.addEventListener('change', async () => {
    await HackCardApi.nfcStoragePrefs({
      history_sd: $('#nfc-history-save-sd')?.checked ? 1 : 0,
      dumps_sd: $('#nfc-dumps-save-sd')?.checked ? 1 : 0
    });
    loadNfc();
  });
  $('#btn-nfc-history-export-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#nfc-msg'))) return;
    const d = await HackCardApi.nfcHistoryExportSd();
    msg($('#nfc-msg'), d.success ? `History on SD: ${d.path || ''}` : (d.error || 'Export failed'), d.success);
  });
  $('#btn-nfc-dump-export-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#nfc-msg'))) return;
    const d = await HackCardApi.nfcDumpExportSd();
    msg($('#nfc-msg'), d.success ? `Dump on SD: ${d.path || ''}` : (d.error || 'Export failed'), d.success);
  });
  $('#btn-nfc-classic-export-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#nfc-msg'))) return;
    const d = await HackCardApi.nfcClassicExportSd();
    msg($('#nfc-msg'), d.success ? `Classic on SD: ${d.path || ''}` : (d.error || 'Export failed'), d.success);
  });

  const nfcLab = async (fn) => {
    try { nfcOut($('#nfc-lab-msg'), await fn()); } catch (e) { msg($('#nfc-lab-msg'), e.message, false); }
  };
  $('#btn-nfc-erase')?.addEventListener('click', () => nfcLab(() => HackCardApi.nfcErase(pin())));
  $('#btn-nfc-clone')?.addEventListener('click', () => nfcLab(() => HackCardApi.nfcClone({ pin: pin() })));
  $('#btn-nfc-kill')?.addEventListener('click', () => nfcLab(() => HackCardApi.nfcKill(pin())));
  $('#btn-nfc-last-dump')?.addEventListener('click', async () => {
    $('#nfc-lab-msg').textContent = await HackCardApi.nfcLastDump();
  });
  $('#btn-nfc-classic-dump')?.addEventListener('click', () => nfcLab(async () => {
    await HackCardApi.nfcStoragePrefs({
      history_sd: $('#nfc-history-save-sd')?.checked ? 1 : 0,
      dumps_sd: $('#nfc-dumps-save-sd')?.checked ? 1 : 0
    });
    return HackCardApi.nfcClassicDump({ ...nfcSdDumpParams() });
  }));
  $('#btn-nfc-classic-clone')?.addEventListener('click', () => nfcLab(() => HackCardApi.nfcClassicClone(pin())));
  $('#btn-nfc-keys-scan')?.addEventListener('click', () => nfcLab(() => HackCardApi.nfcKeysScan(pin())));

  // Peripherals (RGB + Buzzer)
  async function loadPeripherals() {
    try {
      const s = await HackCardApi.status();
      if (s.brightness != null && $('#rgb-brightness')) {
        $('#rgb-brightness').value = s.brightness;
        const bLbl = $('#rgb-brightness-val');
        if (bLbl) bLbl.textContent = `${s.brightness}%`;
      }
      if (s.buzzer_volume != null && $('#buzzer-volume')) {
        $('#buzzer-volume').value = s.buzzer_volume;
        const vLbl = $('#buzzer-volume-val');
        if (vLbl) vLbl.textContent = `${s.buzzer_volume}%`;
      }
      if (s.melody_playing != null) {
        setMelodyPlayingUi(!!s.melody_playing);
        if (s.melody_playing && !melodyPollTimer) {
          startMelodyPoll('Song finished');
        }
      }
    } catch { /* ignore */ }
  }

  async function saveRingBrightness(value) {
    const v = Math.min(100, Math.max(10, parseInt(value, 10) || 80));
    const lbl = $('#rgb-brightness-val');
    if (lbl) lbl.textContent = `${v}%`;
    try {
      const d = await HackCardApi.apps({ action: 'ring_brightness', value: v });
      msg($('#rgb-msg'), typeof d === 'string' ? d : (d.raw || `Brightness ${v}% saved`), true);
    } catch (e) {
      msg($('#rgb-msg'), e.message, false);
    }
  }

  async function saveBuzzerVolume(value) {
    const v = Math.min(100, Math.max(10, parseInt(value, 10) || 80));
    const lbl = $('#buzzer-volume-val');
    if (lbl) lbl.textContent = `${v}%`;
    try {
      const d = await HackCardApi.apps({ action: 'buzzer_volume', value: v });
      const st = await HackCardApi.status();
      if (!st.melody_playing) {
        await HackCardApi.apps({ action: 'buzzer', value: 'success' });
      }
      msg($('#buzzer-msg'), typeof d === 'string' ? d : (d.raw || `Volume ${v}% saved`), true);
    } catch (e) {
      msg($('#buzzer-msg'), e.message, false);
    }
  }

  let melodyPollTimer = null;

  function setMelodyPlayingUi(playing) {
    const stopBtn = $('#btn-buzzer-stop-song');
    if (stopBtn) stopBtn.disabled = !playing;
  }

  function stopMelodyPoll() {
    if (melodyPollTimer) {
      clearInterval(melodyPollTimer);
      melodyPollTimer = null;
    }
  }

  function startMelodyPoll(finishedLabel) {
    stopMelodyPoll();
    setMelodyPlayingUi(true);
    melodyPollTimer = setInterval(async () => {
      try {
        const st = await HackCardApi.status();
        if (!st.melody_playing) {
          stopMelodyPoll();
          setMelodyPlayingUi(false);
          msg($('#buzzer-msg'), finishedLabel || 'Song finished', true);
        }
      } catch {
        stopMelodyPoll();
        setMelodyPlayingUi(false);
      }
    }, 700);
  }

  async function playBuzzerAction(body, okText) {
    const el = $('#buzzer-msg');
    try {
      const d = await HackCardApi.apps(body);
      const raw = typeof d === 'string' ? d : d?.raw || '';
      const ok = d?.ok !== false && !raw.includes('503') && !raw.includes('not ready');
      msg(el, ok ? (okText || 'Playing…') : (raw || 'Buzzer failed — wait for boot chime / peripherals'), ok);
    } catch (e) {
      msg(el, e.message, false);
    }
  }
  const rgbColor = () => {
    const hex = $('#rgb-color')?.value || '#0078ff';
    const n = parseInt(hex.slice(1), 16);
    return { r: (n >> 16) & 255, g: (n >> 8) & 255, b: n & 255 };
  };

  $$('[data-pattern]').forEach(btn => btn.addEventListener('click', async () => {
    const c = rgbColor();
    await HackCardApi.apps({ action: 'ring_demo', value: btn.dataset.pattern, r: c.r, g: c.g, b: c.b });
    msg($('#rgb-msg'), `Pattern ${btn.dataset.pattern} running`, true);
  }));
  $('#btn-rgb-stop')?.addEventListener('click', async () => { await HackCardApi.apps({ action: 'ring_stop' }); msg($('#rgb-msg'), 'Stopped', true); });
  $$('[data-ring]').forEach(btn => btn.addEventListener('click', async () => {
    await HackCardApi.apps({ action: 'ring', value: btn.dataset.ring });
  }));
  $$('[data-buzzer-demo]').forEach(btn => btn.addEventListener('click', async () => {
    const label = btn.textContent?.trim() || `Song ${btn.dataset.buzzerDemo}`;
    const vol = $('#buzzer-volume')?.value || '—';
    msg($('#buzzer-msg'), `Starting ${label} at volume ${vol}%…`, true);
    try {
      const d = await HackCardApi.apps({ action: 'buzzer_demo', value: btn.dataset.buzzerDemo });
      if (d.success) {
        startMelodyPoll(`${label} finished`);
        msg($('#buzzer-msg'), `Playing ${label} — adjust volume slider anytime`, true);
      } else {
        msg($('#buzzer-msg'), d.error || d.raw || 'Could not start song', false);
      }
    } catch (e) {
      msg($('#buzzer-msg'), e.message, false);
    }
  }));
  $('#btn-buzzer-stop-song')?.addEventListener('click', async () => {
    try {
      await HackCardApi.apps({ action: 'buzzer_melody_stop' });
      stopMelodyPoll();
      setMelodyPlayingUi(false);
      msg($('#buzzer-msg'), 'Song stopped', true);
    } catch (e) {
      msg($('#buzzer-msg'), e.message, false);
    }
  });
  $$('[data-buzzer]').forEach(btn => btn.addEventListener('click', () => {
    playBuzzerAction({ action: 'buzzer', value: btn.dataset.buzzer }, `Playing ${btn.dataset.buzzer.replace('_', ' ')}`);
  }));
  $('#rgb-brightness')?.addEventListener('input', e => {
    const lbl = $('#rgb-brightness-val');
    if (lbl) lbl.textContent = `${e.target.value}%`;
  });
  $('#rgb-brightness')?.addEventListener('change', e => saveRingBrightness(e.target.value));
  $('#buzzer-volume')?.addEventListener('input', e => {
    const lbl = $('#buzzer-volume-val');
    if (lbl) lbl.textContent = `${e.target.value}%`;
  });
  $('#buzzer-volume')?.addEventListener('change', e => saveBuzzerVolume(e.target.value));

  // HID
  let selectedHidPayload = null;
  const hidPayloadMsg = (text, ok) => msg($('#hid-payload-msg'), text, ok);
  const hidCustomMsg = (text, ok) => msg($('#hid-msg'), text, ok);

  function markHidQuickPressed(btn) {
    if (!btn?.dataset?.hid) return;
    if (btn.classList.contains('btn-primary') || btn.classList.contains('btn-blue')) return;
    const group = btn.closest('.hid-quick-actions');
    if (!group) return;
    group.querySelectorAll('[data-hid].btn-pressed').forEach(b => {
      b.classList.remove('btn-pressed', 'btn-red');
      b.classList.add('btn-ghost');
    });
    btn.classList.remove('btn-ghost');
    btn.classList.add('btn-red', 'btn-pressed');
  }

  function updateHidSdUi(sdReady) {
    const ready = !!sdReady;
    const saveSd = $('#btn-hid-save-sd');
    const hint = $('#hid-sd-hint');
    if (saveSd) {
      saveSd.disabled = !ready;
      saveSd.title = ready ? 'Writes to /payloads/ on SD card' : 'Insert SD card and reboot';
    }
    if (hint) hint.hidden = ready;
  }

  const HID_SCRIPT_TEMPLATE = `REM HackCard DuckyScript
OS windows
DELAY 1500
STRING Hello from HackCard!
ENTER
`;

  function normalizePayloadName(name) {
    const n = String(name || '').trim();
    if (!n) return '';
    if (/\.(duck|txt|ds)$/i.test(n)) return n;
    return `${n}.duck`;
  }

  async function loadHidPayloads() {
    const listEl = $('#hid-payload-list');
    const storageEl = $('#hid-payload-storage');
    try {
      const d = await HackCardApi.hidPayloads();
      updateHidSdUi(d.sd_ready);
      if (storageEl) {
        const sdNote = d.sd_ready ? 'SD + flash' : 'flash only (no SD)';
        const freeKb = d.flash_free_bytes != null ? Math.round(Number(d.flash_free_bytes) / 1024) : null;
        const freeNote = freeKb != null ? ` · ~${freeKb} KB free on flash` : '';
        storageEl.textContent = `Storage: ${sdNote}${freeNote} · ${d.count || 0} script(s) in /payloads/`;
      }
      const rows = d.payloads || [];
      if (!listEl) return;
      if (!rows.length) {
        listEl.innerHTML = '<tr><td colspan="4">No payloads yet — click New or Upload</td></tr>';
        return;
      }
      listEl.innerHTML = rows.map(p => `
        <tr class="${selectedHidPayload === p.name ? 'selected' : ''}">
          <td>${escapeHtml(p.name)}</td><td>${p.size ?? '?'} B</td><td>${escapeHtml(p.source || '?')}</td>
          <td><button type="button" class="btn btn-sm btn-ghost hid-payload-pick" data-name="${escapeHtml(p.name)}">Open</button></td>
        </tr>`).join('');
      listEl.querySelectorAll('.hid-payload-pick').forEach(btn => {
        btn.addEventListener('click', () => selectHidPayload(btn.dataset.name));
      });
      if (!selectedHidPayload && rows[0]) selectHidPayload(rows[0].name);
    } catch {
      if (listEl) listEl.innerHTML = '<tr><td colspan="4">Could not load payloads</td></tr>';
    }
  }

  async function selectHidPayload(name) {
    selectedHidPayload = name;
    const editor = $('#hid-payload-editor');
    const nameEl = $('#hid-payload-name');
    if (nameEl) nameEl.value = name;
    if (!editor) return;
    editor.value = 'Loading…';
    try {
      const d = await HackCardApi.hidPayloadRead(name);
      editor.value = d.content ? d.content.replace(/\\n/g, '\n') : '';
      await loadHidPayloads();
    } catch {
      editor.value = '';
      hidPayloadMsg('Could not read payload', false);
    }
  }

  function currentHidScriptName() {
    return normalizePayloadName($('#hid-payload-name')?.value || selectedHidPayload || '');
  }

  async function saveHidEditorToDevice() {
    const name = currentHidScriptName();
    const content = $('#hid-payload-editor')?.value || '';
    if (!name) return { success: false, error: 'Enter a filename ending in .duck or .txt' };
    if (!content.trim()) return { success: false, error: 'Editor is empty' };
    const d = await HackCardApi.hidPayloadSave({ pin: pin(), name, content });
    if (d.success) {
      selectedHidPayload = name;
    }
    return d;
  }

  async function loadHid() {
    try {
      const s = await HackCardApi.hidStatus();
      $('#hid-status').textContent = s.ready
        ? `Ready — host ${s.host_connected ? 'connected' : 'disconnected'} · OS ${s.target_os || 'windows'} · ${s.payload_count ?? 0} payload(s)`
        : 'USB HID not ready';
      const armEl = $('#hid-arm-status');
      if (armEl) {
        armEl.textContent = s.button_armed
          ? `BOOT armed: ${s.button_payload} (${Math.ceil((s.button_expires_ms || 0) / 1000)}s left)`
          : 'BOOT arm: idle — use Arm to run payload on long-press';
      }
      await loadHidPayloads();
    } catch { /* ignore */ }
  }

  $('#btn-hid-new-script')?.addEventListener('click', () => {
    selectedHidPayload = null;
    const nameEl = $('#hid-payload-name');
    const editor = $('#hid-payload-editor');
    if (nameEl) nameEl.value = 'my_script.duck';
    if (editor) editor.value = HID_SCRIPT_TEMPLATE;
    loadHidPayloads();
    hidPayloadMsg('New script — edit and Save to device', true);
  });

  $('#btn-hid-upload-trigger')?.addEventListener('click', () => $('#hid-payload-upload')?.click());

  $('#hid-payload-upload')?.addEventListener('change', async e => {
    const file = e.target.files?.[0];
    if (!file) return;
    try {
      const text = await file.text();
      selectedHidPayload = null;
      const nameEl = $('#hid-payload-name');
      const editor = $('#hid-payload-editor');
      if (nameEl) nameEl.value = normalizePayloadName(file.name);
      if (editor) editor.value = text;
      hidPayloadMsg(`Loaded ${file.name} — click Save to write to device`, true);
    } catch {
      hidPayloadMsg('Could not read uploaded file', false);
    }
    e.target.value = '';
  });

  $('#btn-hid-save-script')?.addEventListener('click', async () => {
    const d = await saveHidEditorToDevice();
    hidPayloadMsg(d.error || (d.success ? `Saved ${currentHidScriptName()}` : 'Save failed'), d.success);
    if (d.success) {
      await loadHidPayloads();
      await selectHidPayload(currentHidScriptName());
    }
  });

  $('#btn-hid-save-sd')?.addEventListener('click', async () => {
    if (!requireSdCard($('#hid-payload-msg'))) return;
    const name = currentHidScriptName();
    const content = $('#hid-payload-editor')?.value || '';
    if (!name) return hidPayloadMsg('Enter a filename ending in .duck or .txt', false);
    if (!content.trim()) return hidPayloadMsg('Editor is empty', false);
    const d = await HackCardApi.hidPayloadSaveSd({ pin: pin(), name, content });
    updateHidSdUi(d.sd_ready);
    if (!d.sd_ready && d.success === false) {
      hidPayloadMsg(d.error || 'No SD card — insert SD and reboot, or use Save to device', false);
      return;
    }
    hidPayloadMsg(d.error || (d.success ? `Saved to SD: /payloads/${name}` : 'Save to SD failed'), d.success);
    if (d.success) {
      selectedHidPayload = name;
      await loadHidPayloads();
    }
  });

  $('#btn-hid-delete-script')?.addEventListener('click', async () => {
    const name = currentHidScriptName();
    if (!name) return hidPayloadMsg('Enter or select a payload to delete', false);
    if (!confirm(`Delete ${name} from device?`)) return;
    const d = await HackCardApi.hidPayloadDelete({ pin: pin(), name });
    hidPayloadMsg(d.error || (d.success ? `Deleted ${name}` : 'Delete failed'), d.success);
    if (d.success) {
      selectedHidPayload = null;
      $('#hid-payload-name').value = '';
      $('#hid-payload-editor').value = '';
      loadHidPayloads();
    }
  });

  $('#btn-hid-run-script')?.addEventListener('click', async () => {
    const content = $('#hid-payload-editor')?.value || '';
    if (!content.trim()) return hidPayloadMsg('Editor is empty — click New or paste a script', false);
    const osMap = { windows: 'windows', mac: 'mac', macos: 'mac', linux: 'linux' };
    const osVal = $('#hid-os')?.value || 'windows';
    hidPayloadMsg('Running on USB host… focus Notepad on the PC', true);
    const d = await HackCardApi.hidRunScript({ pin: pin(), os: osMap[osVal] || osVal, content });
    hidPayloadMsg(d.error || (d.success ? 'Script sent to USB host' : 'Run failed'), d.success);
    if (d.success) {
      const saved = await saveHidEditorToDevice();
      if (saved.success) {
        await loadHidPayloads();
      }
    }
    loadHid();
  });

  $('#btn-hid-arm-script')?.addEventListener('click', async () => {
    const saved = await saveHidEditorToDevice();
    if (!saved.success) return hidPayloadMsg(saved.error || 'Save before arm failed', false);
    const name = currentHidScriptName();
    const osMap = { windows: 'windows', mac: 'mac', macos: 'mac', linux: 'linux' };
    const osVal = $('#hid-os')?.value || 'windows';
    const d = await HackCardApi.hidArm({ pin: pin(), name, os: osMap[osVal] || osVal });
    hidPayloadMsg(d.error || (d.success ? 'BOOT long-press armed (5 min)' : 'Arm failed'), d.success);
    loadHid();
  });

  $('#btn-hid-disarm')?.addEventListener('click', async () => {
    await HackCardApi.hidDisarm();
    hidPayloadMsg('BOOT disarmed', true);
    loadHid();
  });

  $$('[data-hid]').forEach(btn => btn.addEventListener('click', async () => {
    const action = btn.dataset.hid;
    const p = { action, pin: pin() };
    if (action === 'type_text') p.text = $('#hid-text').value;
    if (action === 'open_website') p.url = $('#hid-url').value;
    if (action === 'unlock_screen') {
      p.password = $('#hid-unlock-password')?.value || '';
      if (!p.password.trim()) {
        hidCustomMsg('Enter the host lock-screen password first', false);
        return;
      }
    }
    if (action === 'mouse_click') p.button = btn.dataset.button || 'left';
    if (action === 'mouse_move') { p.dx = btn.dataset.dx || '0'; p.dy = btn.dataset.dy || '0'; }
    markHidQuickPressed(btn);
    const d = await HackCardApi.hidRun(p);
    const ok = d.ok !== false && (d.success !== false) && !String(d.error || d.raw || '').toLowerCase().includes('invalid pin');
    hidCustomMsg(d.error || d.message || d.raw || 'Sent to USB host', ok);
    if (!ok && btn.classList.contains('btn-pressed')) {
      btn.classList.remove('btn-pressed', 'btn-red');
      btn.classList.add('btn-ghost');
    }
  }));
  $('#hid-os')?.addEventListener('change', async e => {
    await HackCardApi.hidOs({ 0: 0, windows: 0, mac: 1, macos: 1, linux: 2 }[e.target.value] ?? e.target.value);
  });

  // BLE
  async function loadBle() {
    const el = $('#ble-status');
    try {
      const s = await HackCardApi.bleStatus();
      renderBleStatus(s);
      loadBleScanLast();
    } catch {
      if (el) {
        el.className = 'status-line err';
        el.textContent = 'Could not reach BLE status API — rejoin HackCard Wi‑Fi and refresh.';
      }
    }
  }
  function renderBleScan(devices) {
    const list = devices || [];
    const box = $('#ble-scan-list');
    const cards = $('#ble-scan-cards');
    if (box) {
      box.innerHTML = list.length ? list.map(d => `<tr><td>${escapeHtml(d.name || d.local_name || '?')}</td><td>${escapeHtml(d.address || d.mac || '?')}</td><td>${d.rssi ?? '?'}</td></tr>`).join('')
        : '<tr><td colspan="3">No devices</td></tr>';
    }
    renderMobileCards(cards, list.map(d => ({
      title: d.name || d.local_name || '(unknown)',
      meta: [`${d.address || d.mac || '?'}`, `RSSI ${d.rssi ?? '?'}`]
    })), 'No devices');
  }

  async function loadBleScanLast() {
    try {
      const d = await HackCardApi.bleScanLast();
      renderBleScan(d.devices || d.results || []);
    } catch { /* ignore */ }
  }

  $('#btn-ble-scan')?.addEventListener('click', async () => {
    msg($('#ble-msg'), 'Scanning 5s...', true);
    const d = await HackCardApi.bleScan();
    renderBleScan(d.devices || []);
    msg($('#ble-msg'), `Found ${(d.devices || []).length} devices`, true);
  });
  $('#btn-ble-refresh')?.addEventListener('click', async () => {
    const d = await HackCardApi.bleContactRefresh();
    msg($('#ble-msg'), d.success !== false ? 'Profile refreshed on BLE' : (d.error || 'Failed'), d.success !== false);
  });
  $('#btn-ble-adv-start')?.addEventListener('click', async () => {
    const d = await HackCardApi.bleAdvertiseStart({ pin: pin(), name: $('#ble-name').value || 'HackCard' });
    msg($('#ble-msg'), d.success ? 'Advertising started' : (d.error || 'Failed'), d.success);
    loadBle();
  });
  $('#btn-ble-adv-stop')?.addEventListener('click', async () => {
    await HackCardApi.bleAdvertiseStop();
    msg($('#ble-msg'), 'Advertising stopped', true);
    loadBle();
  });
  $('#btn-ble-contact')?.addEventListener('click', async () => {
    const d = await HackCardApi.bleContactStart();
    msg($('#ble-msg'), d.success !== false ? 'Contact card active' : (d.error || 'Failed'), d.success !== false);
    loadBle();
  });
  $('#btn-ble-contact-stop')?.addEventListener('click', async () => {
    await HackCardApi.bleContactStop();
    msg($('#ble-msg'), 'Contact card stopped', true);
    loadBle();
  });

  // Terminal
  const termHistory = [];
  let termHistIdx = -1;
  async function loadTerminal() {
    const out = $('#term-out');
    if (out && !out.dataset.loaded) {
      out.textContent = await HackCardApi.cliBanner();
      out.dataset.loaded = '1';
    }
  }
  async function runTermCommand(cmd) {
    const out = $('#term-out');
    if (!cmd.trim() || !out) return;
    termHistory.unshift(cmd);
    termHistIdx = -1;
    out.textContent += `\nhackcard> ${cmd}\n`;
    try {
      const r = await HackCardApi.cli(cmd);
      out.textContent += (r.raw || r.output || JSON.stringify(r)) + '\n';
    } catch (err) { out.textContent += `Error: ${err.message}\n`; }
    out.scrollTop = out.scrollHeight;
  }
  $('#term-form')?.addEventListener('submit', async e => {
    e.preventDefault();
    const input = $('#term-input');
    const cmd = input.value.trim();
    await runTermCommand(cmd);
    input.value = '';
  });
  $('#term-input')?.addEventListener('keydown', e => {
    const input = $('#term-input');
    if (e.key === 'ArrowUp' && termHistory.length) {
      termHistIdx = Math.min(termHistIdx + 1, termHistory.length - 1);
      input.value = termHistory[termHistIdx] || '';
      e.preventDefault();
    }
    if (e.key === 'ArrowDown' && termHistory.length) {
      termHistIdx = Math.max(termHistIdx - 1, -1);
      input.value = termHistIdx < 0 ? '' : termHistory[termHistIdx];
      e.preventDefault();
    }
  });

  // Settings & system
  async function loadStorageSettings() {
    try {
      const s = await HackCardApi.status();
      applyStorageFromStatus(s);
    } catch {
      const tierLine = $('#storage-tier-line');
      if (tierLine) tierLine.textContent = 'Could not load storage status — rejoin HackCard Wi‑Fi';
    }
  }

  async function loadSettings() {
    try {
      const s = await HackCardApi.status();
      applyStorageFromStatus(s);
      if (s.ap_ssid && $('#setup-ap-ssid')) {
        $('#setup-ap-ssid').value = s.ap_ssid;
      }
      applyLabPinFromStatus(s);
      if (s.owner_name || s.owner) $('#setup-name').value = s.owner_name || s.owner;
      if (s.title) $('#setup-title').value = s.title;
      if (s.email) { $('#setup-email').value = s.email; $('#prof-email').value = s.email; }
      if (s.captive_portal != null && $('#setup-captive')) $('#setup-captive').checked = !!s.captive_portal;
      if (s.owner_name || s.owner) $('#prof-name').value = s.owner_name || s.owner;
      if (s.title) $('#prof-title').value = s.title;
      if (s.mode_title || s.mode) $('#mode-current').textContent = s.mode_title || s.mode;
      if (s.brightness != null) {
        $('#set-brightness').value = s.brightness;
        const bLbl = $('#set-brightness-val');
        if (bLbl) bLbl.textContent = `${s.brightness}%`;
      }
      if (s.buzzer_volume != null && $('#set-buzzer-volume')) {
        $('#set-buzzer-volume').value = s.buzzer_volume;
        const vLbl = $('#set-buzzer-volume-val');
        if (vLbl) vLbl.textContent = `${s.buzzer_volume}%`;
      }
      if (s.boot_ring_pattern != null && $('#set-boot-pattern')) {
        $('#set-boot-pattern').value = String(s.boot_ring_pattern);
      }
      const rgbToHex = (r, g, b) => {
        const hex = (n) => Number(n).toString(16).padStart(2, '0');
        return `#${hex(r)}${hex(g)}${hex(b)}`;
      };
      if ($('#set-boot-color') && s.boot_ring_r != null) {
        $('#set-boot-color').value = rgbToHex(s.boot_ring_r, s.boot_ring_g, s.boot_ring_b);
      }
      if (s.wifi_led_pattern != null && $('#set-wifi-led-pattern')) {
        $('#set-wifi-led-pattern').value = String(s.wifi_led_pattern);
      }
      if ($('#set-wifi-led-color') && s.wifi_led_r != null) {
        $('#set-wifi-led-color').value = rgbToHex(s.wifi_led_r, s.wifi_led_g, s.wifi_led_b);
      }
      if (s.wifi_ring_pattern != null && $('#set-wifi-ring-pattern')) {
        $('#set-wifi-ring-pattern').value = String(s.wifi_ring_pattern);
      }
      if ($('#set-wifi-ring-color') && s.wifi_ring_r != null) {
        $('#set-wifi-ring-color').value = rgbToHex(s.wifi_ring_r, s.wifi_ring_g, s.wifi_ring_b);
      }
      if (s.buzzer_enabled != null) $('#set-buzzer').checked = s.buzzer_enabled !== false;
      if (s.boot_animation != null) $('#set-bootanim').checked = s.boot_animation !== false;
      if (s.activity_logging != null) $('#set-logging').checked = s.activity_logging !== false;
      if (s.captive_portal != null) $('#set-portal').checked = !!s.captive_portal;
      if (s.tune_success != null) $('#set-tune-success').value = s.tune_success;
      if (s.tune_error != null) $('#set-tune-error').value = s.tune_error;
      if (s.tune_boot != null) $('#set-tune-boot').value = s.tune_boot;
      if (s.tune_nfc != null) $('#set-tune-nfc').value = s.tune_nfc;
      if (s.first_boot_complete === false) showSettingsTab('setup');
    } catch { /* ignore */ }
  }

  async function loadApSettings() {
    try {
      const ap = await HackCardApi.wifiApGet();
      if (ap.ssid) $('#ap-ssid').value = ap.ssid;
      if (ap.password != null) $('#ap-pass').value = ap.password;
      if (ap.channel) $('#ap-channel').value = ap.channel;
      $('#ap-status-line').textContent = `SSID: ${ap.ssid || '—'} · Ch ${ap.channel ?? '—'} · IP ${ap.ip || '—'}`;
    } catch { $('#ap-status-line').textContent = 'Could not load AP status'; }
  }

  async function loadProfileSettings() {
    try {
      const p = await fetch('/api/profile/public').then(r => r.json());
      if (p.name) $('#prof-name').value = p.name;
      if (p.title) $('#prof-title').value = p.title;
      if (p.company) $('#prof-company').value = p.company;
      if (p.bio) $('#prof-bio').value = p.bio;
      if (p.email) $('#prof-email').value = p.email;
      if (p.phone) $('#prof-phone').value = p.phone;
      if (p.website) $('#prof-website').value = p.website;
      if (p.linkedin) $('#prof-linkedin').value = p.linkedin;
      if (p.github && $('#prof-github')) $('#prof-github').value = p.github;
    } catch { /* ignore */ }
  }

  async function loadDeviceMode() {
    const grid = $('#mode-grid');
    try {
      const [data, st] = await Promise.all([HackCardApi.modes(), HackCardApi.status().catch(() => ({}))]);
      const list = data.modes || [];
      const stored = data.current;
      let current = stored;
      if (st.lab_access && stored === 'business') {
        current = 'lab';
      }
      const active = list.find(m => m.id === current);
      if ($('#mode-current')) $('#mode-current').textContent = active?.title || current || '—';
      if (stored === 'business' && st.lab_access) {
        msg($('#mode-msg'), 'You are using the lab console — switch to Lab mode below to save that preference.', true);
      }
      if (!grid) return;
      grid.innerHTML = list.map(m => {
        const isActive = m.id === current;
        return `<div class="persona-card${isActive ? ' active' : ''}">
          <div class="persona-head"><strong>${escapeHtml(m.title)}</strong>${isActive ? ' <span class="badge badge-green">Active</span>' : ''}</div>
          <p class="hint">${escapeHtml(m.description)}</p>
          <button type="button" class="btn btn-sm ${isActive ? 'btn-ghost' : 'btn-primary'}" data-mode-select="${escapeHtml(m.id)}" ${isActive ? 'disabled' : ''}>${isActive ? 'Current' : 'Switch to this mode'}</button>
        </div>`;
      }).join('');
      grid.querySelectorAll('[data-mode-select]').forEach(btn => {
        btn.addEventListener('click', async () => {
          const id = btn.dataset.modeSelect;
          const d = await HackCardApi.modeSet(id);
          msg($('#mode-msg'), typeof d === 'string' ? d : (d.raw || 'Mode updated'), true);
          loadDeviceMode();
          loadDashboard();
          if (id === 'business') {
            setTimeout(() => { window.location.href = '/portal'; }, 800);
          }
        });
      });
    } catch {
      if (grid) grid.innerHTML = '<p class="hint">Could not load device modes</p>';
    }
  }

  $('#btn-setup-save')?.addEventListener('click', async () => {
    const setupPayload = {
      name: $('#setup-name').value,
      title: $('#setup-title').value,
      email: $('#setup-email').value,
      ap_ssid: $('#setup-ap-ssid').value,
      ap_password: $('#setup-ap-pass').value
    };
    const setupPin = ($('#setup-lab-pin')?.value || '').trim();
    if (setupPin) setupPayload.lab_pin = setupPin;
    if ($('#setup-captive')) setupPayload.captive = $('#setup-captive').checked ? '1' : '0';
    const newSsid = $('#setup-ap-ssid').value.trim();
    msg($('#setup-msg'), 'Saving setup… AP may restart', true);
    const d = await HackCardApi.setup(setupPayload);
    const raw = typeof d === 'string' ? d : (d?.raw || '');
    const ok = d?.ok !== false && (typeof d === 'string' || !raw || raw.indexOf('Could not') < 0);
    msg($('#setup-msg'), raw || (ok ? 'Setup saved' : 'Setup failed'), ok);
    if (ok) {
      showSetupBanner(false);
      showSettingsReconnectNotice(wifiReconnectNoticeHtml(newSsid || raw.match(/SSID[^\n]*/)?.[0] || ''));
      loadSettings().catch(() => {});
      loadDashboard().catch(() => {});
    }
  });

  $('#btn-profile-save')?.addEventListener('click', async () => {
    const d = await HackCardApi.profile({
      name: $('#prof-name').value,
      title: $('#prof-title').value,
      company: $('#prof-company').value,
      bio: $('#prof-bio')?.value || '',
      email: $('#prof-email').value,
      phone: $('#prof-phone').value,
      website: $('#prof-website').value,
      linkedin: $('#prof-linkedin').value,
      github: $('#prof-github')?.value || '',
      pin: pin()
    });
    const ok = d?.ok !== false && (typeof d === 'string' || (d?.raw && d.raw.indexOf('Profile saved') >= 0));
    msg(
      $('#profile-msg'),
      typeof d === 'string' ? d : (d?.raw || (ok ? 'Public profile saved — preview at /portal' : 'Could not save profile')),
      ok
    );
    if (ok) loadProfileSettings();
  });

  $('#btn-ap-save')?.addEventListener('click', async () => {
    const d = await HackCardApi.wifiApSet({
      pin: pin(),
      ssid: $('#ap-ssid').value,
      password: $('#ap-pass').value,
      channel: $('#ap-channel').value
    });
    const ok = d.success !== false;
    msg($('#ap-msg'), ok ? 'AP updated — Wi‑Fi will restart' : (d.error || 'Failed'), ok);
    if (ok) {
      showSettingsReconnectNotice(wifiReconnectNoticeHtml($('#ap-ssid').value.trim()));
    }
    loadApSettings();
  });
  $('#btn-ap-refresh')?.addEventListener('click', loadApSettings);


  function parseHexColor(hex) {
    const h = String(hex || '#0078ff').replace('#', '');
    if (h.length !== 6) return { r: 0, g: 120, b: 255 };
    return { r: parseInt(h.slice(0, 2), 16), g: parseInt(h.slice(2, 4), 16), b: parseInt(h.slice(4, 6), 16) };
  }

  $('#set-brightness')?.addEventListener('input', e => {
    const lbl = $('#set-brightness-val');
    if (lbl) lbl.textContent = `${e.target.value}%`;
  });
  $('#set-buzzer-volume')?.addEventListener('input', e => {
    const lbl = $('#set-buzzer-volume-val');
    if (lbl) lbl.textContent = `${e.target.value}%`;
  });

  async function saveSettingsPayload(payload, msgEl, extraOkHint) {
    msg(msgEl, 'Saving…', true);
    try {
      const d = await HackCardApi.settings(payload);
      const raw = typeof d === 'string' ? d : (d?.raw || '');
      const ok = d?.ok !== false && (typeof d === 'string' || !raw || raw.toLowerCase().indexOf('could not') < 0);
      let text = raw || (ok ? 'Saved' : 'Save failed');
      if (ok && extraOkHint) text = `${text.replace(/[!.\s]+$/, '')}. ${extraOkHint}`;
      if (ok && payload?.lab_pin) syncLabPinFields(payload.lab_pin);
      msg(msgEl, text, ok);
      return ok;
    } catch (e) {
      msg(msgEl, e.message || 'Save failed — check Wi‑Fi connection', false);
      return false;
    }
  }

  async function previewWifiLook(msgEl) {
    const led = parseHexColor($('#set-wifi-led-color')?.value);
    const ring = parseHexColor($('#set-wifi-ring-color')?.value);
    msg(msgEl, 'Previewing…', true);
    try {
      await HackCardApi.apps({
        action: 'wifi_preview',
        r: led.r,
        g: led.g,
        b: led.b,
        led_pattern: $('#set-wifi-led-pattern')?.value || '0',
        ring_pattern: $('#set-wifi-ring-pattern')?.value || '0',
        ring_r: ring.r,
        ring_g: ring.g,
        ring_b: ring.b
      });
      msg(msgEl, 'Preview active — use Save in each section to keep', true);
    } catch (e) {
      msg(msgEl, e.message || 'Preview failed', false);
    }
  }

  $('#btn-set-preview-wifi-led')?.addEventListener('click', () => previewWifiLook($('#settings-msg-wifi-led')));
  $('#btn-set-preview-wifi-ring')?.addEventListener('click', () => previewWifiLook($('#settings-msg-wifi-ring')));

  $('#btn-set-preview-boot')?.addEventListener('click', async () => {
    const c = parseHexColor($('#set-boot-color')?.value);
    const pattern = $('#set-boot-pattern')?.value || '0';
    const msgEl = $('#settings-msg-boot');
    msg(msgEl, 'Playing preview…', true);
    try {
      if (pattern !== '0') {
        await HackCardApi.apps({ action: 'ring_demo', value: pattern, r: c.r, g: c.g, b: c.b });
      } else {
        await HackCardApi.apps({ action: 'ring', value: 'boot' });
      }
      if ($('#set-buzzer')?.checked) {
        await HackCardApi.apps({ action: 'buzzer', value: 'boot' });
      }
      msg(msgEl, 'Preview sent — Save boot & startup to keep for next power-on', true);
    } catch (e) {
      msg(msgEl, e.message || 'Preview failed', false);
    }
  });

  $('#btn-save-lab-pin')?.addEventListener('click', () => {
    const labPin = ($('#set-lab-pin')?.value || '').trim();
    if (labPin.length < 4 || labPin.length > 15) {
      msg($('#settings-msg-lab'), 'Lab PIN must be 4–15 characters', false);
      return;
    }
    syncLabPinFields(labPin);
    saveSettingsPayload({ lab_pin: labPin }, $('#settings-msg-lab'));
  });

  $('#btn-save-display')?.addEventListener('click', () => {
    saveSettingsPayload({ brightness: $('#set-brightness').value }, $('#settings-msg-display'), 'Brightness applied now.');
  });

  $('#btn-save-wifi-led')?.addEventListener('click', () => {
    const c = parseHexColor($('#set-wifi-led-color')?.value);
    saveSettingsPayload({
      wifi_led_pattern: $('#set-wifi-led-pattern')?.value || '0',
      wifi_led_r: String(c.r),
      wifi_led_g: String(c.g),
      wifi_led_b: String(c.b)
    }, $('#settings-msg-wifi-led'), 'Wi‑Fi LED theme applied now.');
  });

  $('#btn-save-wifi-ring')?.addEventListener('click', () => {
    const c = parseHexColor($('#set-wifi-ring-color')?.value);
    saveSettingsPayload({
      wifi_ring_pattern: $('#set-wifi-ring-pattern')?.value || '0',
      wifi_ring_r: String(c.r),
      wifi_ring_g: String(c.g),
      wifi_ring_b: String(c.b)
    }, $('#settings-msg-wifi-ring'), 'Ring scan style applied now.');
  });

  $('#btn-save-boot')?.addEventListener('click', () => {
    const c = parseHexColor($('#set-boot-color')?.value);
    saveSettingsPayload({
      bootanim: $('#set-bootanim').checked ? '1' : '0',
      tune_boot: $('#set-tune-boot').value,
      boot_ring_pattern: $('#set-boot-pattern')?.value || '0',
      boot_ring_r: String(c.r),
      boot_ring_g: String(c.g),
      boot_ring_b: String(c.b)
    }, $('#settings-msg-boot'), 'Reboot once to test boot LED/chime.');
  });

  $('#btn-save-buzzer')?.addEventListener('click', () => {
    saveSettingsPayload({
      buzzer: $('#set-buzzer').checked ? '1' : '0',
      buzzer_volume: $('#set-buzzer-volume')?.value || '80',
      tune_success: $('#set-tune-success').value,
      tune_error: $('#set-tune-error').value,
      tune_nfc: $('#set-tune-nfc').value
    }, $('#settings-msg-buzzer'), 'Buzzer settings applied now.');
  });

  $('#btn-save-system')?.addEventListener('click', () => {
    saveSettingsPayload({
      logging: $('#set-logging').checked ? '1' : '0',
      captive: $('#set-portal').checked ? '1' : '0'
    }, $('#settings-msg-system'));
  });

  $('#setup-banner-go')?.addEventListener('click', () => { showPage('settings'); showSettingsTab('setup'); });

  // Fixed serial console (every page)
  let serialCursor = 0;
  let serialAutoScroll = true;
  let serialLineCount = 0;
  const serialOut = $('#serial-dock-out');
  const serialConsole = $('#serial-console');
  const serialMeta = $('#serial-meta');
  const serialLiveDot = $('#serial-live-dot');

  function scrollSerialToEnd() {
    if (serialOut && serialAutoScroll) serialOut.scrollTop = serialOut.scrollHeight;
  }

  function updateSerialMeta() {
    if (serialMeta) serialMeta.textContent = `Live · ${serialLineCount} lines · web mirror (works in USB OTG)`;
  }

  $('#serial-collapse')?.addEventListener('click', () => {
    const collapsed = serialConsole?.classList.toggle('collapsed');
    const btn = $('#serial-collapse');
    if (btn) {
      btn.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
      btn.textContent = collapsed ? '▸' : '▾';
    }
  });

  $('#serial-clear')?.addEventListener('click', () => {
    if (serialOut) {
      serialOut.textContent = '[SYSTEM] Log cleared\n';
      serialLineCount = 1;
      updateSerialMeta();
      scrollSerialToEnd();
    }
  });

  $('#serial-autoscroll')?.addEventListener('click', e => {
    serialAutoScroll = !serialAutoScroll;
    e.currentTarget.setAttribute('aria-pressed', serialAutoScroll ? 'true' : 'false');
    if (serialAutoScroll) scrollSerialToEnd();
  });

  serialOut?.addEventListener('scroll', () => {
    if (!serialOut) return;
    const nearBottom = serialOut.scrollHeight - serialOut.scrollTop - serialOut.clientHeight < 48;
    if (!nearBottom && serialAutoScroll) {
      serialAutoScroll = false;
      $('#serial-autoscroll')?.setAttribute('aria-pressed', 'false');
    }
  });

  async function pollSerialLog() {
    try {
      const chunk = await HackCardApi.serialLog(serialCursor);
      serialCursor = chunk.cursor;
      serialLiveDot?.classList.remove('off');
      if (chunk.text && serialOut) {
        if (serialOut.textContent.startsWith('[SYSTEM] Waiting')) serialOut.textContent = '';
        serialOut.textContent += chunk.text;
        serialLineCount += (chunk.text.match(/\n/g) || []).length;
        if (serialOut.textContent.length > 120000) {
          serialOut.textContent = serialOut.textContent.slice(-80000);
          serialLineCount = (serialOut.textContent.match(/\n/g) || []).length;
        }
        updateSerialMeta();
        scrollSerialToEnd();
      }
    } catch {
      serialLiveDot?.classList.add('off');
      if (serialMeta) serialMeta.textContent = 'Offline · reconnect to HackCard AP';
    }
  }

  // Init — honor firmware URL paths (/wifi, /nfc, /setup, …)
  sessionStorage.removeItem('hackcard_resume_attempted');
  showPage(pageFromPath());
  (async () => {
    try {
      const s = await HackCardApi.status();
      if (s.business_mode && !s.lab_access) {
        localStorage.removeItem('hackcard_lab_st');
      }
    } catch { /* offline */ }
  })();
  pollSerialLog();
  setInterval(pollSerialLog, 1500);
  setInterval(() => {
    const active = $('.page.active')?.id?.replace('page-', '');
    if (active === 'dashboard') loadDashboard();
    if (active === 'monitor') loadMonitor();
  }, 8000);

  async function pollConnectionStatus() {
    try {
      const s = await HackCardApi.status();
      applyLabPinFromStatus(s);
      applyStorageFromStatus(s);
      setConnDots(true);
    } catch {
      setConnDots(false);
    }
  }
  pollConnectionStatus();
  setInterval(pollConnectionStatus, 5000);
})();
