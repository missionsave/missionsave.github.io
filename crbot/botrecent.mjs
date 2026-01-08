// npm install node-fetch technicalindicators vader-sentiment rss-parser

import fetch from "node-fetch";
import { EMA, RSI, MACD } from "technicalindicators";
import vader from "vader-sentiment";
import Parser from "rss-parser";

const parser = new Parser();

// =========================
// SYMBOLS
// =========================
const symbs = [
  { name: "Bitcoin", pair: "BTCUSDT", cg: "bitcoin", wb: "BTC_PERP" },
  { name: "Etherum", pair: "ETHUSDT", cg: "ethereum", wb: "ETH_PERP" },
  { name: "Sol", pair: "SOLUSDT", cg: "solana", wb: "SOL_PERP" },
  { name: "Xrp", pair: "XRPUSDT", cg: "ripple", wb: "XRP_PERP" },
  { name: "Ada", pair: "ADAUSDT", cg: "cardano", wb: "ADA_PERP" },
  { name: "Tron", pair: "TRXUSDT", cg: "tron", wb: "TRX_PERP" },
  { name: "PAX Gold", pair: "PAXGUSDT", cg: "pax-gold", wb: "PAXG_PERP" },
];

// =========================
// FETCH PRICE HISTORY (BINANCE)
// =========================
async function fetchPrices(pair) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${pair}&interval=1h&limit=200`;
  const res = await fetch(url);
  if (!res.ok) throw new Error(`Binance API failed for ${pair}: ${res.status}`);

  const data = await res.json();
  if (!Array.isArray(data)) throw new Error(`Invalid Binance response for ${pair}`);

  return data.map(c => parseFloat(c[4])); // close prices
}

// =========================
// FUNDAMENTALS (COINGECKO)
// =========================
const CG_KEY = process.env.CG_KEY;

async function fetchFundamentals(coinId) {
  const url = `https://api.coingecko.com/api/v3/coins/${coinId}?localization=false&market_data=true&community_data=true&developer_data=true`;

  const res = await fetch(url, {
    headers: {
      "User-Agent": "Mozilla/5.0",
      "x-cg-demo-api-key": CG_KEY
    }
  });

  const d = await res.json();

  function safe(v, fallback = 0) {
    return (v === null || v === undefined || Number.isNaN(v)) ? fallback : v;
  }

  return {
    marketCap: safe(d.market_data?.market_cap?.usd, 0),
    volume24h: safe(d.market_data?.total_volume?.usd, 0),
    priceChange24h: safe(d.market_data?.price_change_percentage_24h, 0),
    devCommits: safe(d.developer_data?.commit_count_4_weeks, 0),
    twitterFollowers: safe(d.community_data?.twitter_followers, 0),
    redditSubs: safe(d.community_data?.reddit_subscribers, 0)
  };
}

// =========================
// RETRY WRAPPER (FUNDAMENTALS)
// =========================
function sleep(ms) {
  return new Promise(res => setTimeout(res, ms));
}

function isFundamentalsValid(f) {
  return (
    f.marketCap > 0 &&
    f.volume24h > 0 &&
    f.priceChange24h !== null &&
    f.priceChange24h !== undefined
  );
}

async function fetchFundamentalsWithRetry(coinId, retries = 3) {
  for (let i = 0; i < retries; i++) {
    try {
      const f = await fetchFundamentals(coinId);

      if (isFundamentalsValid(f)) {
        return f;
      }

      console.log(`⚠ Fundamentals incomplete for ${coinId}, retrying...`);
    } catch (err) {
      console.log(`⚠ Error fetching fundamentals for ${coinId}: ${err.message}`);
    }

    await sleep(5000 * (i + 1)); // exponential backoff
  }

  console.log(`❌ Fundamentals failed for ${coinId}, using fallback`);
  return {
    marketCap: 5_000_000,
    volume24h: 100_000,
    priceChange24h: 0,
    devCommits: 1,
    twitterFollowers: 500,
    redditSubs: 0
  };
}

// =========================
// HELPERS
// =========================
function normalize(value, min, max) {
  const clamped = Math.min(max, Math.max(min, value));
  return ((clamped - min) / (max - min)) * 100;
}

function timeDecayWeight(hoursAgo = 0, lambda = 0.15) {
  return Math.exp(-lambda * hoursAgo);
}

