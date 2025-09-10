{
const https = require('https');
const crypto = require('crypto');

const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET; 

const bodyObj = {
  request: '/api/v4/main-account/balance',
  nonce: Date.now()
};

const bodyStr = JSON.stringify(bodyObj);
const payload = Buffer.from(bodyStr).toString('base64');
const signature = crypto.createHmac('sha512', API_SECRET).update(payload).digest('hex');

const options = {
  hostname: 'whitebit.com',
  path: '/api/v4/main-account/balance',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'X-TXC-APIKEY': API_KEY,
    'X-TXC-PAYLOAD': payload,
    'X-TXC-SIGNATURE': signature,
    'Content-Length': Buffer.byteLength(bodyStr)
  }
};

// const req = https.request(options, res => {
//   let data = '';
//   res.on('data', chunk => data += chunk);
//   res.on('end', () => {
//     try {
//       console.log('💰 Saldo:', JSON.parse(data));
//     } catch (err) {
//       console.error('Erro ao parsear resposta:', err.message, data);
//     }
//   });
// });

// req.on('error', err => console.error('Erro na requisição:', err.message));

// // Aqui enviamos o JSON cru, não o base64
// req.write(bodyStr);
// req.end();


function getOrderBook(market) {
  const url = `/api/v4/public/orderbook/${market}?limit=1`;

  const options = {
    hostname: 'whitebit.com',
    path: url,
    method: 'GET'
  };

  const req = https.request(options, res => {
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
      try {
		const json = JSON.parse(data);
		// console.log(json);

		const bestAsk = json.asks[0]; // [price, quantity]
		const bestBid = json.bids[0]; // [price, quantity]

		const askPrice = parseFloat(bestAsk[0]);
		const bidPrice = parseFloat(bestBid[0]);

		// % difference relative to ask
		const spreadPct = ((askPrice - bidPrice) / askPrice) * 100;

		console.log(`📈  ${json.ticker_id} Best Ask: ${askPrice} (${bestAsk[1]} )`);
		console.log(`📉  ${json.ticker_id} Best Bid: ${bidPrice} (${bestBid[1]} )`);
		console.log(`📊  ${json.ticker_id} Spread: ${spreadPct.toFixed(5)}%`);
      } catch (err) {
        console.error('Erro ao parsear resposta:', err.message);
      }
    });
  });

  req.on('error', err => console.error('Erro na requisição:', err.message));
  req.end();
}

getOrderBook('BTC_PERP');
// getOrderBook('BTC_USDT');
// getOrderBook('ETH_USDT');
getOrderBook('ETH_PERP');
// getOrderBook('SOL_USDT');
getOrderBook('DOGE_PERP');
getOrderBook('XRP_PERP');
getOrderBook('SOL_PERP');






function getKrakenFuturesBidAsk(symbol = 'pi_xbtusd') {
  const path = `/derivatives/api/v3/orderbook?symbol=${symbol}`;

  https.get({ hostname: 'futures.kraken.com', path, method: 'GET' }, res => {
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
      try {
        const json = JSON.parse(data);
        const ob = json.orderBook;
        if (!ob || !ob.asks?.length || !ob.bids?.length) {
          console.error('Livro de ordens vazio ou formato inesperado:', json);
          return;
        }
        const [askPrice, askQty] = ob.asks[0];
        const [bidPrice, bidQty] = ob.bids[0];
        console.log(`📈 KBest Ask: ${askPrice} (${askQty})`);
        console.log(`📉 KBest Bid: ${bidPrice} (${bidQty})`);
      } catch (err) {
        console.error('Erro ao parsear resposta:', err.message, data);
      }
    });
  }).on('error', err => console.error('Erro na requisição:', err.message));
}
// getKrakenFuturesBidAsk();

	return;
}


