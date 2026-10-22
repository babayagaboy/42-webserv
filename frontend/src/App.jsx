import Home from "./pages/Home";
import Get from "./pages/Get";
import Post from "./pages/Post";
import Patch from "./pages/Patch";
import Put from "./pages/Put";
import Delete from "./pages/Delete";
import Head from "./pages/Head";
import Options from "./pages/Options";
import Navbar from "./components/Navbar";
import { HashRouter, Route, Routes } from "react-router-dom";

function App() {
  return (
    <HashRouter>
      <main className="min-h-screen bg-base-100 px-4 pb-10 pt-24 text-base-content max-[350px]:pt-36 sm:px-6 sm:pt-28">
        <div>
          <Navbar />
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/get" element={<Get />} />
            <Route path="/post" element={<Post />} />
            <Route path="/patch" element={<Patch />} />
            <Route path="/put" element={<Put />} />
            <Route path="/delete" element={<Delete />} />
            <Route path="/head" element={<Head />} />
            <Route path="/options" element={<Options />} />
          </Routes>
        </div>
      </main>
    </HashRouter>
  );
}

export default App;
