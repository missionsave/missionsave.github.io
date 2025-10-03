const crypto = require('crypto');
const fs = require('fs');
const vm = require('vm');
const path = require('path');

vm.runInThisContext(fs.readFileSync(path.join(__dirname, 'brain.min.js'), 'utf8'));
const { LSTMTimeStep } = brain.recurrent;

// const symbols = [{ name: "Ethereum", pair: "XRPUSDT", nc: "ETH_PERP" }];
// const symbols = [{ name: "Ethereum", pair: "SOLUSDT", nc: "ETH_PERP" }];
// const symbols = [{ name: "Ethereum", pair: "ETHUSDT", nc: "ETH_PERP" },
				//  { name: "Ethereum", pair: "BTCUSDT", nc: "ETH_PERP" }
// ];
// const symbols = [{ name: "Ethereum", pair: "SUIUSDT", nc: "ETH_PERP" }];
// const symbols = [{ name: "Ethereum", pair: "TIAUSDT", nc: "ETH_PERP" }];
// const symbols = [{ name: "Ethereum", pair: "BTCUSDT", nc: "ETH_PERP" }];
const symbols = [{ name: "Ethereum", pair: "ETHUSDT", nc: "ETH_PERP" }];
let nextc = 0, nexth = 0, nextl= 0, interval = "1d", lb = 0;
const historyLength = 901 + lb, RERUNS = 9, TRAIN_ITER = 400*4, atleast = 0.01, budget = 100, eps = 1e-9;
const portfolio = [];

async function fetchData(sym) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  return await (await fetch(url)).json();
}
async function fetchData_v1save(sym, outFile = "data_v1.json") {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  const res = await fetch(url);
  const data = await res.json();
  fs.writeFileSync(outFile, JSON.stringify(data));
  console.log(`Saved ${data.length} records for ${sym} → ${outFile}`);
  return data;
}

function normalize(rows) {
  const f = rows[0].length, mins = Array(f).fill(Infinity), maxs = Array(f).fill(-Infinity);
  for (let i = 0; i < rows.length; i++) for (let j = 0; j < f; j++) {
    const v = rows[i][j]; if (v < mins[j]) mins[j] = v; if (v > maxs[j]) maxs[j] = v;
  }
  const norm = [];
  for (let i = 0; i < rows.length; i++) {
    const row = []; for (let j = 0; j < f; j++) row.push((rows[i][j] - mins[j]) / (maxs[j] - mins[j] || 1));
    norm.push(row);
  }
  return { norm, mins, maxs };
} 
function denorm(r, mins, maxs) {
  const out = []; for (let i = 0; i < r.length; i++) out.push(r[i] * (maxs[i] - mins[i]) + mins[i]);
  return out;
} 
function median(a) {
  const s = [...a].sort((x, y) => x - y), m = Math.floor(s.length / 2);
  return s.length % 2 ? s[m] : (s[m - 1] + s[m]) / 2;
} 
// function trainOnce(rows) {
//   const { norm, mins, maxs } = normalize(rows);
//   const net = new LSTMTimeStep({ inputSize: 4, hiddenLayers: [16], outputSize: 4 });
//   net.train(norm, { iterations: TRAIN_ITER, learningRate: .1 });
//   return denorm(net.run(norm.slice(-1)), mins, maxs);
// } 
// function predict(rows, runs = RERUNS) {
//   const h = [], l = [], c = [], v = [];
//   for (let i = 0; i < runs; i++) {
//     const [H, L, C, V] = trainOnce(rows); h.push(H); l.push(L); c.push(C); v.push(V);
//   }
//   return [median(h), median(l), median(c), median(v)];
// }
function trainOnce(rows, win) {
  // use last `win` rows instead of full history
  const slice = rows.slice(-win);
  const { norm, mins, maxs } = normalize(slice);
  const net = new LSTMTimeStep({ inputSize: 4, hiddenLayers: [4], outputSize: 4 });
  net.train(norm, { iterations: TRAIN_ITER, learningRate: 0.1 });
  return denorm(net.run(norm.slice(-1)), mins, maxs);
}

function predict(rows, runs = RERUNS) {
  const windowSizes = [10,20,50];   // <-- multiple windows
  const h = [], l = [], c = [], v = [];
  for (let i = 0; i < runs; i++) {
    for (let w of windowSizes) {
      if (rows.length < w) continue; // skip if not enough data
      const [H, L, C, V] = trainOnce(rows, w);
      h.push(H); l.push(L); c.push(C); v.push(V);
    }
  }
  return [median(h), median(l), median(c), median(v)];
}


