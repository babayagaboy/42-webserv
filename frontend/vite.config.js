import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import { defineConfig } from 'vite'

// https://vite.dev/config/
const server = 'http://127.0.0.1:8808'

export default defineConfig({
  plugins: [react(), tailwindcss()],
  base: '/react/',
  server: {
    cors: false,
    proxy: {
      '/files': server,
      '/patch_file.py': server,
      '/create_file.py': server,
      '/put_file.py': server,
      '/delete_file.py': server,
      '/head': server,
      '/options': server,
    },
  },
})
