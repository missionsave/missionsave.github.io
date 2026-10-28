/*! coi-serviceworker v0.1.7 - With Worker-level Body Hiding */
let coiConfig = (typeof window !== 'undefined' ? window.coi : undefined) || {};

if (typeof window === 'undefined') {
    // Service Worker context
    self.addEventListener("install", () => self.skipWaiting());
    self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));

    self.addEventListener("fetch", function (event) {
        const r = event.request;
        if (r.cache === "only-if-cached" && r.mode !== "same-origin") return;

        event.respondWith(
            fetch(r).then(async (response) => {
                if (response.status === 0) return response;

                let body = response.body;

                // If the user is requesting the main HTML page, inject a hiding style into <head>
                if (r.destination === 'document' || (r.headers.get('accept') && r.headers.get('accept').includes('text/html'))) {
                    try {
                        const htmlText = await response.text();
                        // Inject a style right after <head> to hide the body instantly
                        const injectedHtml = htmlText.replace(
                            '<head>',
                            '<head><style id="coi-loader-style">body { opacity: 0 !important; background: #1e1e1e !important; }</style>'
                        );

                        const newHeaders = new Headers(response.headers);
                        newHeaders.set("Cross-Origin-Opener-Policy", coiConfig.coop || "same-origin");
                        newHeaders.set("Cross-Origin-Embedder-Policy", coiConfig.coep || "require-corp");
                        // Remove content-length since we modified the body size
                        newHeaders.delete("content-length");

                        return new Response(injectedHtml, {
                            status: response.status,
                            statusText: response.statusText,
                            headers: newHeaders,
                        });
                    } catch (err) {
                        console.error("Failed to inject COI loader style:", err);
                    }
                }

                // Standard response handling for non-HTML assets (scripts, images, etc.)
                const newHeaders = new Headers(response.headers);
                newHeaders.set("Cross-Origin-Opener-Policy", coiConfig.coop || "same-origin");
                newHeaders.set("Cross-Origin-Embedder-Policy", coiConfig.coep || "require-corp");

                return new Response(body, {
                    status: response.status,
                    statusText: response.statusText,
                    headers: newHeaders,
                });
            }).catch((e) => console.error(e))
        );
    });
} else {
    // Page context
    const scriptSrc = document.currentScript ? document.currentScript.src : 'coi-serviceworker.js';

    if (window.crossOriginIsolated) {
        // Already isolated! Remove the injected hiding style so the page becomes visible
        window.addEventListener("DOMContentLoaded", () => {
            const loaderStyle = document.getElementById('coi-loader-style');
            if (loaderStyle) loaderStyle.remove();
        });
    } else if (!navigator.serviceWorker) {
        console.warn("Service Workers are not supported in this browser.");
    } else {
        window.addEventListener("load", () => {
            navigator.serviceWorker.register(scriptSrc).then((registration) => {
                if (!navigator.serviceWorker.controller) {
                    if (!sessionStorage.getItem("coi_reloaded")) {
                        sessionStorage.setItem("coi_reloaded", "true");
                        navigator.serviceWorker.ready.then(() => {
                            window.location.reload();
                        });
                    }
                } else {
                    sessionStorage.removeItem("coi_reloaded");
                    // Once controlled and isolated, clean up the loader style
                    const loaderStyle = document.getElementById('coi-loader-style');
                    if (loaderStyle) loaderStyle.remove();
                }
            }).catch((error) => {
                console.error("COI Service Worker registration failed:", error);
            });
        });
    }
}