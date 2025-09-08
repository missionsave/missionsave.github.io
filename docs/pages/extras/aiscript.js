const inicio = Date.now();

// aiscript.js
import crypto from 'crypto';
// import fetch from 'node-fetch'; // Node 20 has global fetch, but this works for older versions too

const fs = require('fs');
const vm = require('vm');

// Carrega brain.min.js (tem de estar na mesma pasta)
const brainPath = __dirname + '/brain.min.js';
const brainCode = fs.readFileSync(brainPath, 'utf8');
vm.runInThisContext(brainCode);

// Configurações
const symbols_ = [
  { name: 'Bitcoin', pair: 'BTCUSDT' },
  { name: 'Ethereum', pair: 'ETHUSDT' },
  { name: 'BNB', pair: 'BNBUSDT' },
  { name: 'XRP', pair: 'XRPUSDT' },
  { name: 'Solana', pair: 'SOLUSDT' }
];

const interval = '1d';
const historyLength = 7;
const RERUNS = 8 * 3;
const TRAIN_ITER = 400;

// Funções utilitárias
async function fetchData(symbol) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${historyLength}`;
  const res = await fetch(url);
  return res.json();
}

function normalizePerFeature(data) {
  const features = data[0].length;
  const mins = Array(features).fill(Infinity);
  const maxs = Array(features).fill(-Infinity);
  data.forEach(row => row.forEach((v, i) => {
    if (v < mins[i]) mins[i] = v;
    if (v > maxs[i]) maxs[i] = v;
  }));
  const normalized = data.map(row => row.map((v, i) => (v - mins[i]) / (maxs[i] - mins[i] || 1)));
  return { normalized, mins, maxs };
}

function denormalizeRow(row, mins, maxs) {
  return row.map((v, i) => v * (maxs[i] - mins[i]) + mins[i]);
}

function trainOnce(rows) {
  const { normalized, mins, maxs } = normalizePerFeature(rows);
  const net = new brain.recurrent.LSTMTimeStep({
    inputSize: 4,
    hiddenLayers: [16],
    outputSize: 4
  });
  net.train(normalized, { iterations: TRAIN_ITER, learningRate: 0.01 });
  const lastSeq = normalized.slice(-1);
  const predictionNorm = net.run(lastSeq);
  return denormalizeRow(predictionNorm, mins, maxs);
}

function median(arr) {
  const s = [...arr].sort((a, b) => a - b);
  const m = Math.floor(s.length / 2);
  return s.length % 2 ? s[m] : (s[m - 1] + s[m]) / 2;
}

function trainAndPredictMedian(rows, runs = RERUNS) {
  const highs = [], lows = [], closes = [], vols = [];
  for (let i = 0; i < runs; i++) {
    const p = trainOnce(rows);
    highs.push(p[0]); lows.push(p[1]); closes.push(p[2]); vols.push(p[3]);
  }
  return [median(highs), median(lows), median(closes), median(vols)];
}

function generateSignal(predClose, prevClose) {
  const diffPct = ((predClose - prevClose) / prevClose) * 100;
  let signal, entry, sl, tp;
  if (diffPct > 0.5) {
    signal = 'BUY'; entry = prevClose;
    sl = prevClose * 0.98; tp = prevClose * 1.03;
  } else if (diffPct < -0.5) {
    signal = 'SELL'; entry = prevClose;
    sl = prevClose * 1.02; tp = prevClose * 0.97;
  } else {
    signal = 'HOLD'; entry = prevClose; sl = null; tp = null;
  }
  return { signal, diffPct, entry, sl, tp };
}

// Script principal
(async () => {
  console.log("🚀 Início do cálculo de sinais...");
  const results = [];

  for (const coin of symbols_) {
    const rawData = await fetchData(coin.pair);
    const historicalData = rawData.slice(0, -1);
    const currentCandle = rawData[rawData.length - 1];

    const rows = historicalData.map(c => [
      parseFloat(c[2]), parseFloat(c[3]), parseFloat(c[4]), parseFloat(c[5])
    ]);

    const [predHigh, predLow, predClose] = trainAndPredictMedian(rows, RERUNS);
    const prevClose = parseFloat(historicalData[historicalData.length - 1][4]);

    const { signal, diffPct, entry, sl, tp } = generateSignal(predClose, prevClose);

    const maxWinPct = signal === 'BUY'
      ? ((tp - entry) / entry) * 100
      : signal === 'SELL' ? ((entry - tp) / entry) * 100 : 0;

    const maxLossPct = signal === 'BUY'
      ? ((entry - sl) / entry) * 100
      : signal === 'SELL' ? ((sl - entry) / entry) * 100 : 0;

    const expectedRaw = signal === 'BUY'
      ? ((predClose - entry) / entry) * 100
      : signal === 'SELL' ? ((entry - predClose) / entry) * 100 : 0;

    const expectedWinPct = Math.max(0, Math.min(expectedRaw, maxWinPct));

    const prevDate = new Date(historicalData[historicalData.length - 1][0]).toISOString().split("T")[0];

    results.push({
      name: coin.name,
      pair: coin.pair,
      prevDate,
      signal,
      prevClose,
      predClose,
      diffPct,
      entry,
      stopLoss: sl,
      takeProfit: tp,
      maxWinPct,
      maxLossPct,
      expectedWinPct
    });
  }

  console.log("📊 Sinais calculados:", results);
  // Aqui podes usar `results` para qualquer outra ação no workflow






const key = process.env.MEXC_KEY;
const secret = process.env.MEXC_SECRET;

if (!key || !secret) {
  console.error('Missing MEXC_KEY or MEXC_SECRET in environment variables');
  process.exit(1);
}

// Helper to sign query
function sign(query, secret) {
  return crypto.createHmac('sha256', secret).update(query).digest('hex');
}

(async () => {
  const ts = Date.now();
  const query = `timestamp=${ts}`;
  const sig = sign(query, secret);

  // 1. Get open orders
  const ordersRes = await fetch(`https://api.mexc.com/api/v3/openOrders?${query}&signature=${sig}`, {
    headers: { 'X-MEXC-APIKEY': key }
  });
  const orders = await ordersRes.json();

  // 2. Get account info
  const accountRes = await fetch(`https://api.mexc.com/api/v3/account?${query}&signature=${sig}`, {
    headers: { 'X-MEXC-APIKEY': key }
  });
  const account = await accountRes.json();

  // 3. Calculate total equity
  let equity = null;
  if (Array.isArray(account.balances)) {
    equity = account.balances.reduce((sum, b) =>
      sum + parseFloat(b.free) + parseFloat(b.locked), 0
    );
  }

  // 4. Output JSON
  console.log(JSON.stringify({
    openOrders: orders,
    totalEquity: equity
  }, null, 2));
})();














  const fim = Date.now();
  console.log(`⏱ Tempo total de execução: ${(fim - inicio) / 1000} segundos`);
  console.log("✅ Script concluído com sucesso");
})();




