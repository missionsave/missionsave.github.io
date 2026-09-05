import { EmailMessage } from "cloudflare:email";


const ALLOWED_ORIGINS = [
  "https://missionsave.github.io",
  "https://missionsave.org",
  "https://www.missionsave.org",
  "https://superdb-api.superbem.workers.dev",
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





// --- EMAILSENDTOME ENDPOINT (Resend) ---
if (url.pathname === "/emailsendtome" && request.method === "POST") {
  try {
    const data = await request.json().catch(() => null);

    if (!data) {
      return new Response(JSON.stringify({ error: "Invalid JSON" }), {
        status: 400,
        headers: { "Content-Type": "application/json", ...cors({}, request) }
      });
    }

    const name = (data.name || "").trim();
    const email = (data.email || "").trim();
    const message = (data.message || "").trim();

    if (!name || !email || !message) {
      return new Response(JSON.stringify({ error: "Missing fields" }), {
        status: 400,
        headers: { "Content-Type": "application/json", ...cors({}, request) }
      });
    }

    if (!env.MYEMAIL) {
      return new Response(JSON.stringify({ error: "MYEMAIL not configured" }), {
        status: 500,
        headers: { "Content-Type": "application/json", ...cors({}, request) }
      });
    }

    const domain = env.MYEMAIL.includes("@")
      ? env.MYEMAIL.split("@")[1]
      : "example.com";

    const payload = {
      from: `Contact Form <no-reply@${domain}>`,
      to: env.MYEMAIL,
      subject: `Contact form: ${name}`,
      text: `From: ${name} <${email}>\n\n${message}`
    };

    const resendRes = await fetch("https://api.resend.com/emails", {
      method: "POST",
      headers: {
        "Authorization": `Bearer ${env.RESEND_API_KEY}`,
        "Content-Type": "application/json"
      },
      body: JSON.stringify(payload)
    });

    const txt = await resendRes.text();

    if (!resendRes.ok) {
      return new Response(JSON.stringify({ error: "Send failed", details: txt }), {
        status: 502,
        headers: { "Content-Type": "application/json", ...cors({}, request) }
      });
    }

    return new Response(JSON.stringify({ ok: true }), {
      status: 200,
      headers: { "Content-Type": "application/json", ...cors({}, request) }
    });

  } catch (err) {
    return new Response(JSON.stringify({ error: err.message }), {
      status: 500,
      headers: { "Content-Type": "application/json", ...cors({}, request) }
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
        "https://api.github.com/repos/missionsave/missionsave.github.io/actions/workflows/cwin.yaml/dispatches",
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
// Global variables for caching
let MAP_CACHE = null;
let LAST_FETCH = 0;



    // 1. Helper for slugs
    const getSlug = (s) => s.toLowerCase().trim()
      .replace(/\s+/g, '_')
      .replace(/[^a-z0-9_]/g, '');

    // 2. Load + Cache JSON
    try {
      if (!MAP_CACHE || (Date.now() - LAST_FETCH > 300000)) {
        const r = await fetch("https://missionsave.github.io/tdoc.json", {
          cf: { cacheTtl: 300 }
        });
        if (r.ok) {
          const j = await r.json();
          MAP_CACHE = new Map();
          for (const [cat, arr] of Object.entries(j)) {
            MAP_CACHE.set(cat, cat);
            if (Array.isArray(arr)) {
              for (const it of arr) {
                if (it.title) {
                  MAP_CACHE.set(`${cat}/${getSlug(it.title)}`, it.title);
                }
              }
            }
          }
          LAST_FETCH = Date.now();
        }
      }
    } catch (e) {}

    // 3. Resolve path/title
    //let p = url.searchParams.get("p") || "";
  let p = url.searchParams.get("p");
    // If no ?p= exists, derive it from the path
if (!p) {
  const segments = url.pathname.split("/").filter(Boolean);
  if (segments.length > 0) p = segments[0];
}
if (!p) p="";

    if (url.pathname.startsWith("/og/")) {
      p = url.pathname.replace("/og/", "").replace(/\.(png|svg)$/, "");
    }
    p = p.replace(/^\/+|\/+$/g, "");

    let ogTitle = "MissionSave";
    if (MAP_CACHE) {
      ogTitle = MAP_CACHE.get(p) || MAP_CACHE.get(p.split("/")[0]) || "MissionSave";
    }

    
    // 4. ROUTE: Serve SVG directly
    function svgWrappedText(
  text,
  x = "80",
  y = "440",
  fill = "#38bdf8",
  fontSize = "40",
  fontFamily = "sans-serif",
  maxCharsPerLine = 48,
  lineHeightMultiplier = 1.2
) {
  const fs = Number(fontSize); // convert for arithmetic
  const lineHeight = fs * lineHeightMultiplier;

  const words = text.split(" ");
  let lines = [];
  let line = "";

  for (const w of words) {
    if ((line + w).length > maxCharsPerLine) {
      lines.push(line.trim());
      line = "";
    }
    line += w + " ";
  }
  if (line.trim()) lines.push(line.trim());

  const tspans = lines
    .map((ln, i) =>
      `<tspan x="${x}" dy="${i === 0 ? 0 : lineHeight}">${ln}</tspan>`
    )
    .join("");

  return `
<text x="${x}" y="${y}" fill="${fill}" font-size="${fontSize}" font-family="${fontFamily}">
  ${tspans}
</text>`;
}



    if (url.pathname.endsWith(".svg")) {
      const block = svgWrappedText(
  "40 fresh raw meals a day — from each autonomous, 40' container-sized greenhouse."
);
      const safeTitle = ogTitle.replace(/[<>&"]/g, "");
      const svg = `<svg width="1200" height="630" viewBox="0 0 1200 630" xmlns="http://www.w3.org/2000/svg">
        <rect width="100%" height="100%" fill="#0f172a"/>
        <text x="80" y="100" fill="#38bd08" font-size="40" font-family="sans-serif">MissionSave</text>
        <text x="80" y="240" fill="white" font-size="80" font-family="sans-serif" font-weight="bold">${safeTitle}</text>
        ${block}
      </svg>`;
      
      return new Response(svg, {
        headers: { 
          "Content-Type": "image/svg+xml", 
          "Cache-Control": "public, max-age=86400",
          "Access-Control-Allow-Origin": "*" 
        }
      });
    }

    // 5. ROUTE: PNG Proxy (Server-side fetch prevents loops)
// Replace Section 5 with this "Cache-First" version
if (url.pathname.startsWith("/og/") && url.pathname.endsWith(".png")) {
  const cache = caches.default;
  let response = await cache.match(request);

  // If we have it in the Cloudflare cache, return it immediately!
  if (response) return response;

  const svgUrl = url.origin + url.pathname.replace(".png", ".svg");
  const converterUrl = `https://wsrv.nl/?url=${encodeURIComponent(svgUrl)}&output=png&w=1200&h=630`;

  try {
    const imgRes = await fetch(converterUrl, {
      headers: { "User-Agent": "MissionSave-Worker/1.0" },
      cf: { cacheTtl: 8888 } // Tells Cloudflare to cache this fetch result
    });

    if (imgRes.ok) {
      // Reconstruct the response to make it "Cacheable"
      response = new Response(imgRes.body, {
        headers: {
          "Content-Type": "image/png",
          "Cache-Control": "public, max-age=8888",
          "X-Cache-Status": "MISS"
        }
      });

      // Store it in the edge cache
      await cache.put(request, response.clone());
      return response;
    }
  } catch (err) {
    return Response.redirect(svgUrl, 302);
  }
}

    // 6. Proxy to GitHub Pages
    let path = url.pathname;
    if (!path.endsWith("/") && !path.includes(".")) path += "/";
    //const targetUrl = "https://missionsave.github.io" + path + url.search;
    
    let targetUrl;

if (p) {
//targetUrl = "https://missionsave.github.io/?p=Prototype";
targetUrl = "https://missionsave.github.io/?p=" + encodeURIComponent(p);
} else {
  targetUrl = "https://missionsave.github.io" + path + url.search;
}

    const response = await fetch(targetUrl, { redirect: "manual" });
    if (response.status === 301 || response.status === 302) return response;

    // 7. Inject Meta Tags
    const safeP = p || "default";
    const ogImage = `${url.origin}/og/${safeP}.png`;

    return new HTMLRewriter()
      .on("head", {
        element(h) {
          h.append(`
            <meta property="og:title" content="${ogTitle}">
            <meta property="og:image" content="${ogImage}?v=1">
            <meta property="og:image:width" content="1200">
            <meta property="og:image:height" content="630">
            <meta property="og:type" content="website">
            <meta name="twitter:card" content="summary_large_image">
          `, { html: true });
        }
      })
      .transform(response);
    


  

  


  },




  // --- CRON trigger here ---
 async scheduled(event, env, ctx) {
    ctx.waitUntil(fetch(
      "https://api.github.com/repos/missionsave/missionsave.github.io/actions/workflows/cwin.yaml/dispatches",
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