// =========================
// FUNDAMENTALS SCORE
// =========================
function computeFundamentalsScore(f, hoursAgo = 0) {
  const w = timeDecayWeight(hoursAgo);

  const mc = f.marketCap > 0 ? f.marketCap : 5_000_000;
  const vol = f.volume24h > 0 ? f.volume24h : 100_000;
  const commits = f.devCommits > 0 ? f.devCommits : 1;
  const community = (f.twitterFollowers + f.redditSubs) > 0
    ? (f.twitterFollowers + f.redditSubs)
    : 500;

  const marketCapScore = Math.min(100, Math.log10(mc) * 10);
  const volumeScore = Math.min(100, Math.log10(vol) * 10);
  const priceScore = Math.max(0, Math.min(100, f.priceChange24h + 50));
  const devScore = Math.min(100, commits * 2);
  const communityScore = Math.min(100, Math.log10(community) * 10);

  const fundamentals =
    0.35 * marketCapScore +
    0.25 * volumeScore +
    0.20 * devScore +
    0.10 * communityScore +
    0.10 * priceScore;

  return fundamentals * w;
}

// =========================
// NEWS SENTIMENT ENGINE (TWITTER → REDDIT → RSS)
// =========================
async function fetchTweets(keyword, limit = 50) {
  const url = `https://nitter.net/search?f=tweets&q=${encodeURIComponent(keyword)}&since=&until=&near=`;

  try {
    const res = await fetch(url, { timeout: 8000 });
    const html = await res.text();

    const tweets = [...html.matchAll(/<p class="tweet-content">([\s\S]*?)<\/p>/g)]
      .map(m => m[1].replace(/<[^>]+>/g, "").trim())
      .slice(0, limit);

    return tweets;
  } catch (e) {
    console.log("Twitter scrape failed:", e.message);
    return [];
  }
}

async function fetchNewsHeadlines(keyword, limit = 30) {
  const url = `https://news.google.com/rss/search?q=${encodeURIComponent(keyword)}&hl=en-US&gl=US&ceid=US:en`;

  try {
    const feed = await parser.parseURL(url);
    const titles = feed.items.map(i => i.title).slice(0, limit);
    return titles;
  } catch (e) {
    console.log("Google News RSS failed:", e.message);
    return [];
  }
}




async function fetchRSSHeadlines() {
  try {
    const feed = await parser.parseURL("https://www.coindesk.com/arc/outboundfeeds/rss/");
    return feed.items.map(i => i.title);
  } catch (e) {
    console.log("RSS scrape failed:", e.message);
    return [];
  }
}

function scoreSentiment(texts) {
  if (!texts.length) return 50;

  let total = 0;
  for (const t of texts) {
    const s = vader.SentimentIntensityAnalyzer.polarity_scores(t).compound;
    total += s;
  }

  const avg = total / texts.length;
  return ((avg + 1) / 2) * 100; // normalize -1..1 → 0..100
}

// Retry + fallback chain per symbol
async function getNewsScore(symbol) {
  const keyword = symbol.toUpperCase();

  // 1. Twitter (via Nitter)
  let tweets = await fetchTweets(keyword, 80);
  if (tweets.length > 5) return scoreSentiment(tweets);

  // Retry Twitter
  tweets = await fetchTweets(keyword, 80);
  if (tweets.length > 5) return scoreSentiment(tweets);

  // 2. Google News (substitui Reddit)
  const news = await fetchNewsHeadlines(keyword, 40);
  if (news.length > 3) return scoreSentiment(news);

  // 3. Coindesk RSS fallback
  const rss = await fetchRSSHeadlines();
  if (rss.length > 3) return scoreSentiment(rss);

  // 4. fallback neutro
  return 50;
}


