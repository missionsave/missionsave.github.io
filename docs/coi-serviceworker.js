if (typeof window === "undefined") {
	// ---- service worker context ----
	self.addEventListener("install", () => self.skipWaiting());
	self.addEventListener("activate", e => e.waitUntil(self.clients.claim()));
  
	self.addEventListener("fetch", e => {
	  const req = e.request;
	  if (req.cache === "only-if-cached" && req.mode !== "same-origin") return;
  
	  e.respondWith(
		fetch(req)
		  .then(r => {
			if (r.status === 0) return r; // opaque response, can't modify
			const h = new Headers(r.headers);
			h.set("Cross-Origin-Opener-Policy", "same-origin");
			h.set("Cross-Origin-Embedder-Policy", "require-corp");
			h.set("Cross-Origin-Resource-Policy", "cross-origin");
			return new Response(r.body, {
			  status: r.status,
			  statusText: r.statusText,
			  headers: h,
			});
		  })
		  .catch(err => { console.error(err); return Response.error(); })
	  );
	});
  } else {
	// ---- page context ----
	if (!window.crossOriginIsolated && navigator.serviceWorker) {
	  navigator.serviceWorker
		.register(document.currentScript.src)
		.then(reg => {
		  reg.addEventListener("updatefound", () => location.reload());
		  if (reg.active && !navigator.serviceWorker.controller) location.reload();
		})
		.catch(err => console.error("COI SW registration failed:", err));
	}
  }