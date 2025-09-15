const ALLOWED_ORIGINS = [
  "https://missionsave.github.io",
  "https://missionsave.org"
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





    // --- Proxy everything else to GitHub Pages ---
    const targetUrl = "https://missionsave.github.io" + url.pathname + url.search;

    // Forward the incoming request to GitHub Pages
    const response = await fetch(targetUrl, {
      method: request.method,
      headers: request.headers,
      body: request.method !== "GET" && request.method !== "HEAD" ? await request.clone().arrayBuffer() : undefined,
      redirect: "manual"
    });

    // Return the proxied response to the client
    return new Response(response.body, {
      status: response.status,
      headers: response.headers
    });
  },

  // --- CRON trigger here ---
 async scheduled(event, env, ctx) {
    ctx.waitUntil(fetch(
      "https://api.github.com/repos/missionsave/missionsave.github.io/actions/workflows/daily.yml/dispatches",
      {
        method: "POST",
        headers: {
          "Authorization": `Bearer ${env.GITHUB_TOKEN}`,
          "Accept": "application/vnd.github+json",
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ ref: "main" })
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