{
	// Retrieve cash and margin wallet details.


const { createHash, createHmac } = require("crypto")
const { URLSearchParams } = require("url")

const API_KEY = process.env.KRAKEN_FUTURES_KEY;
const API_SECRET = process.env.KRAKEN_FUTURES_SECRET; 
function main() {
   request({
      method: "GET",
      path: "/derivatives/api/v3/accounts",
      publicKey: API_KEY,
      privateKey: API_SECRET,
      environment: "https://futures.kraken.com"
   })
   .then((resp) => resp.text())
   .then(console.log)
}

function request({method = "GET", path = "", query = {}, body = {}, nonce = "", publicKey = "", privateKey = "", environment = ""}) {
   let url = environment + path
   let queryString = ""
   if (Object.keys(query).length > 0) {
      queryString = mapToURLValues(query).toString()
      url += "?" + queryString
   }
   let bodyString = null
   if (Object.keys(body).length > 0) {
      bodyString = mapToURLValues(body).toString()
   }
   const headers = {}
   if (publicKey.length > 0) {
      headers["APIKey"] = publicKey
      headers["Authent"] = getSignature(privateKey, queryString + (bodyString ?? ""), nonce, path)
      if (nonce.length > 0) {
         headers["Nonce"] = nonce
      }
   }
   return fetch(url, { method: method, headers: headers, body: bodyString })
}

function getSignature(privateKey = "", data = "", nonce = "", path = "") {
   return sign({
      privateKey: privateKey,
      message: createHash("sha256")
      .update(data + nonce + path.replace("/derivatives", ""))
      .digest("binary")
   })
}

function sign({privateKey = "", message = ""}) {
   return createHmac(
      "sha512",
      Buffer.from(privateKey, "base64")
   )
   .update(message, "binary")
   .digest("base64")
}

function mapToURLValues(object) {
   return new URLSearchParams(Object.entries(object).map(([k, v]) => {
      if (typeof v == 'object') {
         v = JSON.stringify(v)
      }
      return [k, v]
   }))
}

main();
return;
}




{
// const crypto = require("crypto");
// const fetch = require("node-fetch");

const API_KEY = process.env.KRAKEN_FUTURES_KEY;
const API_SECRET = process.env.KRAKEN_FUTURES_SECRET; 
// Get all asset balances.

const { createHash, createHmac } = require("crypto")

function main() {
   request({
      method: "POST",
      path: "/0/private/Balance",
      publicKey: API_KEY,
      privateKey: API_SECRET,
      environment: "https://api.kraken.com",
   })
   .then((resp) => resp.text())
   .then(console.log)
}

function request({method = "GET", path = "", query = {}, body = {}, publicKey = "", privateKey = "", environment = ""}) {
   let url = environment + path
   let queryString = ""
   if (Object.keys(query).length > 0) {
      queryString = mapToURLValues(query).toString()
      url += "?" + queryString
   }
   let nonce = ""
   if (publicKey.length > 0) {
      nonce = body["nonce"]
      if (!nonce) {
         nonce = getNonce()
         body["nonce"] = nonce
      }
   }
   const headers = {}
   let bodyString = null
   if (Object.keys(body).length > 0) {
      bodyString = JSON.stringify(body)
      headers["Content-Type"] = "application/json"
   }
   if (publicKey.length > 0) {
      headers["API-Key"] = publicKey
      headers["API-Sign"] = getSignature(privateKey, queryString + (bodyString || ""), nonce, path)
   }
   return fetch(url, { method: method, headers: headers, body: bodyString })
}

function getNonce() {
   return Date.now().toString()
}

function getSignature(privateKey = "", data = "", nonce = "", path = "") {
   return sign({
      privateKey: privateKey,
      message: path + createHash("sha256").update(nonce + data).digest("binary")
   })
}

function sign({privateKey = "", message = ""}) {
   return createHmac(
      "sha512",
      Buffer.from(privateKey, "base64")
   )
   .update(message, "binary")
   .digest("base64")
}

function mapToURLValues(object) {
   return new URLSearchParams(Object.entries(object).map(([k, v]) => {
      if (typeof v == 'object') {
         v = JSON.stringify(v)
      }
      return [k, v]
   }))
}

main()
return;
}


























