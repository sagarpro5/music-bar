// ==UserScript==
// @name         YouTube Subscription Cleaner
// @namespace    https://github.com/sagarpro5/music-bar
// @version      7.0
// @description  Keep Tech, Health and Business subscriptions and unsubscribe from the rest.
// @match        https://www.youtube.com/feed/channels*
// @grant        none
// ==/UserScript==

(function () {
    'use strict';

    const DRY_RUN = false;
    const CLICK_DELAY = 700;
    const LOAD_DELAY = 2200;

    const KEEP_WORDS = [
        'tech','technology','computer','computing','programming','coding','coder',
        'developer','development','software','hardware','linux','android','iphone',
        'ipad','apple','microsoft','windows','google','chrome','firefox','ai',
        'artificial intelligence','machine learning','deep learning','cyber',
        'cybersecurity','ethical hacking','hacking','robotics','electronics',
        'gadget','gadgets','pc','server','servers','network','networking','cloud',
        'python','javascript','typescript','java','c++','c programming',
        'web development','app development','devops','database','electroboom',
        'health','healthy','fitness','medical','medicine','doctor','doctors',
        'nutrition','diet','workout','gym','exercise','mental health','wellness',
        'physio','physiotherapy','healthcare','dentist','dental','yoga',
        'weight loss','bodybuilding','health tips',
        'business','finance','financial','investing','investment','investor',
        'stock','stocks','trading','entrepreneur','entrepreneurship','startup',
        'startups','marketing','sales','economics','money','wealth','accounting',
        'real estate','crypto','cryptocurrency','banking','mutual fund',
        'mutual funds','personal finance','financial education'
    ];

    const deleted = [];
    let running = false;

    const sleep = ms => new Promise(r => setTimeout(r, ms));

    function channelName(button) {
        const aria = button.getAttribute('aria-label') || '';
        const m = aria.match(/\bfor\s+(.+)$/i);
        if (m) return m[1].trim();

        const row = button.closest('ytd-channel-renderer, ytd-grid-channel-renderer, #content');
        if (row) {
            const link = row.querySelector('a[href*="/@"], a[href*="/channel/"]');
            if (link?.textContent.trim()) return link.textContent.trim();
        }
        return 'Unknown channel';
    }

    function keep(name) {
        const n = name.toLowerCase();
        return KEEP_WORDS.some(word => n.includes(word));
    }

    function subscribedButtons() {
        return [...document.querySelectorAll('button')].filter(b => {
            const text = `${b.innerText || ''} ${b.getAttribute('aria-label') || ''}`.toLowerCase();
            return text.includes('subscribed') && !text.includes('subscribe to');
        });
    }

    function menuUnsubscribe() {
        return [...document.querySelectorAll('[role="menuitem"], tp-yt-paper-item, ytd-menu-service-item-renderer')]
            .find(el => (el.innerText || '').trim().toLowerCase() === 'unsubscribe');
    }

    // Exact confirmation control from the current YouTube UI.
    function confirmationButton() {
        return document.querySelector('button[aria-label="Unsubscribe"]');
    }

    async function waitForConfirmation(timeout = 5000) {
        const end = Date.now() + timeout;
        while (Date.now() < end) {
            const button = confirmationButton();
            if (button && !button.disabled) return button;
            await sleep(100);
        }
        return null;
    }

    async function unsubscribe(button, name) {
        button.scrollIntoView({ block: 'center', behavior: 'instant' });
        button.click();
        await sleep(CLICK_DELAY);

        const menu = menuUnsubscribe();
        if (!menu) {
            console.warn('[Cleaner] Menu Unsubscribe not found:', name);
            return false;
        }

        if (DRY_RUN) {
            console.log('[Cleaner] WOULD DELETE:', name);
            document.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true }));
            return false;
        }

        menu.click();

        const confirm = await waitForConfirmation();
        if (!confirm) {
            console.warn('[Cleaner] Blue Unsubscribe confirmation not found:', name);
            return false;
        }

        confirm.click();
        deleted.push(name);
        console.log('[Cleaner] 🗑️ DELETED:', name);
        await sleep(CLICK_DELAY);
        return true;
    }

    async function loadMore() {
        window.scrollTo(0, document.body.scrollHeight);
        await sleep(800);

        const button = [...document.querySelectorAll('button')]
            .find(b => (b.innerText || '').trim().toLowerCase() === 'load more');

        if (!button) return false;
        button.click();
        await sleep(LOAD_DELAY);
        return true;
    }

    function printDeleted() {
        console.log('==============================');
        console.log('🗑️ DELETED CHANNELS');
        console.log('==============================');
        if (!deleted.length) console.log('No channels deleted.');
        else deleted.forEach((name, i) => console.log(`${i + 1}. ${name}`));
        console.log(`Total deleted: ${deleted.length}`);
        console.log('==============================');
    }

    async function start() {
        if (running) return console.log('[Cleaner] Already running.');
        running = true;
        console.log('[Cleaner] Started. Live mode:', !DRY_RUN);

        while (running) {
            const buttons = subscribedButtons();
            let removed = false;

            for (const button of buttons) {
                if (!running) break;
                if (!document.contains(button)) continue;

                const name = channelName(button);
                if (keep(name)) {
                    console.log('[Cleaner] KEEP:', name);
                    continue;
                }

                console.log('[Cleaner] REMOVE:', name);
                if (await unsubscribe(button, name)) {
                    removed = true;
                    break; // YouTube rebuilds the DOM; reacquire everything.
                }
            }

            if (!running) break;
            if (DRY_RUN) break;
            if (removed) continue;

            if (!(await loadMore())) break;
        }

        running = false;
        printDeleted();
        console.log('[Cleaner] Finished.');
    }

    function stop() {
        running = false;
        console.log('[Cleaner] Stopped.');
        printDeleted();
    }

    window.subscriptionCleaner = { start, stop, deleted };
    console.log('[Cleaner] Ready — run subscriptionCleaner.start()');
})();
