import { useEffect, useState } from "react";

function Put() {
  const [files, setFiles] = useState([]);
  const [selectedFile, setSelectedFile] = useState("__new__");
  const [newFilename, setNewFilename] = useState("");
  const [content, setContent] = useState("");
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

  async function selectFile(event) {
    const filename = event.target.value;
    setSelectedFile(filename);
    setContent("");
    setMessage("");

    if (filename === "__new__") return;

    try {
      const response = await fetch(`/files/${encodeURIComponent(filename)}`);
      if (!response.ok) throw new Error(`Server returned ${response.status}`);
      setContent(await response.text());
    } catch (error) {
      setMessage(`Could not load ${filename}: ${error.message}`);
    }
  }

  async function sendPut(event) {
    event.preventDefault();
    const filename = selectedFile === "__new__" ? newFilename.trim() : selectedFile;
    setSending(true);
    setMessage("");

    try {
      const response = await fetch("/put_file.py", {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename, content }),
      });
      const result = await response.text();
      if (!response.ok) throw new Error(result || `Server returned ${response.status}`);

      setMessage(result);
      if (selectedFile === "__new__") {
        setFiles((current) => [...current, filename]);
        setSelectedFile(filename);
        setNewFilename("");
      }
    } catch (error) {
      setMessage(`PUT failed: ${error.message}`);
    } finally {
      setSending(false);
    }
  }

  return (
    <section className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="mb-4 text-xl font-semibold text-slate-900">PUT: create or replace a file</h2>
      <form className="grid gap-3" onSubmit={sendPut}>
        <label className="text-sm font-medium text-slate-900" htmlFor="put-file">Select a file</label>
        <select
          id="put-file"
          className="rounded-lg border border-slate-300 px-3 py-2 text-slate-900"
          value={selectedFile}
          onChange={selectFile} >
          <option value="__new__">Create new file</option>
          {files.map((file) => <option key={file} value={file}>{file}</option>)}
        </select>

        {selectedFile === "__new__" && (
          <input
            className="rounded-lg border border-slate-300 px-3 py-2 text-slate-900"
            value={newFilename}
            onChange={(event) => setNewFilename(event.target.value)}
            placeholder="new-file.txt"
            aria-label="New filename"
            required />
        )}

        <label className="text-sm font-medium text-slate-900" htmlFor="put-content">File content</label>
        <textarea
          id="put-content"
          className="min-h-28 rounded-lg border border-slate-300 px-3 py-2 text-slate-900"
          value={content}
          onChange={(event) => setContent(event.target.value)}
          placeholder="File content" />
        <button className="w-fit rounded-lg bg-indigo-600 px-4 py-2 font-medium text-white hover:bg-indigo-700 disabled:opacity-50" type="submit" disabled={sending}>
          {sending ? "Sending..." : "PUT file"}
        </button>
      </form>
      {message && <p className="mt-3 rounded-lg bg-slate-100 p-3 text-sm text-slate-700" role="status">{message}</p>}
    </section>
  );
}

export default Put;
