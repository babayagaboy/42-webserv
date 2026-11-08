import { initDelete } from './delete.js';
import { initCookieTest } from './cookies.js';
import { request, responseSummary } from './api.js';
import { initFiles } from './files.js';
import { initCreate, initEditor } from './forms.js';

async function checkConnection() {
    const output = document.querySelector('#server-response');
    const button = document.querySelector('#check-server');
    if (button) {
        button.disabled = true;
    }
    try {
        const response = await request('/', { method: 'HEAD' });
        if (!response.ok) {
            throw new Error(`Server returned ${response.status}. Try checking again.`);
        }
        if (output) {
            const contentType = response.headers.get('content-type');
            const length = response.headers.get('content-length');
            output.textContent = `HTTP/1.1 ${response.status} ${response.statusText}`;
            if (contentType) output.textContent += `\ncontent-type: ${contentType}`;
            if (length) output.textContent += `\ncontent-length: ${length}`;
        }
    } catch (error) {
        if (output) {
            output.textContent = error.message;
        }
    } finally {
        if (button) {
            button.disabled = false;
        }
    }
}

function initInspector() {
    const form = document.querySelector('#inspect-form');
    if (!form) {
        return;
    }
    form.addEventListener('submit', async event => {
        event.preventDefault();
        const method = form.elements.method.value;
        const filename = form.elements.filename.value.trim();
        const path = filename ? `/files/${encodeURIComponent(filename)}` : '/';
        const button = form.querySelector('button');
        const output = document.querySelector('#inspect-response');
        const status = document.querySelector('#inspect-status');
        button.disabled = true;
        status.textContent = 'Sending…';
        output.textContent = `${method} ${path}\nWaiting for the server…`;
        try {
            const response = await request(path, { method });
            output.textContent = responseSummary(response, method, path);
            status.textContent = `${response.status} ${response.statusText}`;
        } catch (error) {
            status.textContent = 'Request failed';
            output.textContent = error.message;
        } finally {
            button.disabled = false;
        }
    });
}

document.querySelector('#check-server')?.addEventListener('click', checkConnection);
if (document.querySelector('#server-response')) {
    checkConnection();
}
const page = document.body.dataset.page;
if (page === 'home' || page === 'files') {
    initFiles(page === 'home');
}
if (page === 'files') {
    initInspector();
    initCookieTest();
}
if (page === 'create') {
    initCreate();
}
if (page === 'edit') {
    initEditor();
}
if (page === 'delete') {

    initDelete();

}
