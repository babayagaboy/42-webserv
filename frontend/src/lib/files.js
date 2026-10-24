const endpoints = {
  POST: "/create_file.py",
  PATCH: "/patch_file.py",
  PUT: "/put_file.py",
  DELETE: "/delete_file.py",
};

export async function loadFiles(signal) {
  const response = await fetch("/files", { signal });
  if (!response.ok) throw new Error(`Could not load files (${response.status}).`);
  return response.json();
}

export async function sendFileRequest(method, filename, content) {
  const response = await fetch(endpoints[method], {
    method,
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ filename, content }),
  });
  const message = await response.text();
  if (!response.ok) throw new Error(message || `${method} failed (${response.status}).`);
  return message;
}
