import { useEffect, useState } from "react";

function Delete() {
  const [files, setFiles] = useState([]);
  const [filename, setFilename] = useState("");
  const [message, setMessage] = useState("");
  const [sending, setSending] = useState(false);

  useEffect(() => {
    async function loadFiles() {
      try {
        const response = await fetch("/files");
        if (!response.ok) throw new Error(`Server returned ${response.status}`);
        setFiles(await response.json());
      } catch (error) {
        setMessage(`Could not load files: ${error.message}`);
      }
    }

    loadFiles();
  }, []);

  async function sendDelete(event) {
    event.preventDefault();
    setSending(true);
    setMessage("");

    try {
      const response = await fetch("/delete_file.py", {
        method: "DELETE",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename }),
      });
      const result = await response.text();
      if (!response.ok) throw new Error(result || `Server returned ${response.status}`);

      setFiles((current) => current.filter((file) => file !== filename));
      setFilename("");
      setMessage(result || "File deleted successfully");
    } catch (error) {
      setMessage(`DELETE failed: ${error.message}`);
    } finally {
      setSending(false);
    }
  }

  return (
    <section className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="mb-4 text-xl font-semibold text-slate-900">DELETE: remove a file</h2>
      <form className="grid gap-3" onSubmit={sendDelete}>
        <label className="text-sm font-medium text-slate-900" htmlFor="delete-file">Select a file to delete</label>
        <select
          id="delete-file"
          className="rounded-lg border border-slate-300 px-3 py-2 text-slate-900"
          value={filename}
          onChange={(event) => setFilename(event.target.value)}
          required
        >
          <option value="">{files.length ? "Choose a file" : "No files available"}</option>
          {files.map((file) => <option key={file} value={file}>{file}</option>)}
        </select>
        <button className="w-fit rounded-lg bg-red-600 px-4 py-2 font-medium text-white hover:bg-red-700 disabled:opacity-50" type="submit" disabled={sending || !filename}>
          {sending ? "Deleting..." : "Delete file"}
        </button>
      </form>
      {message && <p className="mt-3 rounded-lg bg-slate-100 p-3 text-sm text-slate-700" role="status">{message}</p>}
    </section>
  );
}

export default Delete;
