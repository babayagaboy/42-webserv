import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import { defineConfig } from 'vite'

// https://vite.dev/config/
const backend = 'http://127.0.0.1:8808'

export default defineConfig({
  plugins: [react(), tailwindcss()],
  base: '/react/',
  server: {
    cors: false,
    proxy: {
      '/files': backend,
      '/patch_file.py': backend,
      '/create_file.py': backend,
      '/put_file.py': backend,
      '/delete_file.py': backend,
      '/head': backend,
      '/options': backend,
    },
  },
})