function signal(pred, prev) {
  const diff = ((pred - prev) / prev) * 100; let s = "HOLD", sl, tp;
  if (diff > atleast) { s = "buy"; sl = prev * .98; tp = prev * 1.03; }
  else if (diff < -atleast) { s = "sell"; sl = prev * 1.02; tp = prev * .97; }
  return { s, diff, sl, tp };
}
async function simulate(raw) {
  let sumper = 0;

  // --- safe normalization helper ---
  function safeNormalize(rows) {
    if (!rows || rows.length === 0) return { norm: [], mins: [], maxs: [] };
    const cols = rows[0].length;
    const mins = Array(cols).fill(Infinity);
    const maxs = Array(cols).fill(-Infinity);

    // find min/max
    for (const r of rows) {
      for (let i = 0; i < cols; i++) {
        if (r[i] < mins[i]) mins[i] = r[i];
        if (r[i] > maxs[i]) maxs[i] = r[i];
      }
    }

    // normalize safely
    const norm = rows.map(r =>
      r.map((v, i) =>
        maxs[i] === mins[i] ? 0.5 : (v - mins[i]) / (maxs[i] - mins[i])
      )
    );
    return { norm, mins, maxs };
  }

  // --- safe denorm ---
  function safeDenorm(arr, mins, maxs) {
    return arr.map((v, i) => v * (maxs[i] - mins[i]) + mins[i]);
  }

  // build base rows
  const rowsAll = raw.map(c => [+c[2], +c[3], +c[4], +c[5]]);

  // take first (1000 - 200) for base training
  const baseRows = rowsAll.slice(-800, -200);
  console.log(baseRows.length);
//   return;
  const { norm: baseNorm, mins: baseMins, maxs: baseMaxs } = safeNormalize(baseRows);

  const net = new LSTMTimeStep({ inputSize: 4, hiddenLayers: [8], outputSize: 4 });

  // --- safe initial training ---
  if (baseNorm.length > 2) {
    try {
      net.train(baseNorm, { iterations: 100, learningRate: 0.15 });
    } catch (e) {
      console.warn("⚠️ Initial training skipped:", e.message);
    }
  } else {
    console.warn("⚠️ Not enough base data to train.");
  }

  console.log(`\n📈 Simulation on last 200 candles:\n`);
  let equity = 1000;
  const riskFrac = 0.25; // 25% of equity risked per trade

  for (let i = 200; i > 0; i--) {
    const hist = rowsAll.slice(-(i + 14), -i);  // last 14 before the candle
    if (hist.length < 14) continue;

    // retrain lightly with last 14
    const { norm, mins, maxs } = safeNormalize(hist);
    if (norm.length > 2) {
      try {
        net.train(norm, { iterations: 120, learningRate: 0.01 });
      } catch (e) {
        console.warn(`⚠️ Skipped retrain at step ${200 - i + 1}:`, e.message);
        continue;
      }
    }

    // predict next close
    const [H, L, C, V] = safeDenorm(net.run(norm.slice(-1)), mins, maxs);

    const entry = hist[hist.length - 1][2]; // last close as entry
    const actual = rowsAll[rowsAll.length - i][2]; // actual close of candle
    const predDiff = ((C - entry) / entry) * 100;

  // decide buy/sell
const sli = 1;   // stop loss in % (e.g. 1 = 1%)
const tpi = 4;   // take profit in % (e.g. 2 = 2%)

let dir = "HOLD", sl, tp;
if (predDiff > atleast) {
  dir = "BUY";
  sl = entry * (1 - sli / 100);   // e.g. 1% below entry
  tp = entry * (1 + tpi / 100);   // e.g. 2% above entry
} else if (predDiff < -atleast) {
  dir = "SELL";
  sl = entry * (1 + sli / 100);   // e.g. 1% above entry
  tp = entry * (1 - tpi / 100);   // e.g. 2% below entry
}

// simulate outcome
let resultPct = 0;
if (dir === "BUY") {
  if (actual >= tp) resultPct = tpi;       // hit TP
  else if (actual <= sl) resultPct = -sli; // hit SL
  else resultPct = ((actual - entry) / entry) * 100;
} else if (dir === "SELL") {
  if (actual <= tp) resultPct = tpi;       // hit TP
  else if (actual >= sl) resultPct = -sli; // hit SL
  else resultPct = ((entry - actual) / entry) * 100;
}

// convert resultPct into equity change
if (dir !== "HOLD") {
  const riskAmt = equity * riskFrac;
  let pnl = 0;
  if (resultPct === -sli) {
    pnl = -riskAmt;
  } else if (resultPct === tpi) {
    // reward is proportional to TP/SL ratio
    pnl = riskAmt * (tpi / sli);
  } else {
    // scale partial outcome relative to SL
    pnl = riskAmt * (resultPct / sli);
  }
  equity += pnl;
}


    sumper += resultPct;

    console.log(
      `#${200 - i + 1} ${dir} entry:${entry.toFixed(2)} pred:${C.toFixed(2)} act:${actual.toFixed(2)} ` +
      `res:${resultPct.toFixed(2)}% sum:${sumper.toFixed(2)}% equity:${equity.toFixed(2)}`
    );
  }
const json = net.toJSON();
fs.writeFileSync("model.json", JSON.stringify(json));
  console.log(`\n💰 Final equity: ${equity.toFixed(2)}\n`);
}