// =========================
// TECHNICAL + FUNDAMENTAL + NEWS BULLISH SCORE
// =========================
function generateBullishScore(prices, fundamentalsScore, newsScore) {
  const close = prices;

  const ema20 = EMA.calculate({ period: 20, values: close });
  const ema50 = EMA.calculate({ period: 50, values: close });
  const rsiArr = RSI.calculate({ period: 14, values: close });
  const macdArr = MACD.calculate({
    fastPeriod: 12,
    slowPeriod: 26,
    signalPeriod: 9,
    values: close,
    SimpleMAOscillator: false,
    SimpleMASignal: false
  });

  const last = close.length - 1;
  const lastEMA20 = ema20[ema20.length - 1];
  const lastEMA50 = ema50[ema50.length - 1];
  const lastRSI = rsiArr[rsiArr.length - 1];
  const lastMACD = macdArr[macdArr.length - 1];

  const emaRatio = (lastEMA20 - lastEMA50) / lastEMA50;
  const emaScore = normalize(emaRatio, -0.05, 0.05);

  const macdDiff = lastMACD.MACD - lastMACD.signal;
  const macdScore = normalize(macdDiff, -100, 100);

  const rsiDistance = Math.abs(lastRSI - 50);
  const rsiScore = normalize(50 - rsiDistance, 0, 50);

  const bullishScore =
    0.28 * emaScore +
    0.28 * macdScore +
    0.18 * rsiScore +
    0.16 * fundamentalsScore +
    0.10 * newsScore;

  let signal = "HOLD";
  let sl = 0;
  let tp = 0;

  if (bullishScore >= 75) { signal = "STRONG BUY"; sl = 2; tp = 6; }
  else if (bullishScore >= 55) { signal = "BUY"; sl = 3; tp = 4; }
  else if (bullishScore <= 25) { signal = "STRONG SELL"; sl = 2; tp = 6; }
  else if (bullishScore <= 45) { signal = "SELL"; sl = 3; tp = 4; }

  return {
    price: close[last],
    ema20: lastEMA20,
    ema50: lastEMA50,
    rsi: lastRSI,
    macd: lastMACD,
    emaScore,
    macdScore,
    rsiScore,
    fundamentalsScore,
    newsScore,
    bullishScore: Math.round(bullishScore * 10) / 10,
    signal,
    stopLossPercent: sl,
    takeProfitPercent: tp
  };
}

// =========================
// MAIN LOOP
// =========================
async function run() {
  console.log("=== BULLISH SCORE SCAN ===");
  console.log(new Date().toISOString());

  for (let s of symbs) {
    try {
      const [prices, fundamentals, newsScore] = await Promise.all([
        fetchPrices(s.pair),
        fetchFundamentalsWithRetry(s.cg),
        getNewsScore(s.name)
      ]);

      const fundamentalsScore = computeFundamentalsScore(fundamentals, 0);
      const result = generateBullishScore(prices, fundamentalsScore, newsScore);
      s.result = result;

      console.log(`\n${s.name} (${s.pair})`);
      console.log(result);

    } catch (err) {
      console.error(`Error processing ${s.name}:`, err.message);
    }
  }
}

await run();

// =========================
// WHITEBIT SECTION
// =========================
 
/**
 * WhiteBIT Collateral (Futures) – HTTP Trade v4
 * Complete, API-correct implementation
 *
 * Requirements:
 *   Node.js >= 18 (for native fetch)
 *   export WB_KEY=your_key
 *   export WB_SECRET=your_secret
 */ 
/**
 * WhiteBIT HTTP Trade v4 – Collateral
 * Fully aligned with official documentation
 *
 * Env:
 *   export WB_KEY=xxxx
 *   export WB_SECRET=yyyy
 */
 

/* =========================================================
   CONFIG
   ========================================================= */ 
import crypto from "crypto";

let markets = null;

const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET;
const BASE = "https://whitebit.com";

if (!API_KEY || !API_SECRET) {
  throw new Error("WB_KEY and WB_SECRET must be defined");
}

/* =========================================================
   SIGNING
   ========================================================= */
function sign(body) {
  const bodyStr = JSON.stringify(body);
  const payload = Buffer.from(bodyStr).toString("base64");
  const signature = crypto.createHmac("sha512", API_SECRET).update(payload).digest("hex");
  return { bodyStr, payload, signature };
}

/* =========================================================
   POST HELPER
   ========================================================= */
async function wbPost(path, body = {}) {
  const { bodyStr, payload, signature } = sign(body);
  const res = await fetch(BASE + path, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "X-TXC-APIKEY": API_KEY,
      "X-TXC-PAYLOAD": payload,
      "X-TXC-SIGNATURE": signature
    },
    body: bodyStr
  });
  if (!res.ok) {
    const txt = await res.text();
    throw new Error(`[${res.status}] ${txt}`);
  }
  return res.json();
}

