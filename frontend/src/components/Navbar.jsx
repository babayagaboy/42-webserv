import { NavLink } from "react-router-dom";
import ThemeSelector from "./ThemeSelector";

const links = [
  { to: "/", label: "Home" },
  { to: "/get", label: "GET" },
  { to: "/post", label: "POST" },
  { to: "/patch", label: "PATCH" },
  { to: "/put", label: "PUT" },
  { to: "/delete", label: "DELETE" },
  { to: "/head", label: "HEAD" },
  { to: "/options", label: "OPTIONS" },
];

function Navbar() {
  return (
    <nav aria-label="Main navigation" className="fixed inset-x-0 top-0 z-50 border-b border-base-300 bg-base-200">
      <div className="mx-auto flex h-20 max-w-7xl items-center justify-between gap-4 px-4 sm:px-6">
        <span className="text-lg font-semibold tracking-tight text-base-content">webserver</span>

        <div className="flex items-center gap-1 sm:gap-2">
          {links.map(({ to, label }) => (
            <NavLink
              key={to}
              to={to}
              end={to === "/"}
              className="rounded-lg px-3 py-2 text-sm font-medium transition-colors focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary sm:px-4"
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