async function simulate_v2verygood(raw) {
  let sumper = 0;

  // build base rows
  const rowsAll = raw.map(c => [+c[2], +c[3], +c[4], +c[5]]);

  // take first (1000 - 200) for base training
  const baseRows = rowsAll.slice(-800, -200);
  const { norm: baseNorm, mins, maxs } = normalize(baseRows);
  const net = new LSTMTimeStep({ inputSize: 4, hiddenLayers: [8], outputSize: 4 });
  net.train(baseNorm, { iterations: 600, learningRate: 0.1 }); // base training

  console.log(`\n📈 Simulation on last 200 candles:\n`);
  let equity = 100;
  const riskFrac = 0.25; // 50% of equity risked per trade

  for (let i = 200; i > 0; i--) {
    const hist = rowsAll.slice(-(i + 14), -i);  // last 14 before the candle
    if (hist.length < 14) continue;

    // retrain lightly with last 14
    const { norm, mins, maxs } = normalize(hist);
    net.train(norm, { iterations: 40 * 3, learningRate: 0.01 });

    // predict next close
    const [H, L, C, V] = denorm(net.run(norm.slice(-1)), mins, maxs);

    const entry = hist[hist.length - 1][2]; // last close as entry
    const actual = rowsAll[rowsAll.length - i][2]; // actual close of candle
    const predDiff = ((C - entry) / entry) * 100;

    // decide buy/sell
    let dir = "HOLD", sl, tp;
    if (predDiff > atleast) { dir = "BUY"; sl = entry * 0.98; tp = entry * 1.03; }
    else if (predDiff < -atleast) { dir = "SELL"; sl = entry * 1.02; tp = entry * 0.97; }

    // simulate outcome
    let resultPct = 0;
    if (dir === "BUY") {
      if (actual >= tp) resultPct = 3;
      else if (actual <= sl) resultPct = -2;
      else resultPct = ((actual - entry) / entry) * 100;
    } else if (dir === "SELL") {
      if (actual <= tp) resultPct = 3;
      else if (actual >= sl) resultPct = -2;
      else resultPct = ((entry - actual) / entry) * 100;
    }

    // convert resultPct into equity change
    if (dir !== "HOLD") {
      const riskAmt = equity * riskFrac;
      // stop loss = -2% → lose full riskAmt
      // take profit = +3% → gain 1.5 * riskAmt
      // else scale linearly
      let pnl = 0;
      if (resultPct === -2) pnl = -riskAmt;
      else if (resultPct === 3) pnl = riskAmt * 1.5;
      else pnl = riskAmt * (resultPct / 2); // scale relative to SL (-2%)
      equity += pnl;
    }

    sumper += resultPct;

    console.log(
      `#${200 - i + 1} ${dir} entry:${entry.toFixed(2)} pred:${C.toFixed(2)} act:${actual.toFixed(2)} ` +
      `res:${resultPct.toFixed(2)}% sum:${sumper.toFixed(2)}% equity:${equity.toFixed(2)}`
    );
  }

  console.log(`\n💰 Final equity: ${equity.toFixed(2)}\n`);
}