const inicio = Date.now();

// aiscript.js
// import crypto from 'crypto';

const https = require('https');
const crypto = require('crypto');
const fs = require('fs');
const vm = require('vm');


const WebSocket = require('ws');
// const crypto = require('crypto');

// import fetch from 'node-fetch'; // Node 20 has global fetch, but this works for older versions too

// Carrega brain.min.js (tem de estar na mesma pasta)
const brainPath = __dirname + '/brain.min.js';
const brainCode = fs.readFileSync(brainPath, 'utf8');
vm.runInThisContext(brainCode);

// Configurações
const symbols_ = [
  { name: 'Bitcoin', pair: 'BTCUSDC' },
  { name: 'Ethereum', pair: 'ETHUSDT' },
  { name: 'BNB', pair: 'BNBUSDT' },
  { name: 'XRP', pair: 'XRPUSDT' },
  { name: 'Solana', pair: 'SOLUSDT' }
];

const interval = '1d';
const historyLength = 7;
const RERUNS = 8 * 3;
const TRAIN_ITER = 40;

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


// async function kfetchData(symbol) {
//   const url = `https://demo-futures.kraken.com/trade`;
// //   const url = `https://futures.kraken.com/trade`;
// //   const url = `https://api.binance.com/`; 
//   console.log("url",url);
//   const res = await fetch(url);
//   return res;//.json();
// }
// const kdata = await kfetchData('BTCUSDT');
// console.log("📊 Conteúdo recebido:", kdata);



// return;


// const WebSocket = require('ws');
// const crypto = require('crypto');

const API_KEY = process.env.KRAKEN_FUTURES_KEY;
const API_SECRET = process.env.KRAKEN_FUTURES_SECRET; // Base64
// console.log(API_KEY);
// const WS_URL = 'wss://demo-futures.kraken.com/ws/v1';

{
// Get all asset balances.

const { createHash, createHmac } = require("crypto")

function main() {
   request({
      method: "POST",
      path: "/0/private/Balance",
      publicKey: API_KEY,
      privateKey: API_SECRET,
      environment: "https://api.kraken.com",
   })
   .then((resp) => resp.text())
   .then(console.log)
}

function request({method = "GET", path = "", query = {}, body = {}, publicKey = "", privateKey = "", environment = ""}) {
   let url = environment + path
   let queryString = ""
   if (Object.keys(query).length > 0) {
      queryString = mapToURLValues(query).toString()
      url += "?" + queryString
   }
   let nonce = ""
   if (publicKey.length > 0) {
      nonce = body["nonce"]
      if (!nonce) {
         nonce = getNonce()
         body["nonce"] = nonce
      }
   }
   const headers = {}
   let bodyString = null
   if (Object.keys(body).length > 0) {
      bodyString = JSON.stringify(body)
      headers["Content-Type"] = "application/json"
   }
   if (publicKey.length > 0) {
      headers["API-Key"] = publicKey
      headers["API-Sign"] = getSignature(privateKey, queryString + (bodyString || ""), nonce, path)
   }
   return fetch(url, { method: method, headers: headers, body: bodyString })
}

function getNonce() {
   return Date.now().toString()
}

function getSignature(privateKey = "", data = "", nonce = "", path = "") {
   return sign({
      privateKey: privateKey,
      message: path + createHash("sha256").update(nonce + data).digest("binary")
   })
}

function sign({privateKey = "", message = ""}) {
   return createHmac(
      "sha512",
      Buffer.from(privateKey, "base64")
   )
   .update(message, "binary")
   .digest("base64")
}

function mapToURLValues(object) {
   return new URLSearchParams(Object.entries(object).map(([k, v]) => {
      if (typeof v == 'object') {
         v = JSON.stringify(v)
      }
      return [k, v]
   }))
}

main()
return;
}

















