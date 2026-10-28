/*! coi-serviceworker v0.1.7 - Fixed ReferenceError */
let coiConfig = (typeof window !== 'undefined' ? window.coi : undefined) || {};

if (typeof window === 'undefined') {
    // Service Worker context (Handles fetch events and adds COOP/COEP headers)
    self.addEventListener("install", () => self.skipWaiting());
    self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));

    self.addEventListener("fetch", function (event) {
        const r = event.request;
        if (r.cache === "only-if-cached" && r.mode !== "same-origin") return;

        event.respondWith(
            fetch(r).then((response) => {
                if (response.status === 0) return response;

                const newHeaders = new Headers(response.headers);
                newHeaders.set("Cross-Origin-Opener-Policy", coiConfig.coop || "same-origin");
                newHeaders.set("Cross-Origin-Embedder-Policy", coiConfig.coep || "require-corp");

                return new Response(response.body, {
                    status: response.status,
                    statusText: response.statusText,
                    headers: newHeaders,
                });
            }).catch((e) => console.error(e))
        );
    });
} else {
    // Page context - Capture currentScript SYNCHRONOUSLY right away
    const scriptSrc = document.currentScript ? document.currentScript.src : 'coi-serviceworker.js';

    if (window.crossOriginIsolated) {
        // Already cross-origin isolated, do nothing
    } else if (!navigator.serviceWorker) {
        console.warn("Service Workers are not supported in this browser.");
    } else {
        window.addEventListener("load", () => {
            navigator.serviceWorker.register(scriptSrc).then((registration) => {
                console.log("COI Service Worker registered with scope:", registration.scope);

                // Check if we are already controlled by the service worker
                if (!navigator.serviceWorker.controller) {
                    // Prevent reload loops using session storage flag
                    if (!sessionStorage.getItem("coi_reloaded")) {
                        sessionStorage.setItem("coi_reloaded", "true");
                        
                        // Wait until the service worker is fully active before reloading
                        navigator.serviceWorker.ready.then(() => {
                            window.location.reload();
                        });
                    }
                } else {
                    // Clear the flag once isolation is successfully working
                    sessionStorage.removeItem("coi_reloaded");
                }
            }).catch((error) => {
                console.error("COI Service Worker registration failed:", error);
            });
        });
    }
}