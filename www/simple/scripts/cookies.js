import { request } from './api.js';

async function cookieRequest(action) {
    const response = await request('/cookie_test.py', {
        method: action === 'check' ? 'GET' : 'POST',
        credentials: 'same-origin',
        body: action === 'check' ? undefined : new URLSearchParams({ action }),
    });

    if (!response.headers.get('content-type')?.includes('application/json')) {
        throw new Error('Cookie test unavailable. Check the server configuration and try again.');
    }

    const result = await response.json();
    if (!response.ok) {
        throw new Error(result.error || `Cookie test failed (${response.status}). Try again.`);
    }

    if (result.received !== null && typeof result.received !== 'string') {
        throw new Error('The server returned an invalid cookie result. Try again.');
    }

    return result;
}

export function initCookieTest() {
    const controls = document.querySelector('#cookie-controls');
    const buttons = controls.querySelectorAll('button');
    const output = document.querySelector('#cookie-response');
    const status = document.querySelector('#cookie-status');

    async function runTest(action) {
        buttons.forEach(button => { button.disabled = true; });
        output.setAttribute('aria-busy', 'true');
        status.textContent = 'Checking…';
        output.textContent = 'Waiting for the server…';
        const lines = [];

        try {
            if (action !== 'check') {
                const result = await cookieRequest(action);
                lines.push(`Server sent:\nSet-Cookie: ${result.set_cookie}`);
                output.textContent = `${lines[0]}\n\nChecking the next request…`;
            }

            const result = await cookieRequest('check');
            const received = result.received === null
                ? 'No test cookie received.'
                : `Received cookie: webserv_test=${result.received}`;
            lines.push(`Server received on check:\n${received}`);

            if (action === 'set' && result.received !== 'hello') {
                status.textContent = 'Not verified';
                lines.push('The test cookie was not returned. Check browser cookie settings and try again.');
            } else if (action === 'clear' && result.received !== null) {
                status.textContent = 'Not cleared';
                lines.push('The server still receives the test cookie. Try Clear cookie again.');
            } else {
                status.textContent = result.received === null ? 'No test cookie' : 'Cookie received';
            }
            output.textContent = lines.join('\n\n');
        } catch (error) {
            status.textContent = 'Request failed';
            lines.push(error.message);
            output.textContent = lines.join('\n\n');
        } finally {
            output.removeAttribute('aria-busy');
            buttons.forEach(button => { button.disabled = false; });
        }
    }

    controls.addEventListener('click', event => {
        const button = event.target.closest('button[data-cookie-action]');
        if (button) {
            runTest(button.dataset.cookieAction);
        }
    });

    runTest('check');
}
