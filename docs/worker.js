const ALLOWED_ORIGINS = [
  "https://missionsave.github.io",
  "https://missionsave.org",
  "http://127.0.0.1:8080"
];

function cors(extra = {}, request) {
  const origin = request.headers.get("Origin");
  let allowOrigin = "null";

  if (ALLOWED_ORIGINS.includes(origin)) {
    allowOrigin = origin;
  }

  return {
    "Access-Control-Allow-Origin": allowOrigin,
    "Access-Control-Allow-Methods": "POST, OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type, Authorization",
    ...extra
  };
}

function isWrite(sql) {
  return /^\s*(insert|update|delete|create|drop|alter|replace|pragma|attach|vacuum|begin|commit|rollback)\b/i.test(sql);
}

export default {
  async fetch(request, env,ctx) {
    // --- Handle CORS preflight ---
    if (request.method === "OPTIONS") {
      return new Response(null, { headers: cors({}, request) });
    }

    const url = new URL(request.url);

    // --- Your D1 database API ---
    if (request.method === "POST" && url.pathname === "/sql") {
      if (!env.db) {
        return new Response("D1 binding ausente", { status: 500, headers: cors({}, request) });
      }

      let payload;
      try {
        payload = await request.json();
      } catch {
        return new Response("JSON inválido", { status: 400, headers: cors({}, request) });
      }

      const sql = (payload?.sql || "").trim();
      const params = Array.isArray(payload?.params) ? payload.params : [];

      if (!sql) {
        return new Response("Campo 'sql' é obrigatório", { status: 400, headers: cors({}, request) });
      }

      const wantsWrite = isWrite(sql);

      // Auth for write queries
      if (wantsWrite) {
        const auth = request.headers.get("Authorization") || "";
        const token = auth.startsWith("Bearer ") ? auth.slice(7) : null;
        if (!token || token !== env.API_KEY) {
          return new Response("Não autorizado", { status: 401, headers: cors({}, request) });
        }
      }

      try {
        if (wantsWrite) {
          const res = await env.db.prepare(sql).bind(...params).run();
          return new Response(JSON.stringify({
            success: true,
            changes: res.meta?.changes ?? undefined,
            lastRowId: res.meta?.last_row_id ?? undefined
          }), { headers: { ...cors({}, request), "Content-Type": "application/json" } });
        } else {
          const { results } = await env.db.prepare(sql).bind(...params).all();
          return new Response(JSON.stringify({ success: true, rows: results }), {
            headers: { ...cors({}, request), "Content-Type": "application/json" }
          });
        }
      } catch (err) {
        return new Response(JSON.stringify({ success: false, error: String(err) }), {
          status: 400,
          headers: { ...cors({}, request), "Content-Type": "application/json" }
        });
      }
    }



    if (url.pathname === '/binancegethist') {
      const symbol = url.searchParams.get('symbol') || 'BTCUSDT';
      const interval = url.searchParams.get('interval') || '1d';
      const limit = url.searchParams.get('limit') || '6';

      const binanceUrl = `https://data-api.binance.vision/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${limit}`;
      const res = await fetch(binanceUrl);

      return new Response(await res.text(), {
        status: res.status,
        headers: {
          "Content-Type": "application/json",
          "Access-Control-Allow-Origin": "*"
        }
      });
    }


    if (url.pathname === "/epub") {
      return handleEpubProxy(request, env, ctx);
    }



    // --- NEW: Proxy Euromillions API ---
    if (request.method === "GET" && url.pathname === "/euromillions") {
      try {
        const resp = await fetch("https://nunofcguerreiro.com/api-euromillions-json?result=all");
        const json = await resp.json();

        return new Response(JSON.stringify(json), {
          headers: { ...cors({}, request), "Content-Type": "application/json" }
        });
      } catch (err) {
        return new Response(JSON.stringify({ error: String(err) }), {
          status: 500,
          headers: { ...cors({}, request), "Content-Type": "application/json" }
        });
      }
    }









// -----------------------------------
// ENDPOINT /gethtml
// -----------------------------------
if (url.pathname === "/gethtml") {
  const target = url.searchParams.get("url");

  if (!target) {
    return new Response("Missing parameter: ?url=", { status: 400 });
  }

  const method = request.method;
  const body =
    method !== "GET" && method !== "HEAD"
      ? await request.text()
      : null;

  // Fetch ao servidor remoto (com headers reais)
  const resp = await fetch(target, {
    method,
    body,
    headers: {
      "User-Agent":
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0 Safari/537.36",
      "Accept": "application/json, text/plain, */*",
      "Referer": "https://www.jogossantacasa.pt/",
      "Origin": "https://www.jogossantacasa.pt"
    }
  });

  const contentType = resp.headers.get("Content-Type") || "text/plain";
  const data = await resp.text();

  // Resposta com CORS liberado
  return new Response(data, {
    status: resp.status,
    headers: {
      "Access-Control-Allow-Origin": "*",
      "Access-Control-Allow-Headers": "*",
      "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
      "Content-Type": contentType
    }
  });
}

    
 
    









    


    // 👇 manual trigger route
    if (url.pathname === "/trigger-github") {
      const resp = await fetch(
        "https://api.github.com/repos/missionsave/missionsave.github.io/actions/workflows/botrecent.yaml/dispatches",
        {
          method: "POST",
          headers: {
            "Authorization": `Bearer ${env.GITHUB_TOKEN}`,
            "Accept": "application/vnd.github+json",
            "Content-Type": "application/json",
            "User-Agent": "missionsave-worker" // 👈 required
          },
          body: JSON.stringify({ ref: "master" })
        }
      );

      const text = await resp.text();
      return new Response(
        `GitHub API status: ${resp.status}\n\n${text}`,
        { status: resp.ok ? 200 : 500 }
      );
    }






if (url.pathname === "/google-img") {

  const targetImage = url.searchParams.get("url");
  if (!targetImage) {
    return new Response("Missing url", { status: 400 });
  }

  const cache = caches.default;
  const cacheKey = new Request(request.url, request);

  const cached = await cache.match(cacheKey);
  if (cached) return cached;

  try {

    const imgResp = await fetch(targetImage, {
      headers: {
        "User-Agent": "Mozilla/5.0",
        "Referer": "https://docs.google.com/",
        "Accept": "image/avif,image/webp,image/apng,image/*,*/*;q=0.8"
      }
    });

    if (!imgResp.ok) return imgResp;

    const newHeaders = new Headers(imgResp.headers);

    newHeaders.set("Access-Control-Allow-Origin", "*");
    newHeaders.set("Cache-Control", "public, max-age=604800");

    newHeaders.delete("Cross-Origin-Resource-Policy");
    newHeaders.delete("Content-Security-Policy");

    const response = new Response(imgResp.body, {
      status: imgResp.status,
      headers: newHeaders
    });

    ctx.waitUntil(cache.put(cacheKey, response.clone()));

    return response;

  } catch (err) {
    return new Response("Proxy error", { status: 500 });
  }
}


if (url.pathname === "/clean-doc") {

  const docUrl = url.searchParams.get("url");

  if (!docUrl) {
    return new Response("Missing ?url=", {
      status: 400,
      headers: cors({}, request)
    });
  }

  const cache = caches.default;
  const cacheKey = new Request(request.url, request);

  const cached = await cache.match(cacheKey);

  if (cached) {
    return new Response(cached.body, {
      status: 200,
      headers: cors({ "Content-Type": "text/html; charset=utf-8" }, request)
    });
  }

  const res = await fetch(docUrl, {
    headers: { "User-Agent": "Mozilla/5.0" }
  });

  let html = await res.text();

  const marker = '<div id="contents">';
  const idx = html.indexOf(marker);

  if (idx === -1) {
    return new Response("Could not find contents marker", {
      status: 500,
      headers: cors({}, request)
    });
  }

  let cleaned = html.substring(idx);

  const workerUrl = new URL(request.url).origin;

  function proxyUrl(u) {
    return `${workerUrl}/google-img?url=${encodeURIComponent(u)}`;
  }

  function shouldProxy(u) {
    return (
      u.includes("googleusercontent.com") ||
      u.includes("gstatic.com") ||
      u.includes("docs.google.com/docs-images-rt")
    );
  }

  // -------- src ----------
  cleaned = cleaned.replace(
    /(src)=["'](https?:\/\/[^"']+)["']/gi,
    (m, attr, u) => shouldProxy(u) ? `${attr}="${proxyUrl(u)}"` : m
  );

  // -------- data-src ----------
  cleaned = cleaned.replace(
    /(data-src)=["'](https?:\/\/[^"']+)["']/gi,
    (m, attr, u) => shouldProxy(u) ? `${attr}="${proxyUrl(u)}"` : m
  );

  // -------- srcset ----------
  cleaned = cleaned.replace(
    /srcset=["']([^"']+)["']/gi,
    (match, list) => {

      const rewritten = list.split(",").map(part => {

        const [u, size] = part.trim().split(/\s+/);

        if (shouldProxy(u)) {
          return `${proxyUrl(u)} ${size || ""}`.trim();
        }

        return part.trim();

      }).join(", ");

      return `srcset="${rewritten}"`;
    }
  );

  // -------- CSS url() images ----------
  cleaned = cleaned.replace(
    /url\((https?:\/\/[^)]+)\)/gi,
    (match, u) => {
      if (shouldProxy(u)) {
        return `url(${proxyUrl(u)})`;
      }
      return match;
    }
  );

  // -------- remove scripts ----------
  cleaned = cleaned.replace(/<script[\s\S]*?<\/script>/gi, "");

  // -------- simplify styles ----------
  cleaned = cleaned.replace(/style="([^"]*)"/gi, (match, css) => {

    const cleanedCss = css
      .replace(/padding[^;]*;?/gi, "")
      .replace(/margin-(top|bottom|right)[^;]*;?/gi, "")
      .replace(/min-width[^;]*;?/gi, "")
      .replace(/max-width[^;]*;?/gi, "")
      .trim();

    return cleanedCss ? `style="${cleanedCss}"` : "";
  });

  const styleFix = `
  <style>
  body,#contents{margin:0!important;padding:0!important;max-width:100%!important}
  #contents>div{margin:0!important;padding:0!important}
  img{max-width:100%!important;height:auto!important;display:block}
  .doc-content{width:100%!important;padding:0!important}
  </style>
  `;

  const metaFix = `<meta name="referrer" content="no-referrer">`;

  const finalHtml = metaFix + styleFix + cleaned;

  const response = new Response(finalHtml, {
    headers: {
      ...cors({ "Content-Type": "text/html; charset=utf-8" }, request),
      "Cache-Control": "public, max-age=21600"
    }
  });

  ctx.waitUntil(cache.put(cacheKey, response.clone()));

  return response;
}