/**
 * Gera assinatura HMAC exigida pela Kraken Futures WS API
 * @param {number} nonce - Número único (timestamp em microssegundos)
 * @returns {string} Assinatura base64
 */
function gerarAssinatura(nonce) {
  return crypto
    .createHmac('sha256', Buffer.from(API_SECRET, 'base64'))
    .update(nonce.toString())
    .digest('base64');
}

/**
 * Cria payload de subscrição autenticada
 * @param {string} feed - Nome do feed privado (ex: 'open_orders')
 * @returns {object} Payload pronto para enviar
 */
function criarPayloadAutenticado(feed) {
//   const nonce = Date.now() * 1000; // microssegundos
  const nonce = (Date.now() * 1000).toString();
  return {
    event: 'subscribe',
    feed,
    api_key: API_KEY,
    nonce,
    auth_signature: gerarAssinatura(nonce)
  };
}

/**
 * Subscreve a um feed privado e resolve com a primeira mensagem recebida
 * @param {WebSocket} ws - Conexão WebSocket já aberta
 * @param {string} feed - Nome do feed privado
 * @returns {Promise<object>} Primeira mensagem recebida do feed
 */
function subscreverFeedPrivado(ws, feed) {
  return new Promise((resolve) => {
    const payload = criarPayloadAutenticado(feed);
    ws.send(JSON.stringify(payload));

    ws.on('message', (msg) => {
      const data = JSON.parse(msg);
      if (data.feed === feed) {
        resolve(data);
      }
    });
  });
}

/**
 * Conecta ao WS, obtém open orders e saldo/margem, e fecha
 */
async function obterOrdensESaldo() {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(WS_URL);

    ws.on('open', async () => {
      try {
        console.log('Conectado ao WebSocket Demo Kraken Futures');

        const [ordens, saldo] = await Promise.all([
          subscreverFeedPrivado(ws, 'open_orders'),
          subscreverFeedPrivado(ws, 'account_balances_and_margins')
        ]);

        console.log('\n📋 Ordens abertas:');
        console.log(JSON.stringify(ordens, null, 2));

        console.log('\n💰 Saldo e margens:');
        console.log(JSON.stringify(saldo, null, 2));

        ws.close();
        resolve({ ordens, saldo });
      } catch (err) {
        reject(err);
      }
    });

    ws.on('error', reject);
  });
}

// Executa
(async () => {
  try {
    await obterOrdensESaldo();
  } catch (err) {
    console.error('Erro:', err.message);
  }
})();



return;














// const https = require('https');

// function get(path) {
//   return new Promise((resolve, reject) => {
//     const req = https.request({
//       hostname: 'demo-futures.kraken.com',
//       port: 443,
//       path,
//       method: 'GET',
//       headers: {
//         'User-Agent': 'kraken-futures-demo-test',
//         'Accept': 'application/json'
//       }
//     }, (res) => {
//       let data = '';
//       res.on('data', c => data += c);
//       res.on('end', () => resolve({ statusCode: res.statusCode, data }));
//     });
//     req.on('error', reject);
//     req.end();
//   });
// }

// (async () => {
//   const r = await get('/derivatives/api/v3/tickers');
//   console.log('GET /derivatives/api/v3/tickers ->', r.statusCode);
//   console.log(r.data);
// })();





// kraken-futures-orders.js
// const https = require('https');
// const crypto = require('crypto');

// const apiKey = process.env.KRAKEN_FUTURES_KEY;
// const apiSecret = process.env.KRAKEN_FUTURES_SECRET; // base64 encoded

// function krakenFuturesRequest(path, body = {}) {
//   return new Promise((resolve, reject) => {
//     const nonce = Date.now().toString();
//     const postData = JSON.stringify(body);