/* =========================================================
   MARKETS & PRECISION
   ========================================================= */
async function getMarkets() {
  if (!markets) {
    const res = await fetch("https://whitebit.com/api/v4/public/markets");
    markets = await res.json();
  }
  return markets;
}

function getMarketInfo(symbol) {
  const m = markets.find(mkt => mkt.name === symbol);
  if (!m) throw new Error(`Market ${symbol} not found`);
  const qtyPrec = Number(m.stockPrec || 8);  // quantity decimals
  const pricePrec = Number(m.moneyPrec || 2);
  return {
    minQty: Number(m.minAmount || 0.0001),
    qtyStep: Math.pow(10, -qtyPrec),
    priceStep: Math.pow(10, -pricePrec),
    minNotional: 5  // for trigger orders
  };
}

function roundToStep(value, step) {
  return Math.floor(value / step) * step;  // conservative floor for safety
}

/* =========================================================
   EQUITY & POSITION
   ========================================================= */
async function get_equity() {
  const bodyObj = {
	request: '/api/v4/collateral-account/summary',
	nonce: Date.now().toString()
  };
  const bodyStr = JSON.stringify(bodyObj);
  const payload = Buffer.from(bodyStr).toString('base64');
  const signature = crypto.createHmac('sha512', API_SECRET).update(payload).digest('hex');
  const res = await fetch(`https://whitebit.com/api/v4/collateral-account/summary`, {
	method: 'POST',
	headers: {
	  'Content-Type': 'application/json',
	  'X-TXC-APIKEY': API_KEY,
	  'X-TXC-PAYLOAD': payload,
	  'X-TXC-SIGNATURE': signature
	},
	body: bodyStr
  }); 
  if (!res.ok) {
	const text = await res.text();
	throw new Error(`[HTTP ${res.status}] ${text}`);
  }
  const resjs=await res.json();
//   console.log(Number(resjs.margin)+Number(resjs.freeMargin));
  return Number(resjs.margin)+Number(resjs.freeMargin);
}

async function getSymbolPosition(symbol) {
  const positions = await wbPost("/api/v4/collateral-account/positions/open", {
    request: "/api/v4/collateral-account/positions/open",
    nonce: Date.now().toString()
  });
  return positions.find(p => p.market === symbol) || null;
}

/* =========================================================
   CANCEL EXISTING OCO (from conditional orders)
   ========================================================= */
async function cancelExistingOco(symbol) {
  try {
    const res = await wbPost("/api/v4/conditional-orders", {
      request: "/api/v4/conditional-orders",
      nonce: Date.now().toString(),
      market: symbol,
      limit: 100
    });

    const records = Array.isArray(res?.records) ? res.records : [];

    for (const order of records) {
      if (order.type === "oco" && order.id) {
        await wbPost("/api/v4/order/oco-cancel", {
          request: "/api/v4/order/oco-cancel",
          nonce: Date.now().toString(),
          market: symbol,
          orderId: order.id
        });

        console.log(`Canceled OCO ID: ${order.id}`);
      }
    }
  } catch (e) {
    console.warn(
      "Cancel OCO failed or no OCO (safe):",
      e?.response?.data || e?.message || e
    );
  }
}

/* =========================================================
   PLACE LIMIT ENTRY
   ========================================================= */
async function placeLimitOrder_v1(symbol, side, amount, price) {
  return wbPost("/api/v4/order/collateral/limit", {
    request: "/api/v4/order/collateral/limit",
    nonce: Date.now().toString(),
    market: symbol,
    side,
    amount: amount.toFixed(8),
    price: price.toFixed(2),
    postOnly: true
  });
}
async function placeLimitOrder(symbol, side, qty, price, postOnly = false) {
  return wbPost("/api/v4/order/collateral/limit", {
    request: "/api/v4/order/collateral/limit",
    nonce: Date.now().toString(),
    market: symbol,
    side,
    amount: qty.toString(),
    price: price.toString(),
    type: "limit",
    postOnly: postOnly === true   // ← IMPORTANT
  });
}

/* =========================================================
   PLACE SEPARATE OCO (true editable OCO)
   ========================================================= */
