(() => {
  if (!WebAssembly.promising) return;
  const origGet = WebAssembly.Table.prototype.get;
  const cache = new WeakMap();
  WebAssembly.Table.prototype.get = function (i) {
    const f = origGet.call(this, i);
    if (typeof f !== 'function') return f;
    let w = cache.get(f);
    if (!w) { w = WebAssembly.promising(f); cache.set(f, w); }
    return w;
  };
})();