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
    <div>
      <p className="mb-4 text-sm text-base-content/75">{description}</p>
      <form className="grid gap-3" onSubmit={sendRequest}>
        <label className="text-sm font-medium text-base-content" htmlFor={`${method}-path`}>Resource path</label>
        <input
          id={`${method}-path`}
          className="input input-bordered w-full"
          value={path}
          onChange={(event) => setPath(event.target.value)}
          placeholder="/head"
          required
        />
        <button className="btn btn-primary w-fit" type="submit" disabled={loading}>
          {loading ? "Sending..." : `Send ${method}`}
        </button>
      </form>

      {error && <p className="mt-3 text-sm text-error" role="alert">{error}</p>}
      {result && (
        <div className="mt-4 text-sm text-base-content" role="status">
          <p><strong>Status:</strong> {result.status}</p>
          {method === "OPTIONS" && <p><strong>Allowed methods:</strong> {result.allow || "No Allow header"}</p>}
          <p className="mt-3 font-semibold">Response headers</p>
          <pre className="mt-2 overflow-auto rounded-lg bg-base-200 p-4 text-base-content">{result.headers}</pre>
        </div>
      )}
    </div>
  );
}

export default MethodInspector;
