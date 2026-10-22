import { useState } from "react";

function Post() {
  const [filename, setFilename] = useState("");
  const [content, setContent] = useState("");
  const [message, setMessage] = useState("");

  async function sendPost(event) {
    event.preventDefault();
    setMessage("Sending...");

    try {
      const response = await fetch("/create_file.py", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename, content }),
      });
      setMessage(await response.text());
    } catch (error) {
      setMessage(`POST failed: ${error.message}`);
    }
  }

  return (
    <section className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="mb-4 text-xl font-semibold text-slate-900">POST: create a file</h2>
      <form className="grid gap-3" onSubmit={sendPost}>
        <input className="rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-indigo-500 focus:ring-2 focus:ring-indigo-100" value={filename} onChange={(event) => setFilename(event.target.value)} placeholder="example.txt" required />
        <textarea className="min-h-28 rounded-lg border border-slate-300 px-3 py-2 outline-none focus:border-indigo-500 focus:ring-2 focus:ring-indigo-100" value={content} onChange={(event) => setContent(event.target.value)} placeholder="File content" required />
        <button className="w-fit rounded-lg bg-indigo-600 px-4 py-2 font-medium text-white hover:bg-indigo-700 disabled:opacity-50" type="submit">Create file</button>
      </form>
      {message && <p className="mt-3 rounded-lg bg-slate-100 p-3 text-sm text-slate-700">{message}</p>}
    </section>
  );
}

export default Post;