//     // Create signature
//     const message = path + nonce + postData;
//     const hmac = crypto.createHmac('sha256', Buffer.from(apiSecret, 'base64'))
//                        .update(message)
//                        .digest('base64');

//     const options = {
//       hostname: 'demo-futures.kraken.com',
//       port: 443,
//       path,
//       method: 'POST',
//       headers: {
//         'APIKey': apiKey,
//         'Authent': hmac,
//         'Nonce': nonce,
//         'Content-Type': 'application/json',
//         'Content-Length': Buffer.byteLength(postData)
//       }
//     };

//     const req = https.request(options, (res) => {
//       let data = '';
//       res.on('data', chunk => data += chunk);
//       res.on('end', () => {
//         try {
//           resolve(JSON.parse(data));
//         } catch (err) {
//           reject(err);
//         }
//       });
//     });

//     req.on('error', reject);
//     req.write(postData);
//     req.end();
//   });
// }

// (async () => {
//   try {
//     const result = await krakenFuturesRequest('/derivatives/api/v3/openorders');
//     console.log(JSON.stringify(result, null, 2));
//   } catch (err) {
//     console.error('Error:', err.message);
//   }
// })();



// const https = require('https');
// const crypto = require('crypto');

// const API_KEY = process.env.KRAKEN_FUTURES_KEY;
// const API_SECRET = process.env.KRAKEN_FUTURES_SECRET; // Base64 encoded
const API_HOST = 'demo-futures.kraken.com';

/**
 * Gera a assinatura HMAC exigida pela Kraken Futures API
 * @param {string} path - Caminho do endpoint (ex: /derivatives/api/v3/openorders)
 * @param {string} nonce - Número único crescente (timestamp em ms)
 * @param {string} postData - Corpo da requisição em JSON string
 * @returns {string} Assinatura base64
 */
function gerarAssinatura(path, nonce, postData) {
  return crypto
    .createHmac('sha256', Buffer.from(API_SECRET, 'base64'))
    .update(path + nonce + postData)
    .digest('base64');
}

/**
 * Faz uma requisição autenticada à Kraken Futures API
 * @param {string} path - Caminho do endpoint
 * @param {object} body - Objeto com os parâmetros da requisição
 * @returns {Promise<object>} Resposta da API em JSON
 */
function krakenFuturesRequest(path, body = {}) {
  return new Promise((resolve, reject) => {
    const nonce = Date.now().toString();
    const payload = { nonce, ...body };
    const postData = JSON.stringify(payload);
    const assinatura = gerarAssinatura(path, nonce, postData);

    const options = {
      hostname: API_HOST,
      port: 443,
      path,
      method: 'POST',
      headers: {
        'APIKey': API_KEY,
        'Authent': assinatura,
        'Nonce': nonce,
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(postData)
      }
    };

    const req = https.request(options, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => {
        try {
          resolve(JSON.parse(data));
        } catch (err) {
          reject(new Error(`Erro ao parsear JSON: ${err.message}`));
        }
      });
    });

    req.on('error', reject);
    req.write(postData);
    req.end();
  });
}

/**
 * Exemplo: Lista ordens abertas na conta Demo
 */
async function listarOrdensAbertas() {
  try {
    const path = '/derivatives/api/v3/openorders';
    const resposta = await krakenFuturesRequest(path);
    console.log('Ordens abertas:', JSON.stringify(resposta, null, 2));
  } catch (err) {
    console.error('Erro ao listar ordens abertas:', err.message);
  }
}

// Executa o exemplo
await listarOrdensAbertas();

// async function consultarConta() {
//   try {
//     const path = '/derivatives/api/v3/accounts';
//     const resposta = await krakenFuturesRequest(path);
    
//     if (resposta && resposta.result === 'success') {
//       console.log('Informações da conta:');
//       console.log(JSON.stringify(resposta, null, 2));
//     } else {
//       console.error('Erro na resposta da API:', resposta);
//     }
//   } catch (err) {
//     console.error('Erro ao consultar conta:', err.message);
//   }
// }

