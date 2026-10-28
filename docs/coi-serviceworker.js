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
	(async () => {
	  if (window.crossOriginIsolated || !("serviceWorker" in navigator)) return;
	  try {
		await navigator.serviceWorker.register(document.currentScript.src);
		await navigator.serviceWorker.ready; // waits until a worker is active
  
		if (!navigator.serviceWorker.controller) {
		  // reload once, with a guard so it can never loop
		  let done = false;
		  try { done = sessionStorage.getItem("coi-reloaded") === "1"; } catch (e) {}
		  if (!done) {
			try { sessionStorage.setItem("coi-reloaded", "1"); } catch (e) {}
			location.reload();
		  }
		}
	  } catch (err) {
		console.error("COI SW registration failed:", err);
	  }
	})();
  }