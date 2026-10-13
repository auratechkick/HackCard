/** HackCard — public resume-style profile page */
(() => {
  const $ = (sel) => document.querySelector(sel);

  (function resumeLabAfterReconnect() {
    const scanPending = localStorage.getItem('hackcard_wifi_scan') === '1';
    const connectPending = localStorage.getItem('hackcard_wifi_connect') === '1';
    if (!scanPending && !connectPending) return;
    if (sessionStorage.getItem('hackcard_resume_attempted') === '1') return;

    sessionStorage.setItem('hackcard_resume_attempted', '1');

    (async () => {
      try {
        const st = localStorage.getItem('hackcard_lab_st') || sessionStorage.getItem('hackcard_lab_st') || '';
        const statusUrl = st ? `/api/status?st=${encodeURIComponent(st)}` : '/api/status';
        const res = await fetch(statusUrl, { credentials: 'include' });
        const s = await res.json();
        if (s.lab_access) {
          const params = new URLSearchParams();
          if (st) params.set('st', st);
          params.set('page', 'wifi');
          window.location.replace(`/lab?${params.toString()}`);
          return;
        }
      } catch {
        /* AP may still be reconnecting — stay on profile, no redirect loop */
      }
      localStorage.removeItem('hackcard_wifi_scan');
      localStorage.removeItem('hackcard_wifi_connect');
    })();
  })();

  function addInfoItem(list, label, value, href) {
    if (!value) return;
    const li = document.createElement('li');
    if (href) {
      li.innerHTML = `<span class="info-label">${label}</span><a href="${href}">${value}</a>`;
    } else {
      li.innerHTML = `<span class="info-label">${label}</span><span class="info-value">${value}</span>`;
    }
    list.appendChild(li);
  }

  function renderQr(url) {
    const box = $('#prof-qr');
    if (!box) return;
    box.innerHTML = '';
    if (typeof QRCode !== 'undefined') {
      try {
        new QRCode(box, {
          text: url,
          width: 128,
          height: 128,
          colorDark: '#0a0a0a',
          colorLight: '#ffffff',
          correctLevel: QRCode.CorrectLevel.M
        });
        return;
      } catch { /* fall through */ }
    }
    box.innerHTML = '<p class="empty-note" style="padding:16px">QR unavailable</p>';
  }

  async function loadProfile() {
    const res = await fetch('/api/profile/public');
    const p = await res.json();
    const name = p.name || 'HackCard User';
    const initial = name.trim()[0]?.toUpperCase() || '?';

    $('#prof-avatar').textContent = initial;
    $('#prof-name').textContent = name;
    document.title = name + ' — Professional Profile';

    const titleEl = $('#prof-title');
    if (p.title) {
      titleEl.textContent = p.title;
    } else {
      titleEl.textContent = 'Professional profile';
      titleEl.style.color = '#94a3b8';
    }

    const companyEl = $('#prof-company');
    if (p.company) {
      companyEl.textContent = p.company;
    } else {
      companyEl.hidden = true;
    }

    const aboutSection = $('#section-about');
    const bioEl = $('#prof-bio');
    if (p.bio) {
      bioEl.textContent = p.bio;
      aboutSection.hidden = false;
    }

    renderQr(`${location.origin}/profile.vcf`);

    const contactList = $('#contact-list');
    contactList.innerHTML = '';
    addInfoItem(contactList, 'Email', p.email, p.email ? `mailto:${p.email}` : '');
    addInfoItem(contactList, 'Phone', p.phone, p.phone ? `tel:${p.phone}` : '');
    if (!contactList.children.length) {
      contactList.innerHTML = '<li class="empty-note">Contact details not set yet</li>';
    }

    const linksList = $('#links-list');
    linksList.innerHTML = '';
    addInfoItem(linksList, 'Website', p.website, p.website);
    addInfoItem(linksList, 'LinkedIn', p.linkedin ? 'View profile' : '', p.linkedin);
    addInfoItem(linksList, 'GitHub', p.github ? 'View profile' : '', p.github);
    if (!linksList.children.length) {
      linksList.innerHTML = '<li class="empty-note">Links not set yet</li>';
    }
  }

  const dialog = $('#login-dialog');
  const loginMsg = $('#login-msg');

  $('#btn-operator')?.addEventListener('click', () => {
    loginMsg.textContent = '';
    loginMsg.className = 'msg';
    dialog?.showModal();
    $('#login-pin')?.focus();
  });

  $('#login-cancel')?.addEventListener('click', () => dialog?.close());

  $('#login-form')?.addEventListener('submit', async (e) => {
    e.preventDefault();
    const pin = ($('#login-pin')?.value || '').trim();
    if (!pin) return;
    loginMsg.textContent = 'Checking…';
    loginMsg.className = 'msg';
    try {
      const res = await fetch('/api/lab/unlock', {
        method: 'POST',
        credentials: 'include',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `pin=${encodeURIComponent(pin)}`
      });
      const data = await res.json().catch(() => ({}));
      if (res.ok && data.ok) {
        loginMsg.textContent = 'Opening dashboard…';
        loginMsg.className = 'msg ok';
        sessionStorage.removeItem('hackcard_resume_attempted');
        const token = data.token || '';
        if (token) {
          sessionStorage.setItem('hackcard_lab_st', token);
          localStorage.setItem('hackcard_lab_st', token);
          window.location.replace(`/lab?st=${encodeURIComponent(token)}`);
        } else {
          window.location.replace('/lab');
        }
        return;
      }
      loginMsg.textContent = data.error || 'Wrong password';
      loginMsg.className = 'msg err';
    } catch {
      loginMsg.textContent = 'Could not connect';
      loginMsg.className = 'msg err';
    }
  });

  loadProfile().catch(() => {
    $('#prof-name').textContent = 'Profile unavailable';
  });
})();
