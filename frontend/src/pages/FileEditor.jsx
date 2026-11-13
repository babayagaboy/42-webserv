import { useEffect, useState } from "react";
import { Link, useSearchParams } from "react-router-dom";
import { loadFiles, sendFileRequest } from "../lib/files";

const methods = {
  POST: { label: "Create file", description: "Create a new file. The filename must not already exist." },
  PATCH: { label: "Update file", description: "Update an existing file with the content below." },
  PUT: { label: "Create or replace file", description: "Replace a file's full content, or create it if it does not exist." },
};

function FileEditor() {
  const [searchParams, setSearchParams] = useSearchParams();
  const requestedMethod = searchParams.get("method");
  const method = Object.hasOwn(methods, requestedMethod) ? requestedMethod : "POST";
  const [filename, setFilename] = useState(searchParams.get("file") || "");
  const [content, setContent] = useState("");
  const [files, setFiles] = useState([]);
  const [busy, setBusy] = useState(false);
  const [message, setMessage] = useState("");
  const [error, setError] = useState("");

  useEffect(() => {
    const controller = new AbortController();
    loadFiles(controller.signal).then(setFiles).catch((error) => {
      if (!controller.signal.aborted) setError(error.message);
    });
    return () => controller.abort();
  }, []);

  async function loadContent() {
    setBusy(true);
    setMessage("");
    setError("");
    try {
      const response = await fetch(`/files/${encodeURIComponent(filename.trim())}`);
      if (!response.ok) throw new Error(`Could not load file (${response.status}).`);
      setContent(await response.text());
      setMessage("File content loaded.");
    } catch (error) {
      setError(error.message);
    } finally {
      setBusy(false);
    }
  }

  async function submitFile(event) {
    event.preventDefault();
    const name = filename.trim();
    if (!name) return;
    setBusy(true);
    setMessage("");
    setError("");
    try {
      const result = await sendFileRequest(method, name, content);
      setMessage(result || `${name} saved.`);
      setFiles((current) => current.includes(name) ? current : [...current, name]);
    } catch (error) {
      setError(error.message);
    } finally {
      setBusy(false);
    }
  }

  function changeMethod(event) {
    setSearchParams({ method: event.target.value }, { replace: true });
    setMessage("");
    setError("");
  }

  return (
    <section className="mx-auto max-w-3xl" aria-labelledby="editor-title">
      <h1 id="editor-title" className="text-3xl font-semibold">File editor</h1>
      <p className="mb-8 mt-2 text-base-content/75">Create or edit text files in one place.</p>
      <form onSubmit={submitFile}>
        <fieldset className="grid gap-5" disabled={busy}>
          <div>
            <label className="mb-2 block text-sm font-medium" htmlFor="editor-method">Request method</label>
            <select id="editor-method" className="select select-bordered w-full" value={method} onChange={changeMethod} aria-describedby="method-description">
              {Object.entries(methods).map(([value, { label }]) => <option key={value} value={value}>{value} — {label}</option>)}
            </select>
            <p id="method-description" className="mt-2 text-sm text-base-content/75">{methods[method].description}</p>
          </div>
          <div>
            <label className="mb-2 block text-sm font-medium" htmlFor="editor-filename">Filename</label>
            <div className="flex gap-3">
              <input id="editor-filename" className="input input-bordered w-full" list={method === "POST" ? undefined : "existing-files"} value={filename} onChange={(event) => setFilename(event.target.value)} placeholder="example.txt" required />
              {method !== "POST" && <button className="btn btn-outline" type="button" onClick={loadContent} disabled={!filename.trim() || busy}>Load content</button>}
            </div>
            <datalist id="existing-files">{files.map((file) => <option key={file} value={file} />)}</datalist>
          </div>
          <div>
            <label className="mb-2 block text-sm font-medium" htmlFor="editor-content">File content</label>
            <textarea id="editor-content" className="textarea textarea-bordered min-h-64 w-full font-mono" value={content} onChange={(event) => setContent(event.target.value)} placeholder="Enter the file content" />
          </div>
          <div className="flex items-center gap-4">
            <button className="btn btn-primary" type="submit" disabled={busy || !filename.trim()}>{busy ? "Working…" : methods[method].label}</button>
            <Link className="link" to="/files">Browse files</Link>
          </div>
        </fieldset>
      </form>
      {error && <p className="mt-5 text-error" role="alert">{error}</p>}
      {message && <p className="mt-5 whitespace-pre-wrap text-success" role="status">{message}</p>}
    </section>
  );
}

export default FileEditor;
