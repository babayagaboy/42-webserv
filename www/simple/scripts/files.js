import { loadFiles, fileUrl, fileType, saveFile } from './api.js';

function makeLink(text, href, filename) {
    const link = document.createElement('a');
    link.textContent = text;
    link.href = href;
    if (filename) link.setAttribute('aria-label', `${text} ${filename}`);
    return link;
}

export function initFiles(home) {
    let files = [];
    let page = 0;
    let pendingDelete = '';
    const pageSize = home ? 5 : 8;
    const rows = document.querySelector('#file-rows');
    const count = document.querySelector('#file-count');
    const message = document.querySelector('#file-message');
    const search = document.querySelector('#file-search');
    const refresh = document.querySelector('#refresh-files');
    const previous = document.querySelector('#previous-page');
    const next = document.querySelector('#next-page');
    const dialog = document.querySelector('#delete-dialog');

    function showTableMessage(text) {
        const row = rows.insertRow();
        const cell = row.insertCell();
        cell.colSpan = 3;
        cell.className = 'table-message';
        cell.textContent = text;
    }

    function render() {
        const query = search?.value.toLocaleLowerCase() || '';
        const filtered = files.filter(filename =>
            filename.toLocaleLowerCase().includes(query)
        ).sort((a, b) => a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base' }));
        page = Math.min(page, Math.max(0, Math.ceil(filtered.length / pageSize) - 1));
        const visible = filtered.slice(page * pageSize, (page + 1) * pageSize);
        rows.replaceChildren();

        for (const filename of visible) {
            const metadata = fileType(filename);
            const row = rows.insertRow();
            const nameCell = row.insertCell();
            const open = makeLink(filename, fileUrl(filename));
            open.className = 'file-name';
            open.target = '_blank';
            open.rel = 'noopener';
            nameCell.append(open);
            const typeCell = row.insertCell();
            typeCell.className = 'file-type';
            typeCell.textContent = metadata.label;
            const actions = document.createElement('div');
            actions.className = 'file-actions';
            if (metadata.group !== 'image') {
                actions.append(makeLink('Edit', `/edit_form.py?file=${encodeURIComponent(filename)}`, filename));
            }
            if (!home) {
                const remove = document.createElement('button');
                remove.type = 'button';
                remove.textContent = 'Delete';
                remove.dataset.delete = filename;
                remove.setAttribute('aria-label', `Delete ${filename}`);
                actions.append(remove);
            }
            row.insertCell().append(actions);
        }
        if (!filtered.length) {
            showTableMessage(files.length ? 'No matching files. Try another name.' : 'No files yet. Create or upload your first file to get started.');
        }
        count.textContent = home
            ? `${files.length} ${files.length === 1 ? 'file' : 'files'} in the directory`
            : `${filtered.length ? page * pageSize + 1 : 0}–${Math.min((page + 1) * pageSize, filtered.length)} of ${filtered.length} files`;
        if (previous) {
            previous.disabled = page === 0;
        }
        if (next) {
            next.disabled = (page + 1) * pageSize >= filtered.length;
        }
    }

    async function refreshFiles() {
        if (refresh) {
            refresh.disabled = true;
        }
        message.hidden = true;
        rows.setAttribute('aria-busy', 'true');
        try {
            files = await loadFiles();
            render();
        } catch (error) {
            rows.replaceChildren();
            showTableMessage('Your files could not be loaded.');
            message.textContent = error.message;
            message.dataset.state = 'error';
            message.hidden = false;
            count.textContent = 'Directory unavailable';
            if (previous) {
                previous.disabled = true;
            }
            if (next) {
                next.disabled = true;
            }
        } finally {
            rows.removeAttribute('aria-busy');
            if (refresh) {
                refresh.disabled = false;
            }
        }
    }

    search?.addEventListener('input', () => {
        page = 0;
        render();
    });
    refresh?.addEventListener('click', refreshFiles);
    previous?.addEventListener('click', () => {
        page--;
        render();
    });
    next?.addEventListener('click', () => {
        page++;
        render();
    });
    rows.addEventListener('click', event => {
        const button = event.target.closest('[data-delete]');
        if (!button) {
            return;
        }
        pendingDelete = button.dataset.delete;
        document.querySelector('#delete-filename').textContent = pendingDelete;
        document.querySelector('#delete-error').hidden = true;
        dialog.showModal();
    });
    document.querySelector('#confirm-delete')?.addEventListener('click', async event => {
        const button = event.currentTarget;
        const filename = pendingDelete;
        button.disabled = true;
        try {
            await saveFile('DELETE', filename);
            dialog.close();
            await refreshFiles();
            message.textContent = `Deleted ${filename}.`;
            message.dataset.state = 'success';
            message.hidden = false;
            refresh.focus();
        } catch (error) {
            const output = document.querySelector('#delete-error');
            output.textContent = error.message;
            output.hidden = false;
        } finally {
            button.disabled = false;
        }
    });
    refreshFiles();
}
