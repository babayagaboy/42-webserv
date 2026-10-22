import { useState } from "react";

function Get() {
  const [files, setFiles] = useState(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  async function sendGet() {
    setLoading(true);
    setError("");

    try {
      const response = await fetch("/files");
      if (!response.ok) {
        throw new Error(`GET /files failed with ${response.status}`);
      }
      setFiles(await response.json());
    } catch (requestError) {
      setFiles(null);
      setError(requestError.message);
    } finally {
      setLoading(false);
    }
  }

  return (
    <>
      <button className="rounded-lg bg-slate-900 px-4 py-2 font-medium text-white hover:bg-slate-700 disabled:cursor-wait disabled:opacity-50" type="button" onClick={sendGet} disabled={loading}>
        {loading ? "Loading..." : "Send GET"}
      </button>
      {error && <p className="mt-3 text-sm text-red-600">{error}</p>}
      {files && <pre className="mt-4 max-h-64 overflow-auto rounded-lg bg-slate-950 p-4 text-sm text-emerald-300">{JSON.stringify(files, null, 2)}</pre>}
    </>
  );
};

export default Get;
