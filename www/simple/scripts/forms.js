import { request, readResult, loadFiles, saveFile, fileUrl, fileType, validateFilename } from './api.js';

function showResult(element, message, error = false, filename = '') {
    element.hidden = false;
    element.dataset.state = error ? 'error' : 'success';
    element.textContent = message;
    if (filename) {
        const link = document.createElement('a');
        link.textContent = 'View files';
        link.href = '/manager.html';
        element.append(link);
    }
}

function trackCharacters(textarea, output) {
    const update = () => { output.textContent = `${textarea.value.length.toLocaleString()} characters`; };
    textarea.addEventListener('input', update);
    update();
    return update;
}

export function initCreate() {
    const createForm = document.querySelector('#create-form');
    const createResult = document.querySelector('#create-result');
    trackCharacters(createForm.elements.content, document.querySelector('#content-count'));
    createForm.addEventListener('submit', async event => {
        event.preventDefault();
        const button = createForm.querySelector('button');
        button.disabled = true;
        try {
            const filename = createForm.elements.filename.value.trim();
            const message = await saveFile('POST', filename, createForm.elements.content.value);
            showResult(createResult, message, false, filename);
        } catch (error) {
            showResult(createResult, error.message, true);
        } finally {
            button.disabled = false;
        }
    });

    const uploadForm = document.querySelector('#upload-form');
    const input = document.querySelector('#upload-file');
    const selection = document.querySelector('#upload-selection');
    const result = document.querySelector('#upload-result');
    const dropzone = document.querySelector('#dropzone');
    let droppedFile = null;

    function showSelection(file) {
        const size = file ? `${(file.size / 1024).toLocaleString(undefined, { maximumFractionDigits: 1 })} KB` : '';
        selection.textContent = file ? `${file.name} · ${size}` : 'No file selected.';
        result.hidden = true;
    }
    input.addEventListener('change', () => {
        droppedFile = null;
        input.required = true;
        showSelection(input.files[0]);
    });
    dropzone.addEventListener('dragover', event => {
        event.preventDefault();
        dropzone.dataset.dragging = 'true';
    });
    dropzone.addEventListener('dragleave', () => { delete dropzone.dataset.dragging; });
    dropzone.addEventListener('drop', event => {
        event.preventDefault();
        delete dropzone.dataset.dragging;
        if (event.dataTransfer.files.length !== 1) {
            showResult(result, 'Choose one file at a time.', true);
            return;
        }
        droppedFile = event.dataTransfer.files[0];
        input.value = '';
        input.required = false;
        showSelection(droppedFile);
    });
    uploadForm.addEventListener('submit', async event => {
        event.preventDefault();
        const file = droppedFile || input.files[0];
        if (!file) { showResult(result, 'Choose a file before uploading.', true); return; }
        const button = uploadForm.querySelector('button');
        button.disabled = true;
        try {
            validateFilename(file.name);
            const files = await loadFiles();
            if (files.includes(file.name) && !window.confirm(`Replace ${file.name}? The existing file will be overwritten.`)) {
                return;
            }
            showResult(result, `Uploading ${file.name}…`);
            const body = new FormData();
            body.append('file', file);
            const response = await request('/upload_file.php', { method: 'POST', body });
            const message = await readResult(response);
            showResult(result, `${response.status} · ${message}`, false, file.name);
        } catch (error) {
            showResult(result, error.message, true);
        } finally {
            button.disabled = false;
        }
    });
}