// --- Proxy everything else to GitHub Pages ---
let path = url.pathname;

// If path looks like a "directory" (no extension, not just "/"), add trailing slash
if (!path.endsWith("/") && !path.includes(".")) {
  path = path + "/";
}

const targetUrl = "https://missionsave.github.io" + path + url.search;

const response = await fetch(targetUrl, {
  method: request.method,
  headers: request.headers,
  body: request.method !== "GET" && request.method !== "HEAD"
    ? await request.clone().arrayBuffer()
    : undefined,
  redirect: "manual"
});

return new Response(response.body, {
  status: response.status,
  headers: response.headers
});

  },

  // --- CRON trigger here ---
 async scheduled(event, env, ctx) {
    ctx.waitUntil(fetch(
      "https://api.github.com/repos/missionsave/missionsave.github.io/actions/workflows/botrecent.yaml/dispatches",
      {
        method: "POST",
        headers: {
          "Authorization": `Bearer ${env.GITHUB_TOKEN}`,
          "Accept": "application/vnd.github+json",
          "Content-Type": "application/json",
          "User-Agent": "missionsave-worker" // 👈 required
        },
        body: JSON.stringify({ ref: "master" })
      }
    ));
  }
};

// EPUB proxy handler with caching and origin check
async function handleEpubProxy(request, env, ctx) {
  const reqUrl = new URL(request.url);
  const originHeader = request.headers.get("Origin") || "";
  const target = reqUrl.searchParams.get("url");

  // Allow only missionsave.github.io or missionsave.org
  const allowed = ALLOWED_ORIGINS.includes(originHeader);

  // Build CORS headers
  const corsHeaders = {
    "Access-Control-Allow-Origin": allowed ? originHeader : "null",
    "Access-Control-Allow-Methods": "GET,HEAD,POST,OPTIONS",
    "Access-Control-Allow-Headers": "*",
  };

  // Handle preflight
  if (request.method === "OPTIONS") {
    return new Response(null, { headers: corsHeaders });
  }

  if (!target) {
    return new Response("Missing ?url=", { status: 400, headers: corsHeaders });
  }

  const cacheKey = new Request(request.url, request);
  const cache = caches.default;

  // Try cache first
  let response = await cache.match(cacheKey);
  if (response) {
    // Ensure only our CORS headers are present
    response = new Response(response.body, response);
    response.headers.delete("Access-Control-Allow-Origin");
    response.headers.delete("Access-Control-Allow-Methods");
    response.headers.delete("Access-Control-Allow-Headers");
    Object.entries(corsHeaders).forEach(([k, v]) => response.headers.set(k, v));
    return response;
  }

  // Fetch from origin
  try {
    const originResp = await fetch(target);
    if (!originResp.ok) {
      return new Response(`Failed to fetch target: ${originResp.status}`, {
        status: originResp.status,
        headers: corsHeaders
      });
    }

    // Clone and strip upstream CORS headers
    response = new Response(originResp.body, originResp);
    response.headers.delete("Access-Control-Allow-Origin");
    response.headers.delete("Access-Control-Allow-Methods");
    response.headers.delete("Access-Control-Allow-Headers");

    // Set correct content type and our CORS headers
    response.headers.set("Content-Type", "application/epub+zip");
    Object.entries(corsHeaders).forEach(([k, v]) => response.headers.set(k, v));

    // Cache for 1 day
    ctx.waitUntil(cache.put(cacheKey, response.clone()));

    return response;
  } catch (err) {
    return new Response("Error fetching target: " + err.message, {
      status: 500,
      headers: corsHeaders
    });
  }
}


async function handleOtherRoutes(request) {
  return new Response("Default handler here");
}
