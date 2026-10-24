import { HashRouter, Navigate, Route, Routes } from "react-router-dom";
import Navbar from "./components/Navbar";
import Home from "./pages/Home";
import Files from "./pages/Files";
import FileEditor from "./pages/FileEditor";

function App() {
  return (
    <HashRouter>
      <Navbar />
      <main className="min-h-screen bg-base-100 px-4 pb-10 pt-24 text-base-content sm:px-6 sm:pt-28">
        <Routes>
          <Route path="/" element={<Home />} />
          <Route path="/files" element={<Files />} />
          <Route path="/editor" element={<FileEditor />} />
          <Route path="/get" element={<Navigate to="/files" replace />} />
          <Route path="/head" element={<Navigate to="/files?method=HEAD" replace />} />
          <Route path="/options" element={<Navigate to="/files?method=OPTIONS" replace />} />
          <Route path="/post" element={<Navigate to="/editor?method=POST" replace />} />
          <Route path="/patch" element={<Navigate to="/editor?method=PATCH" replace />} />
          <Route path="/put" element={<Navigate to="/editor?method=PUT" replace />} />
          <Route path="/delete" element={<Navigate to="/files" replace />} />
        </Routes>
      </main>
    </HashRouter>
  );
}

export default App;
