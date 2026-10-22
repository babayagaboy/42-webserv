import { useState } from "react";

function MethodInspector({ method, defaultPath, description }) {
  const [path, setPath] = useState(defaultPath);
  const [result, setResult] = useState(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  async function sendRequest(event) {
    event.preventDefault();
    setLoading(true);
    setResult(null);
    setError("");

    try {
      const response = await fetch(path.trim(), { method });
      const headers = Array.from(response.headers, ([name, value]) => `${name}: ${value}`);
      setResult({
        status: `${response.status} ${response.statusText}`.trim(),
        allow: response.headers.get("Allow"),
        headers: headers.join("\n") || "No response headers",
      });
    } catch (requestError) {
      setError(`${method} failed: ${requestError.message}`);
    } finally {
      setLoading(false);
    }
  }

  return (
    <section className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="mb-2 text-xl font-semibold text-slate-900">{method}: inspect a resource</h2>
      <p className="mb-4 text-sm text-slate-700">{description}</p>
      <form className="grid gap-3" onSubmit={sendRequest}>
        <label className="text-sm font-medium text-slate-900" htmlFor={`${method}-path`}>Resource path</label>
        <input
          id={`${method}-path`}
          className="rounded-lg border border-slate-300 px-3 py-2 text-slate-900"
          value={path}
          onChange={(event) => setPath(event.target.value)}
          placeholder="/head"
          required
        />
        <button className="w-fit rounded-lg bg-indigo-600 px-4 py-2 font-medium text-white hover:bg-indigo-700 disabled:opacity-50" type="submit" disabled={loading}>
          {loading ? "Sending..." : `Send ${method}`}
        </button>
      </form>

      {error && <p className="mt-3 text-sm text-red-600" role="alert">{error}</p>}
      {result && (
        <div className="mt-4 text-sm text-slate-900" role="status">
          <p><strong>Status:</strong> {result.status}</p>
          {method === "OPTIONS" && <p><strong>Allowed methods:</strong> {result.allow || "No Allow header"}</p>}
          <p className="mt-3 font-semibold">Response headers</p>
          <pre className="mt-2 overflow-auto rounded-lg bg-slate-950 p-4 text-emerald-300">{result.headers}</pre>
        </div>
      )}
    </section>
  );
}

export default MethodInspector;