// await consultarConta();




return;

  function formatNum(num, decimals = 2) {
    return parseFloat(num).toLocaleString(undefined, { minimumFractionDigits: decimals, maximumFractionDigits: decimals });
  }






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



	//close inverted /////////////////////////////////////////////////////////








	// 2. Get account info
	const accountRes = await fetch(`https://api.mexc.com/api/v3/account?${query}&signature=${sig}`, {
		headers: { 'X-MEXC-APIKEY': key }
	});
	const account = await accountRes.json();


    const asset = "BNB";
	    const free = parseFloat(account.balances.find(b => b.asset === asset)?.free || 0);
    if (free <= 0) {
      console.error(`❌ Saldo insuficiente para vender ${asset}`);
      return;
    }

	return;

	// 3. Calculate total equity
	let equity = null;
	if (Array.isArray(account.balances)) {
		equity = account.balances.reduce((sum, b) =>
		sum + parseFloat(b.free) + parseFloat(b.locked), 0
		);
	}

	console.log(account);

	// 4. Output JSON
	console.log(JSON.stringify({
		openOrders: orders,
		totalEquity: equity
	}, null, 2));



	var budget=equity;
	const eps = 1e-9;
    const scores = results.map(p => {
      if ((p.signal==='BUY'||p.signal==='SELL') && p.expectedWinPct>0 && p.maxLossPct>0) {
        return p.expectedWinPct/(p.maxLossPct+eps);
      }
      return 0;
    });
    const scoreSum = scores.reduce((a,b)=>a+b,0);
    const amounts = results.map((p,i)=>scoreSum<=0?0:(scores[i]/scoreSum)*budget);


// results.forEach((p,i)=>{ 
// 	// let 
// 	console.log(p.name,p.signal, formatNum(amounts[i]) );
//     });



const API_BASE = 'https://api.mexc.com/api/v3';
const feeMap = {};
// (async () => {
async function getfee(){
  // Get account trade fee info (public endpoint for all symbols)
  const feeRes = await fetch(`${API_BASE}/exchangeInfo`);
  const feeData = await feeRes.json();
//   const feeMap = {};
  feeData.symbols.forEach(s => {
    feeMap[s.symbol] = {
      maker: s.makerCommission || 0.002, // fallback 0.2%
      taker: s.takerCommission || 0.002
    };
  });

  for (const { name, pair } of symbols_) {
    const res = await fetch(`${API_BASE}/ticker/bookTicker?symbol=${pair}`);
    const { bidPrice, askPrice } = await res.json();
    const { maker, taker } = feeMap[pair] || {};

    console.log(`${name} (${pair}) → Bid: ${bidPrice} | Ask: ${askPrice} | Maker fee: ${maker} | Taker fee: ${taker}`);
  }
};//)();

await getfee();

// return;

//   // Get account trade fee info (public endpoint for all symbols)
//   const feeRes = await fetch(`${API_BASE}/exchangeInfo`);
//   const feeData = await feeRes.json();
//   const feeMap = {};
//   feeData.symbols.forEach(s => {
//     feeMap[s.symbol] = {
//       maker: s.makerCommission || 0.002, // fallback 0.2%
//       taker: s.takerCommission || 0.002
//     };
//   });

// for (let i = 0; i < results.length; i++) {
//   const p = results[i];
//  const { maker, taker } = feeMap[p.symbol];
//  console.log(p.symbol,maker,taker);
// }
// return;









  // Helper p/ criar ordens assinadas
  async function sendOrder(paramsObj) {
    const params = new URLSearchParams({ ...paramsObj, timestamp: Date.now().toString() });
    const signature = crypto.createHmac('sha256', secret).update(params.toString()).digest('hex');
    const res = await fetch(`${API_BASE}/order?${params}&signature=${signature}`, {
      method: 'POST',
      headers: { 'X-MEXC-APIKEY': key }
    });
    return res.json();
  }


