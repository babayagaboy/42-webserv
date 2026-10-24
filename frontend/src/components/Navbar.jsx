import { NavLink } from "react-router-dom";
import ThemeSelector from "./ThemeSelector";

const links = [
  { to: "/files", label: "Files" },
  { to: "/editor", label: "File editor" },
];

function Navbar() {
  return (
    <nav aria-label="Main navigation" className="fixed inset-x-0 top-0 z-50 border-b border-base-300 bg-base-200">
      <div className="mx-auto flex h-20 max-w-7xl items-center justify-between gap-4 px-4 sm:px-6">
        <NavLink
          to="/"
          end
          aria-label="webserver"
          className="-ml-2 rounded-lg px-2 py-1 text-lg font-semibold tracking-tight text-base-content
               transition-colors hover:text-primary focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary">
          webserver
        </NavLink>

        <div className="flex items-center gap-1 sm:gap-2">
          {links.map(({ to, label }) => (
            <NavLink
              key={to}
              to={to}
              className={({ isActive }) => `rounded-lg px-3 py-2 text-sm font-medium transition-colors hover:bg-base-300 focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary sm:px-4 ${isActive ? "bg-base-300" : ""}`}
            >
              {label}
            </NavLink>
          ))}
          <ThemeSelector />
        </div>
      </div>
    </nav>
  );
}

export default Navbar;
