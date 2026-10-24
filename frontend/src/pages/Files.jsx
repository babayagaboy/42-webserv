import { useEffect, useState } from "react";
import { Link, useSearchParams } from "react-router-dom";
import MethodInspector from "../components/MethodInspector";
import { loadFiles, sendFileRequest } from "../lib/files";

function Files() {
  const [searchParams, setSearchParams] = useSearchParams();
  const method = searchParams.get("method") === "OPTIONS" ? "OPTIONS" : "HEAD";
  const [files, setFiles] = useState([]);
  const [loading, setLoading] = useState(true);
  const [deleting, setDeleting] = useState("");
  const [confirmDelete, setConfirmDelete] = useState("");
  const [error, setError] = useState("");
  const [message, setMessage] = useState("");

  useEffect(() => {
    const controller = new AbortController();
    loadFiles(controller.signal)
      .then(setFiles)
      .catch((error) => {
        if (!controller.signal.aborted) setError(error.message);
      })
      .finally(() => {
        if (!controller.signal.aborted) setLoading(false);
      });
    return () => controller.abort();
  }, []);

  async function refreshFiles() {
    setLoading(true);
    setError("");
    setMessage("");
    setConfirmDelete("");
    try {
      setFiles(await loadFiles());
    } catch (error) {
      setError(error.message);
    } finally {
      setLoading(false);
    }
  }

  async function deleteFile(filename) {
    setDeleting(filename);
    setError("");
    setMessage("");
    try {
      await sendFileRequest("DELETE", filename);
      setFiles((current) => current.filter((file) => file !== filename));
      setConfirmDelete("");
      setMessage(`Deleted ${filename}.`);
    } catch (error) {
      setError(error.message);
    } finally {
      setDeleting("");
    }
  }

  return (
    <div className="mx-auto max-w-5xl space-y-10">
      <section aria-labelledby="files-title">
        <div className="mb-6 flex items-start justify-between gap-4">
          <div>
            <h1 id="files-title" className="text-3xl font-semibold">Files</h1>
            <p className="mt-2 text-base-content/75">Browse your files with GET, or inspect a resource below.</p>
          </div>
          <button className="btn btn-outline" onClick={refreshFiles} disabled={loading || Boolean(deleting)}>
            {loading ? "Loading…" : "Refresh files"}
          </button>
        </div>
        {error && <p className="mb-4 text-error" role="alert">{error}</p>}
        {message && <p className="mb-4 text-success" role="status">{message}</p>}
        {!loading && !error && files.length === 0 && (
          <p className="py-8">No files yet. <Link className="link link-primary" to="/editor">Create your first file</Link>.</p>
        )}
        <ul className="max-h-96 overflow-y-auto divide-y divide-base-300 border-y border-base-300" aria-label="Files" aria-busy={loading}>
          {files.map((filename) => (
            <li key={filename} className="flex flex-wrap items-center justify-between gap-4 py-4">
              <span className="min-w-0 break-all font-medium">{filename}</span>
              <div className="flex flex-wrap items-center gap-2">
                {confirmDelete === filename ? (
                  <>
                    <span className="text-sm">Permanently delete this file?</span>
                    <button className="btn btn-error btn-sm" disabled={Boolean(deleting)} onClick={() => deleteFile(filename)}>
                      {deleting === filename ? "Deleting…" : "Confirm delete"}
                    </button>
                    <button className="btn btn-ghost btn-sm" disabled={Boolean(deleting)} onClick={() => setConfirmDelete("")}>Cancel</button>
                  </>
                ) : (
                  <>
                    <Link className="btn btn-ghost btn-sm" to={`/editor?method=PATCH&file=${encodeURIComponent(filename)}`} aria-label={`Edit ${filename}`}>Edit</Link>
                    <button className="btn btn-ghost btn-sm text-error" disabled={loading || Boolean(deleting)} onClick={() => setConfirmDelete(filename)} aria-label={`Delete ${filename}`}>Delete</button>
                  </>
                )}
              </div>
            </li>
          ))}
        </ul>
      </section>

      <section aria-labelledby="inspect-title">
        <h2 id="inspect-title" className="mb-4 text-xl font-semibold">Inspect a resource</h2>
        <label className="mb-2 block text-sm font-medium" htmlFor="inspect-method">Request method</label>
        <select id="inspect-method" className="select select-bordered mb-4" value={method} onChange={(event) => setSearchParams({ method: event.target.value }, { replace: true })}>
          <option value="HEAD">HEAD — response headers</option>
          <option value="OPTIONS">OPTIONS — allowed methods</option>
        </select>
        <MethodInspector key={method} method={method} defaultPath={method === "HEAD" ? "/head" : "/options"} description={method === "HEAD" ? "Inspect response headers without downloading the body." : "See which HTTP methods the server allows for a resource."} />
      </section>
    </div>
  );
}

export default Files;
