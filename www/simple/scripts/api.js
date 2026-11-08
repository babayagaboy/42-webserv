const endpoints = {
    POST: '/create_file.py',
    PUT: '/put_file.py',
    PATCH: '/patch_file.py',
    DELETE: '/delete_file.py',
};

export async function request(url, options = {}) {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 15000);
    try {
        return await fetch(url, { cache: 'no-store', ...options, signal: controller.signal });
    } catch (error) {
        if (error.name === 'AbortError') {
            throw new Error('The server took too long to respond. Please try again.');
        }
        throw new Error('Could not reach the server. Check that Webserv is running and try again.');
    } finally {
        clearTimeout(timeout);
    }
}

export async function readResult(response) {
    const text = await response.text();
    if (!response.ok) {
        if (response.status === 409) {
            throw new Error('A file with this name already exists. Choose another name or open it in the editor.');
        }
        const message = response.headers.get('content-type')?.includes('text/html')
            ? `The server returned ${response.status} ${response.statusText}.`
            : text.trim();
        throw new Error(message || `Request failed (${response.status}). Please try again.`);
    }
    return text.trim();
}

export async function loadFiles() {
    const response = await request('/files');
    if (!response.ok) {
        throw new Error(`Could not load files (${response.status}). Use Refresh to try again.`);
    }
    const filenames = await response.json();
    if (!Array.isArray(filenames) || filenames.some(name => typeof name !== 'string')) {
        throw new Error('The server returned an unexpected file list. Please try again.');
    }
    return filenames;
}

export function validateFilename(filename) {
    if (!filename || filename === '.' || filename === '..' || /[/\\\x00-\x1f]/.test(filename)) {
        throw new Error('Enter a filename without folders or control characters.');
    }
}

export function fileUrl(filename) {
    return `/files/${encodeURIComponent(filename)}`;
}

export async function saveFile(method, filename, content = '') {
    validateFilename(filename);
    const response = await request(endpoints[method], {
        method,
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ filename, content }),
    });
    const message = await readResult(response);
    return `${response.status} · ${message}`;
}

export function responseSummary(response, method, path) {
    const headers = [...response.headers].map(([name, value]) => `${name}: ${value}`);
    return `${method} ${path}\nHTTP/1.1 ${response.status} ${response.statusText}\n\n${headers.join('\n')}`;
}

export function fileType(filename) {
    const extension = filename.includes('.') ? filename.split('.').pop().toLowerCase() : '';
    if (['jpg', 'jpeg', 'png', 'gif', 'webp', 'svg', 'avif', 'ico'].includes(extension)) {
        return { group: 'image', label: `${extension.toUpperCase()} image` };
    }
    if (['txt', 'md', 'json', 'html', 'css', 'js', 'csv', 'xml', 'py', 'cpp', 'h', 'conf', 'log', 'yaml', 'yml'].includes(extension)) {
        return { group: 'text', label: `${extension.toUpperCase()} file` };
    }
    return { group: 'other', label: extension ? `${extension.toUpperCase()} file` : 'File' };
}