// // Carrega brain.min.js
// // const brainPath = __dirname + '/brain.min.js';
// // const stats = fs.statSync(brainPath);
// // console.log(`📦 brain.min.js encontrado com ${stats.size} bytes`);
// // const brainCode = fs.readFileSync(brainPath, 'utf8');
// // vm.runInThisContext(brainCode);

// // Configurações para Binance
// // const interval = '1d';
// // const historyLength = 6;

// async function fetchData(symbol) {
//   const url = `https://api.mexc.com/api/v3/account`;
// //   const url = `https://api.binance.com/`;
// //   const url = `https://data-api.binance.vision/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${historyLength}`; //works
// //   const url = `https://api.binance.com/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${historyLength}`;
//   console.log("url",url);
//   const res = await fetch(url);
//   return res.json();
// }

// (async () => {
//   const agora = new Date();
//   console.log(`🕒 Script iniciado em: ${agora.toLocaleString('pt-PT', { timeZone: 'Europe/Lisbon' })}`);

//   // Teste de fetch
//   const data = await fetchData('BTCUSDT');
//   console.log(`📊 Recebidas4 ${data.length} velas de BTCUSDT`);
// //   const data = await fetchData('BTCUSDT');
// console.log("📊 Tipo de dados recebidos:", Array.isArray(data) ? 'Array' : typeof data);
// console.log("📊 Conteúdo recebido:", data);
// console.log(`📊 Recebidas ${Array.isArray(data) ? data.length : 0} velas de BTCUSDT`);


//   // Teste com Brain.js
//   const net = new brain.NeuralNetwork();
//   net.train([
//     { input: [0, 0], output: [0] },
//     { input: [0, 1], output: [1] },
//     { input: [1, 0], output: [1] },
//     { input: [1, 1], output: [0] }
//   ]);

//   const resultado = net.run([1, 0]);
//   console.log("🧠 Resultado do teste:", resultado);

//   const fim = Date.now();
//   console.log(`⏱ Tempo total de execução: ${(fim - inicio) / 1000} segundos`);
//   console.log("✅ Script concluído com sucesso");
// })();