export function initEditor() {
    const form = document.querySelector('#edit-form');
    const method = form.elements.method;
    const select = form.elements.filename;
    const newName = form.elements.new_filename;
    const content = form.elements.content;
    const saveButton = form.querySelector('button');
    const loadResult = document.querySelector('#load-result');
    const result = document.querySelector('#edit-result');
    const updateCount = trackCharacters(content, document.querySelector('#edit-count'));
    let dirty = false;
    let loadSequence = 0;
    let selectedFile = '';

    window.addEventListener('beforeunload', event => {
        if (dirty) { event.preventDefault(); event.returnValue = ''; }
    });
    content.addEventListener('input', () => { dirty = true; });
    newName.addEventListener('input', () => {
        if (newName.value) {
            loadSequence++;
            select.value = '';
            selectedFile = '';
            content.disabled = false;
            saveButton.disabled = false;
            loadResult.textContent = 'Saving will create or replace this filename.';
        }
        dirty = true;
    });

    async function openFile() {
        const filename = select.value;
        if (dirty && !window.confirm('Discard your unsaved changes and open another file?')) {
            select.value = selectedFile;
            return;
        }
        selectedFile = filename;
        const sequence = ++loadSequence;
        dirty = false;
        newName.value = '';
        content.value = '';
        updateCount();
        result.hidden = true;
        content.disabled = true;
        saveButton.disabled = true;
        if (!filename) {
            content.disabled = false;
            saveButton.disabled = false;
            loadResult.textContent = 'Choose a text file or enter a new filename.';
            return;
        }
        loadResult.textContent = `Loading ${filename}…`;
        try {
            if (fileType(filename).group === 'image') {
                throw new Error('This is an image. Choose a text file to edit.');
            }
            const response = await request(fileUrl(filename));
            if (!response.ok) {
                throw new Error(`Could not open the file (${response.status}). Choose it again to retry.`);
            }
            const blob = await response.blob();
            if (blob.size > 1024 * 1024) {
                throw new Error('This file is too large for the text editor (1 MB maximum). Download it to edit locally.');
            }
            const text = new TextDecoder('utf-8', { fatal: true }).decode(await blob.arrayBuffer());
            if (text.includes('\u0000')) {
                throw new Error('This file contains binary data. Download it instead of editing it as text.');
            }
            if (sequence !== loadSequence) {
                return;
            }
            content.value = text;
            content.disabled = false;
            saveButton.disabled = false;
            loadResult.textContent = `Loaded ${filename}. Ready to edit.`;
            updateCount();
        } catch (error) {
            if (sequence !== loadSequence) {
                return;
            }
            loadResult.textContent = error instanceof TypeError
                ? 'This file is not UTF-8 text. Download it instead of editing it here.'
                : error.message;
        }
    }

    select.addEventListener('change', openFile);
    method.addEventListener('change', () => {
        const patch = method.value === 'PATCH';
        document.querySelector('#new-filename-field').hidden = patch;
        newName.disabled = patch;
        if (patch) {
            newName.value = '';
        }
    });
    form.addEventListener('submit', async event => {
        event.preventDefault();
        const controls = [...form.elements];
        const disabledStates = controls.map(control => control.disabled);
        const filename = newName.value.trim() || select.value;
        const saveMethod = method.value;
        const text = content.value;
        if (newName.value.trim() && [...select.options].some(option => option.value === filename)) {
            if (!window.confirm(`Replace ${filename}? The existing file will be overwritten.`)) {
                return;
            }
        }
        controls.forEach(control => { control.disabled = true; });
        try {
            const message = await saveFile(saveMethod, filename, text);
            dirty = false;
            showResult(result, message, false, filename);
            if (![...select.options].some(option => option.value === filename)) {
                select.add(new Option(filename, filename));
            }
            select.value = filename;
            selectedFile = filename;
            newName.value = '';
        } catch (error) {
            showResult(result, error.message, true);
        } finally {
            controls.forEach((control, index) => { control.disabled = disabledStates[index]; });
        }
    });

    async function initializeFiles() {
        try {
            const files = await loadFiles();
            select.replaceChildren(new Option('Choose a saved file', ''));
            files.sort((a, b) => a.localeCompare(b)).forEach(filename => select.add(new Option(filename, filename)));
            const requested = new URLSearchParams(location.search).get('file');
            if (requested && files.includes(requested)) { select.value = requested; await openFile(); }
        } catch (error) {
            loadResult.textContent = error.message;
        }
    }
    initializeFiles();
}