// ─── NEW: Simulation Function ──────────────────────────────
async function simulate_v1(raw) {
  let sumper = 0;

  // build base rows
  const rowsAll = raw.map(c => [+c[2], +c[3], +c[4], +c[5]]);

  // take first (1000 - 200) for base training
  const baseRows = rowsAll.slice(-800, -200);
  const { norm: baseNorm, mins, maxs } = normalize(baseRows);
  const net = new LSTMTimeStep({ inputSize: 4, hiddenLayers: [8], outputSize: 4 });
  net.train(baseNorm, { iterations: 600, learningRate: 0.1 }); // base training

  console.log(`\n📈 Simulation on last 200 candles:\n`);
  let equity=100;
  let risk=0.5;
  // now simulate one by one from -200 → -1
  for (let i = 200; i > 0; i--) {
    const hist = rowsAll.slice(-(i + 14), -i);  // last 14 before the candle
    if (hist.length < 14) continue;

    // retrain lightly with last 14
    const { norm, mins, maxs } = normalize(hist);
    net.train(norm, { iterations: 40*3, learningRate: 0.01 });

    // predict next close
    const [H, L, C, V] = denorm(net.run(norm.slice(-1)), mins, maxs);

    const entry = hist[hist.length - 1][2]; // last close as entry
    const actual = rowsAll[rowsAll.length - i][2]; // actual close of candle
    const predDiff = ((C - entry) / entry) * 100;

    // decide buy/sell
    let dir = "HOLD", sl, tp;
    if (predDiff > atleast) { dir = "BUY"; sl = entry * 0.98; tp = entry * 1.03; }
    else if (predDiff < -atleast) { dir = "SELL"; sl = entry * 1.02; tp = entry * 0.97; }

    // simulate outcome
    let result = 0;
    if (dir === "BUY") {
      if (actual >= tp) result = 3;
      else if (actual <= sl) result = -2;
      else result = ((actual - entry) / entry) * 100;
    } else if (dir === "SELL") {
      if (actual <= tp) result = 3;
      else if (actual >= sl) result = -2;
      else result = ((entry - actual) / entry) * 100;
    }

    sumper += result;

    console.log(`#${200 - i + 1} ${dir} entry:${entry.toFixed(2)} pred:${C.toFixed(2)} act:${actual.toFixed(2)} res:${result.toFixed(2)}% sum:${sumper.toFixed(2)}%`);
  }
}























// (async () => {
async function test(){
  for (let coin of symbols) {
    let raw = await fetchData(coin.pair);
console.log(raw.length);
await simulate(raw);

return;



	// raw = raw.slice(0,-1)
    if (lb > 0) { 
		const next = raw[raw.length - lb-0];
		nextc = +next[1]; 
		nexth = +next[2]; 
		nextl = +next[3]; 
		raw = raw.slice(0, -lb); 
	}

    const hist = raw.slice(0, -1), cur = raw[raw.length - 1];
    const rows = []; for (let c of hist) rows.push([+c[2], +c[3], +c[4], +c[5]]);

    const [ph, pl, pc] = predict(rows);
    const prevClose = +hist[hist.length - 1][4], curClose = +cur[4];
    const { s, diff, sl, tp } = signal(pc, prevClose), entry = prevClose;

    const maxWin = s === "buy" ? ((tp - entry) / entry) * 100 : s === "sell" ? ((entry - tp) / entry) * 100 : 0;
    const maxLoss = s === "buy" ? ((entry - sl) / entry) * 100 : s === "sell" ? ((sl - entry) / entry) * 100 : 0;
    const expRaw = s === "buy" ? ((pc - entry) / entry) * 100 : s === "sell" ? ((entry - pc) / entry) * 100 : 0;
    const expWin = Math.max(0, Math.min(expRaw, maxWin));

    portfolio.push({ ...coin, signal: s, diff, entry, sl, tp, maxWin, maxLoss, expWin, pc, ph, pl, prevClose, curClose });
  }

  let sum = 0, scores = [];
  for (let p of portfolio) {
    const sc = (p.signal !== "HOLD" && p.expWin > 0 && p.maxLoss > 0) ? p.expWin / (p.maxLoss + eps) : 0;
    scores.push(sc); sum += sc;
  }

  console.log("\nAllocation:");
  for (let i = 0; i < portfolio.length; i++) {
    const p = portfolio[i], alloc = sum ? (scores[i] / sum) * budget : 0;
	const pct = ((nextc - p.entry) / p.entry) * 100 * 1;
    // console.log(lb, p.name, p.pair, p.signal, p.entry, p.sl, p.tp, p.diff.toFixed(2), nextc,nexth,nextl, "alloc:", alloc.toFixed(2),+pct,p.ph,p.pl);
	console.log(`
📊 Trade Signal
---------------
Label:        ${lb}
Name:         ${p.name}
Pair:         ${p.pair}
Signal:       ${p.signal}

Entry:        ${p.entry}
Stop Loss:    ${p.sl}
Take Profit:  ${p.tp}

Next High:    ${nexth}
Next Candle:  ${nextc}
Next Low:     ${nextl}

Alloc:        ${alloc.toFixed(2)}
%:            ${pct}
Diff:         ${p.diff.toFixed(2)}

Predict High: ${p.ph}
Predict Close:${p.pc}
Predict Low:  ${p.pl}
`);

  }
}
// })();














