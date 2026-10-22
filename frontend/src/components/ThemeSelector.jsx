import { useEffect, useState } from "react";
import { PaletteIcon } from "lucide-react";

const THEMES = [
  "light",
  "dark",
  "cupcake",
  "bumblebee",
  "emerald",
  "corporate",
  "synthwave",
  "retro",
  "cyberpunk",
  "valentine",
  "halloween",
  "garden",
  "forest",
  "aqua",
  "lofi",
  "pastel",
  "fantasy",
  "wireframe",
  "black",
  "luxury",
  "dracula",
  "cmyk",
  "autumn",
  "business",
  "acid",
  "lemonade",
  "night",
  "coffee",
  "winter",
  "dim",
  "nord",
  "sunset",
];

function ThemeSelector() {
  const [theme, setTheme] = useState(() => {
    if (typeof window !== "undefined") {
      return localStorage.getItem("theme") || "forest";
    }
    return "forest";
  });

  useEffect(() => {
    document.documentElement.setAttribute("data-theme", theme);
    localStorage.setItem("theme", theme);
  }, [theme]);

  return (
    <div className="dropdown dropdown-end navbar-theme-dropdown">
      <button
        type="button"
        aria-label="Choose a theme"
        className="btn btn-ghost btn-square ml-1 focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary"
      >
        <PaletteIcon className="size-5" aria-hidden="true" />
      </button>

      <ul
        tabIndex={0}
        className="dropdown-content navbar-theme-menu menu z-50 max-h-96 w-56 flex-nowrap overflow-y-auto rounded-box bg-base-200 p-2 shadow-xl"
      >
        {THEMES.map((option) => (
          <li key={option}>
            <button
              type="button"
              onClick={() => setTheme(option)}
              className={`flex justify-between ${
                theme === option ? "bg-primary text-primary-content" : ""
              }`}
            >
              <span className="capitalize">{option}</span>
              <span className="flex gap-0.5" data-theme={option} aria-hidden="true">
                <span className="h-4 w-2 rounded-sm bg-primary" />
                <span className="h-4 w-2 rounded-sm bg-secondary" />
                <span className="h-4 w-2 rounded-sm bg-accent" />
                <span className="h-4 w-2 rounded-sm bg-neutral" />
              </span>
            </button>
          </li>
        ))}
      </ul>
    </div>
  );
}

export default ThemeSelector;
