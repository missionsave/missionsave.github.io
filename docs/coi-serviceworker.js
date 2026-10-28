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
			if (r.status === 0) return r; // opaque
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
		  .catch(err => {
			console.error(err);
			return Response.error();
		  })
	  );
	});
  } else {
	// ---- page context ----
	(async () => {
	  // Skip completely in Facebook / Messenger / Instagram in-app browsers.
	  // These environments have broken or incomplete Service Worker support.
	  const ua = navigator.userAgent || "";
	  if (/FBAN|FBAV|Messenger|Instagram|FB_IAB|FB4A|FBIOS/i.test(ua)) {
		return;
	  }
  
	  if (window.crossOriginIsolated || !("serviceWorker" in navigator)) return;
  
	  // document.currentScript can be null in some edge cases
	  const script = document.currentScript;
	  if (!script || !script.src) {
		console.warn("COI SW: no currentScript.src available");
		return;
	  }
  
	  try {
		const reg = await navigator.serviceWorker.register(script.src);
		await navigator.serviceWorker.ready;
  
		if (!navigator.serviceWorker.controller) {
		  // reload once, with a guard so it can never loop
		  let done = false;
		  try {
			done = sessionStorage.getItem("coi-reloaded") === "1";
		  } catch (_) {}
		  if (!done) {
			try {
			  sessionStorage.setItem("coi-reloaded", "1");
			} catch (_) {}
			location.reload();
		  }
		}
	  } catch (err) {
		console.error("COI SW registration failed:", err);
	  }
	})();
  }