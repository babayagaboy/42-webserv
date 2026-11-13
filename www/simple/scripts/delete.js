import { loadFiles, saveFile } from './api.js';

export function initDelete() {
    const form = document.querySelector('#delete-file-form');
    const choice = document.querySelector('#delete-file-choice');
    const button = document.querySelector('#delete-selected');
    const result = document.querySelector('#delete-file-result');
    const dialog = document.querySelector('#delete-dialog');
    const confirm = document.querySelector('#confirm-delete');
    const cancel = dialog.querySelector('[value="cancel"]');
    const errorMessage = document.querySelector('#delete-error');
    let pendingFile = '';

    function updateSelection() {
        button.disabled = !choice.value;
    }

    function showResult(message, error = false) {
        result.textContent = message;
        result.dataset.state = error ? 'error' : 'success';
        result.hidden = false;
    }

    async function refreshFiles() {
        choice.disabled = true;
        button.disabled = true;
        try {
            const files = await loadFiles();
            choice.replaceChildren(new Option(files.length ? 'Choose a saved file' : 'No files to delete', ''));
            files.sort((a, b) => a.localeCompare(b)).forEach(name => choice.add(new Option(name, name)));
            choice.disabled = files.length === 0;
        } catch (error) {
            choice.replaceChildren(new Option('File list unavailable', ''));
            showResult(error.message, true);
        }
    }

    choice.addEventListener('change', updateSelection);
    form.addEventListener('submit', event => {
        event.preventDefault();
        if (!choice.value) {
            return;
        }
        pendingFile = choice.value;
        document.querySelector('#delete-filename').textContent = pendingFile;
        errorMessage.hidden = true;
        dialog.showModal();
        cancel.focus();
    });

    dialog.addEventListener('cancel', event => {
        if (confirm.disabled) {
            event.preventDefault();
        }
    });

    confirm.addEventListener('click', async () => {
        const filename = pendingFile;
        confirm.disabled = true;
        cancel.disabled = true;
        button.disabled = true;
        choice.disabled = true;
        errorMessage.hidden = true;
        try {
            await saveFile('DELETE', filename);
            dialog.close();
            showResult(`Deleted ${filename}.`);
            await refreshFiles();
            choice.focus();
        } catch (error) {
            errorMessage.textContent = error.message;
            errorMessage.hidden = false;
            choice.disabled = false;
            updateSelection();
        } finally {
            confirm.disabled = false;
            cancel.disabled = false;
        }
    });
    refreshFiles();
}