async function placeOco(symbol, qty, slTrigger, tpPrice, isLong) {
  const closeSide = isLong ? "sell" : "buy";
  // SL leg: trigger-market (stop market)
  // TP leg: limit
  // activation_price for SL trigger
  // price for TP limit
  // stop_limit_price ≈ slTrigger for conservative execution
  return wbPost("/api/v4/order/collateral/oco", {
    request: "/api/v4/order/collateral/oco",
    nonce: Date.now().toString(),
    market: symbol,
    side: closeSide,
    amount: qty.toFixed(8),
    price: tpPrice.toFixed(2),               // TP limit price
    activation_price: slTrigger.toFixed(2),  // SL trigger
    stop_limit_price: slTrigger.toFixed(2)   // SL execution price (market-like)
  });
}
async function placeOco_v1(symbol, qty, slTrigger, tpPrice, isLong) {
  const closeSide = isLong ? "sell" : "buy";
  // SL leg: trigger-market (stop market)
  // TP leg: limit
  // activation_price for SL trigger
  // price for TP limit
  // stop_limit_price ≈ slTrigger for conservative execution
  return wbPost("/api/v4/order/collateral/oco", {
    request: "/api/v4/order/collateral/oco",
    nonce: Date.now().toString(),
    market: symbol,
    side: closeSide,
    amount: qty.toString(),
    price: tpPrice.toString(),               // TP limit price
    activation_price: slTrigger.toString(),  // SL trigger
    stop_limit_price: slTrigger.toString()   // SL execution price (market-like)
  });
}

/* =========================================================
   BID/ASK
   ========================================================= */
async function getBidAsk(symbol) {
  const res = await fetch(`${BASE}/api/v4/public/orderbook/${symbol}?depth=0`);
  if (!res.ok) throw new Error("Orderbook fetch failed");
  const data = await res.json();
  return {
    bid: Number(data.bids[0][0]),
    ask: Number(data.asks[0][0])
  };
}

/* =========================================================
   MAIN LOGIC - Separate editable OCO + Precision
   ========================================================= */
