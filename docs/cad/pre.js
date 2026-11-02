(() => {
	// 1. Your existing WebAssembly.promising patch
	if (typeof WebAssembly.promising === 'function') {
	  const origGet = WebAssembly.Table.prototype.get;
	  const cache = new WeakMap();
	  WebAssembly.Table.prototype.get = function (i) {
		const f = origGet.call(this, i);
		if (typeof f !== 'function') return f;
		let w = cache.get(f);
		if (!w) { w = WebAssembly.promising(f); cache.set(f, w); }
		return w;
	  };
	}
  
	// 2. Monkey-patch global ImageData to prevent SharedArrayBuffer crashes
	const OriginalImageData = globalThis.ImageData;
	if (OriginalImageData) {
	  globalThis.ImageData = function (...args) {
		// Check if the first argument is a Uint8ClampedArray backed by a SharedArrayBuffer
		if (args[0] instanceof Uint8ClampedArray) {
		  if (args[0].buffer instanceof SharedArrayBuffer) {
			// Create a local, unshared copy of the pixel data
			args[0] = new Uint8ClampedArray(args[0]);
		  }
		}
		// Safely construct the native ImageData object
		return Reflect.construct(OriginalImageData, args, new.target || globalThis.ImageData);
	  };
	  globalThis.ImageData.prototype = OriginalImageData.prototype;
	}
  })();