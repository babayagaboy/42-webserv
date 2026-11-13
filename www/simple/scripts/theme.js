(() => {
    const root = document.documentElement;
    const savedCookie = document.cookie.split('; ').find(cookie => cookie.startsWith('webserv_theme='));
    root.dataset.theme = savedCookie === 'webserv_theme=dark' ? 'dark' : 'light';

    async function requestTheme(theme) {
        const options = {
            cache: 'no-store',
            signal: AbortSignal.timeout(10000),
        };
        if (theme) {
            options.method = 'POST';
            options.body = new URLSearchParams({ theme });
        }
        const response = await fetch('/theme.py', options);
        if (!response.ok) {
            throw new Error('Could not save the theme. Please try again.');
        }
        const result = await response.json();
        if (result.theme !== 'light' && result.theme !== 'dark') {
            throw new Error('The server returned an invalid theme.');
        }
        return result.theme;
    }

    document.addEventListener('DOMContentLoaded', async () => {
        const button = document.querySelector('#theme-toggle');

        function applyTheme(theme) {
            root.dataset.theme = theme;
            const label = theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode';
            button.setAttribute('aria-label', label);
            button.title = label;
        }

        applyTheme(root.dataset.theme);
        button.hidden = false;
        button.disabled = true;
        try {
            applyTheme(await requestTheme());
        } catch (error) {
            // Keep the saved appearance if the server is temporarily unavailable.
        } finally {
            button.disabled = false;
        }

        button.addEventListener('click', async () => {
            const theme = root.dataset.theme === 'dark' ? 'light' : 'dark';
            button.disabled = true;
            try {
                await requestTheme(theme);
                const savedTheme = await requestTheme();
                if (savedTheme !== theme) {
                    throw new Error('The theme cookie was not saved. Allow cookies and try again.');
                }
                applyTheme(savedTheme);
            } catch (error) {
                alert('Could not change the theme. Check your connection and allow cookies, then try again.');
            } finally {
                button.disabled = false;
            }
        });
    });
})();
