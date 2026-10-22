import { useState } from "react";

function Patch() {
  const [filename, setFilename] = useState("");
  const [content, setContent] = useState("");
  const [message, setMessage] = useState("");

  async function sendPatch(event) {
    event.preventDefault();
    setMessage("Sending...");

    try {
      const response = await fetch("/patch_file.py", {
        method: "PATCH",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename, content }),
      });
      setMessage(await response.text());
    } catch (error) {
      setMessage(`PATCH failed: ${error.message}`);
    }
  }

  return (
    <section className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="mb-4 text-xl font-semibold text-slate-900">
        PATCH: update a file
      </h2>
      <form className="grid gap-3" onSubmit={sendPatch}>
        <input
          className="rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-indigo-500 focus:ring-2 focus:ring-indigo-100"
          value={filename}
          onChange={(event) => setFilename(event.target.value)}
          placeholder="existing-file.txt"
          required
        />
        <textarea
          className="min-h-28 rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-indigo-500 focus:ring-2 focus:ring-indigo-100"
          value={content}
          onChange={(event) => setContent(event.target.value)}
          placeholder="New file content"
          required
        />
        <button
          className="w-fit rounded-lg bg-emerald-600 px-4 py-2 font-medium text-white hover:bg-emerald-700 disabled:opacity-50"
          type="submit"
        >
          Update file
        </button>
      </form>
      {message && (
        <p className="mt-3 rounded-lg bg-slate-100 p-3 text-sm text-slate-700">
          {message}
        </p>
      )}
    </section>
  );
}

export default Patch;