const info = await fetch('https://api.mexc.com/api/v3/exchangeInfo').then(r => r.json());
const sol = info.symbols.find(s => s.symbol === 'SOLUSDT');
console.log(sol.quantityPrecision, sol.filters,sol.baseAssetPrecision);

// return;

// maker/taker fee padrão MEXC Spot = 0.2% (0.002)
const MAKER_FEE = 0.002;
const TAKER_FEE = 0.002;

async function open_order(symbol, side, entry, sl, tp, amountUSDT) {
  // 1️⃣ Preço atual
  const ticker = await fetch(`${API_BASE}/ticker/bookTicker?symbol=${symbol}`).then(r => r.json());
  const _price = parseFloat(side === 'BUY' ? ticker.askPrice : ticker.bidPrice);

  // 2️⃣ Quantidade ajustada à comissão
  const { maker, taker } = feeMap[symbol] || { maker: MAKER_FEE, taker: TAKER_FEE };
  const feeRate = side === 'BUY' ? taker : maker;

  const sol = info.symbols.find(s => s.symbol === symbol);
  const qty = (amountUSDT * (1.0 - feeRate) / _price).toFixed(sol.baseAssetPrecision);

//   console.log( qty,feeRate);
//   console.log(price, qty,feeRate,amountUSDT,Number(amountUSDT)/Number(price));

const amor={
    symbol, side,
    type: 'MARKET',
    price: _price,
    quantity: qty,
    timeInForce: 'GTC'
  };
  // 3️⃣ Ordem principal (entrada)
  const mainOrder = await sendOrder(amor);
  console.log('Ordem principal enviada:', amor,mainOrder);

  return;
  // 4️⃣ Stop Loss
  if (sl) {
    const slOrder = await sendOrder({
      symbol,
      side: side === 'BUY' ? 'SELL' : 'BUY', // inverso p/ fechar posição
      type: 'STOP_LIMIT',
      stopPrice: sl.toString(),
      price: sl.toString(),
      quantity: qty,
      timeInForce: 'GTC'
    });
    console.log('Stop Loss enviado:', slOrder);
  }

  // 5️⃣ Take Profit
  if (tp) {
    const tpOrder = await sendOrder({
      symbol,
      side: side === 'BUY' ? 'SELL' : 'BUY', // inverso p/ fechar posição
      type: 'LIMIT',
      price: tp.toString(),
      quantity: qty,
      timeInForce: 'GTC'
    });
    console.log('Take Profit enviado:', tpOrder);
  }
}


// Exemplo de uso:
// // open_order('ETHUSDT', 'BUY', 2300, 2200, 2500, 36);
// for(let i=0;i<symbols_.length;i++){
// 	formatNum(amounts[i]);
// // console.log('ord', 'ETHUSDT', 'BUY', 2300, 2200, 2500, 36);
// }


// const info = await fetch(`${API_BASE}/selfSymbols`).then(r => r.json());
// const validSymbols = info.symbols.map(s => s.symbol);
// console.log(info);
 


for (let i = 0; i < results.length; i++) {
  const p = results[i];
  if (p.signal === "HOLD"){ 
	console.log(p.pair,"hold");
	continue;
  }
	
// if (!validSymbols.includes(p.pair)) {
//   console.error(`Símbolo ${p.pair} não suportado pela API`);
// //   return;
// }


//   continue;
  const amt = formatNum(amounts[i]); // valor em USDT
  await open_order(
    p.pair,        // par ex: 'ETHUSDT'
    p.signal,      // 'BUY' ou 'SELL'
    p.entry,       // preço de entrada
    p.stopLoss,    // stop loss
    p.takeProfit,  // take profit
    amounts[i]            // valor em USDT
  );

  console.log(p.pair, p.signal, p.entry, p.stopLoss, p.takeProfit, amt);
}



	




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