export async function open_order(
  symbol = "BTC_PERP",
  side = "buy",
  slPerc = 2,
  tpPerc = 6,
  equity = 100
) {
  await getMarkets();
  const info = getMarketInfo(symbol);
  const { minQty, qtyStep, priceStep, minNotional } = info;

  const isLong = side === "buy";
  const oppositeSide = isLong ? "sell" : "buy";

  const { bid, ask } = await getBidAsk(symbol);
  const entry = isLong ? bid : ask;

  // ----------------------------
  // Risk-based sizing (YOUR MODEL)
  // If SL hits → loss = equity * slPerc%
  // ----------------------------
  const riskValue = equity * (slPerc / 100);

  const slPriceRaw = isLong
    ? entry * (1 - slPerc / 100)
    : entry * (1 + slPerc / 100);

  const slDistance = Math.abs(entry - slPriceRaw);

  let targetQty = slDistance > 0
    ? riskValue / slDistance
    : 0;

  // ----------------------------
  // Enforce minimums
  // ----------------------------
  const minQtyFromNotional = minNotional / entry;
  const enforcedMinQty = Math.max(minQty, minQtyFromNotional);

  if (targetQty < enforcedMinQty) {
    targetQty = enforcedMinQty;
  }

  function countDecimals(step) {
  const s = step.toString();
  if (s.includes(".")) {
    return s.split(".")[1].length;
  }
  return 0;
}

  function normalizeQty(qty, qtyStep) { const decimals = countDecimals(qtyStep); const scale = 10 ** decimals; return Math.floor(qty * scale) / scale; }

  // FLOOR to step (never ceil)
  targetQty = Math.floor(targetQty / qtyStep) * qtyStep;
  targetQty=normalizeQty(targetQty,qtyStep);

// console.log(qtyStep,targetQty);

// return;
  if (targetQty < minQty) {
    console.warn("Qty below exchange minimum after flooring");
    return;
  }

  // ----------------------------
  // Current position
  // ----------------------------
  const pos = await getSymbolPosition(symbol);

  let currentQty = 0;
  let currentIsLong = null;

  if (pos && Number(pos.amount) !== 0) {
    currentQty = Math.abs(Number(pos.amount));
	// console.log(pos);
    // currentIsLong = pos.side.toLowerCase() === "long";
    currentIsLong = Number(pos.amount)>0;
  }

console.log("currentQty",currentQty);
// return;

  // ----------------------------
  // SL / TP prices (percent-based)
  // ----------------------------
  let slTrigger = isLong
    ? entry * (1 - slPerc / 100)
    : entry * (1 + slPerc / 100);

  let tpPrice = isLong
    ? entry * (1 + tpPerc / 100)
    : entry * (1 - tpPerc / 100);

  slTrigger = Math.round(slTrigger / priceStep) * priceStep;
  tpPrice = Math.round(tpPrice / priceStep) * priceStep;

  // ----------------------------
  // SAME SIDE & SAME SIZE
  // → only update OCO
  // ----------------------------
	console.log("qtyStep",qtyStep,currentIsLong,isLong,currentIsLong === isLong);
  if (
    currentIsLong === isLong &&
    Math.abs(currentQty - targetQty) < qtyStep
  ) {
	console.log("only update OCO");
    await cancelExistingOco(symbol);
    await placeOco(symbol, currentQty, slTrigger, tpPrice, isLong);
    // await placeOco(symbol, targetQty, slTrigger, tpPrice, isLong);
    return;
  }

//   return;
  // ----------------------------
  // SAME SIDE → INCREASE
  // ----------------------------
  if (currentIsLong === isLong && targetQty > currentQty) {
    const delta = Math.floor((targetQty - currentQty) / qtyStep) * qtyStep;
	console.log("SAME SIDE → INCREASE",delta);
	const ismorethan5=delta*entry>5;
	if (!ismorethan5){
		console.log("Less than 5, skipping...");
		targetQty=currentQty;
	}
    if (ismorethan5 && delta >= minQty) {
      await placeLimitOrder(symbol, side, delta, entry);
    }
  }

  // ----------------------------
  // SAME SIDE → PARTIAL CLOSE
  // ----------------------------
  if (currentIsLong === isLong && targetQty < currentQty) { 
    const delta = Math.floor((currentQty - targetQty) / qtyStep) * qtyStep;
	console.log("SAME SIDE → PARTIAL CLOSE",delta);
	const ismorethan5=delta*entry>5;
	if (!ismorethan5){
		console.log("Less than 5, skipping...");
		targetQty=currentQty;
	}
    if (ismorethan5 && delta >= minQty) {
      await placeLimitOrder(symbol, oppositeSide, delta, entry);
    }
  } 

  // ----------------------------
  // FLIP POSITION (explicit)
  // ----------------------------
  if (currentIsLong !== null && currentIsLong !== isLong) {
	console.log("FLIP POSITION");
    await placeLimitOrder(
      symbol,
      currentIsLong ? "sell" : "buy",
      currentQty,
      entry
    );

    await placeLimitOrder(symbol, side, targetQty, entry);
  }

  // ----------------------------
  // NO POSITION → OPEN
  // ----------------------------
  if (currentIsLong === null) {
    await placeLimitOrder(symbol, side, targetQty, entry);
  }

  // ----------------------------
  // New OCO (always reflects latest SL/TP)
  // ----------------------------
  await cancelExistingOco(symbol);
  await placeOco(symbol, targetQty, slTrigger, tpPrice, isLong);
}

/* =========================================================
   EXECUTION
   ========================================================= */
(async () => {
  try {
    const equity = await get_equity();
    console.log("Equity:", equity);

	const args = process.argv.slice(2); 
	if(args[0]=="equity"){
		console.log("Args:", args);
		return;
	}

	for(let i=0;i<symbs.length;i++){
		console.log(symbs[i].wb,symbs[i].result.signal,symbs[i].result.stopLossPercent,symbs[i].result.takeProfitPercent);
		if(symbs[i].result.signal=="HOLD")continue;
		let side="buy";
		if(symbs[i].result.signal == "STRONG SELL" || symbs[i].result.signal == "SELL")
			side="sell";
		const result = await open_order(symbs[i].wb, side, symbs[i].result.stopLossPercent, symbs[i].result.takeProfitPercent, equity/10.0);
		console.log("Order result:", result);
	}



    // const result = await open_order("BTC_PERP", "buy", 1, 1, 1000);
    // // const result = await open_order("BTC_PERP", "buy", 1, 1, equity);
    // console.log("Order result:", result);
  } catch (err) {
    console.error("ERROR:", err.message);
  }finally { process.exit(0); }
})();