// #region Logging Helpers

 
(async()=>{
	// await console.log(p.name,p.nc,p.signal,p.entry,p.sl,p.tp,p.sdiff.toFixed(2));
	// await getmarkets();
	// await open_order(p.nc,p.signal,p.entry,2.1,3.1,3);




///Whitebit
const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET;  
 
var markets=0;
async function getmarkets(){
if(markets==0){
	const marketsRes = await fetch('https://whitebit.com/api/v4/public/markets');
	markets = await marketsRes.json();
}
}

async function open_order(symbol, side, entry, slPerc, tpPerc, riskUSDT) {
	// await sleep(2000);
	// console.log("symb",symbol);return;
  // 1️⃣ Get market precisions
  const marketInfo = markets.find(m => m.name === symbol);
  if (!marketInfo) throw new Error(`Mercado ${symbol} não encontrado`);
console.log(marketInfo);
  const stockPrec = parseInt(marketInfo.stockPrec);
  const moneyPrec = parseInt(marketInfo.moneyPrec);

  // 2️⃣ Calculate SL and TP prices from percentages
  let sl, tp;
  if (side.toLowerCase() === 'buy') {
	sl = entry * (1 - slPerc / 100);
	tp = tpPerc ? entry * (1 + tpPerc / 100) : undefined;
  } else {
	sl = entry * (1 + slPerc / 100);
	tp = tpPerc ? entry * (1 - tpPerc / 100) : undefined;
  }

  // 3️⃣ Calculate qty based on max risk
  const priceDiff = Math.abs(entry - sl);
  if (priceDiff <= 0) throw new Error('Stop loss inválido para o lado da ordem');

  const qty = (riskUSDT / priceDiff).toFixed(stockPrec);
  const price = parseFloat(entry).toFixed(moneyPrec);

  // 4️⃣ Leverage calculations
  const leverageRisk = (entry / priceDiff).toFixed(2);
  const positionValue = entry * qty;
  const marginUsed = riskUSDT;
  const leverageExchange = (positionValue / marginUsed).toFixed(2);

  console.log("Leverage risco (stop):", leverageRisk);
  console.log("Leverage corretora:", leverageExchange);

  console.log(`📊 Qty: ${qty} | Entry: ${price} | SL: ${sl.toFixed(moneyPrec)} | TP: ${tp ? tp.toFixed(moneyPrec) : '—'} | Lev: ${leverageExchange}x`);

  // 5️⃣ Build order body
  const bodyObj = {
	request: '/api/v4/order/collateral/limit',
	nonce: Date.now(),
	market: symbol,
	side: side.toLowerCase(),
	amount: qty,
	price: price,
	stopLoss: parseFloat(sl).toFixed(moneyPrec),
	takeProfit: tp ? parseFloat(tp).toFixed(moneyPrec) : undefined
  };

  console.log(bodyObj);
  return;

  // 6️⃣ Sign & send (unchanged)
  const bodyStr = JSON.stringify(bodyObj);
  const payload = Buffer.from(bodyStr).toString('base64');
  const signature = crypto.createHmac('sha512', API_SECRET)
						  .update(payload)
						  .digest('hex');

  const res = await fetch('https://whitebit.com/api/v4/order/collateral/limit', {
	method: 'POST',
	headers: {
	  'Content-Type': 'application/json',
	  'X-TXC-APIKEY': API_KEY,
	  'X-TXC-PAYLOAD': payload,
	  'X-TXC-SIGNATURE': signature
	},
	body: bodyStr
  });

  console.log('Order response:', await res.json());
}

  

function sign(payload) {
  return crypto.createHmac('sha512', API_SECRET).update(payload).digest('hex');
}

async function getCollateralSummary() {
  const bodyObj = {
    request: '/api/v4/collateral-account/summary',
    nonce: Date.now().toString()
  };

  const bodyStr = JSON.stringify(bodyObj);
  const payload = Buffer.from(bodyStr).toString('base64');
  const signature = sign(payload);

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

  return res.json();
}

// const API = "https://whitebit.com/api/v4/public/ticker/ETH_USDT";

// async function ethBy25pct(total=100) {
//   let { last_price } = await (await fetch(API)).json();
//   return (total*0.25 / parseFloat(last_price)).toFixed(6);
// }

// ethBy25pct().then(console.log);
await getmarkets();
await open_order(symbols[0].nc,"buy",4480,1.0,4.0,28*0.25);

// Example usage
(async () => {
  try {
    const summary = await getCollateralSummary();
    console.log('[Collateral Summary]', summary);
  } catch (err) {
    console.error('[Error]', err.message);
  }
})();





// test();
})();











// });
// })();
