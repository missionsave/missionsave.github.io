const fs   = require('fs');
const vm   = require('vm');
const path = require('path');
const crypto = require('crypto');

//Load brain.js into this context
const brainPath = path.join(__dirname, 'brain.min.js');
const brainCode = fs.readFileSync(brainPath, 'utf8');
vm.runInThisContext(brainCode); // exposes global `brain`

// If using Node <18, uncomment the next line:
// const fetch = require('node-fetch');

var markets=0;
var tries=0;
var lb=26;////////////
// const hsize=90;
// const hsize=19;
const hsize=9;

///Whitebit
const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET; 


async function cotm(...params) {
	await console.log(...params);	
}
const sleepms = ms => new Promise(resolve => setTimeout(resolve, ms));

async function fetchCandles(symbol, limit = 101, interval = '1d') {
  const url = `https://whitebit.com/api/v1/public/kline?market=${symbol}&interval=${interval}&limit=${limit}`;
  
  const res = await fetch(url);
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  
  const json = await res.json();
  if (!json.result || !Array.isArray(json.result)) {
    throw new Error('Unexpected response format');
  }

  // Prepare arrays
  const vclose = [];
  const vhigh  = [];
  const vlow   = [];

  for (const c of json.result) {
    vclose.push(parseFloat(c[2])); // close
    vhigh.push(parseFloat(c[3]));  // high
    vlow.push(parseFloat(c[4]));   // low
  }

  // Last candle date
  const lastCandle = json.result[json.result.length - 1];
  const lastDate = new Date(lastCandle[0] * 1000);

  return { vclose, vhigh, vlow, lastDate };
}

//a=actual b=previous
function pctDiff(a,b) {
// function pctDiff(b,a) {
  return ((a - b) / b) * 100;
}

function fillper(vorigin){
	var vtofill =[];
    for (let i = 1; i < vorigin.length; i++) {
      const p = vorigin[i]; 
      const pp = vorigin[i-1]; 
	  var win=pctDiff(p, pp); 
	//   console.log(win,p,pp);
	  vtofill.push(Number(win.toFixed(2)));
	}
	// console.log(vtofill.length);
	return vtofill;
}


async function getPositions(market = null) {
  const bodyObj = {
    request: '/api/v4/collateral-account/positions/open',
    nonce: Date.now()
  };

  // Optional: filter by market, e.g. "ETH_PERP"
  if (market) bodyObj.market = market;

  const bodyStr = JSON.stringify(bodyObj);
  const payload = Buffer.from(bodyStr).toString('base64');
  const signature = crypto.createHmac('sha512', API_SECRET)
                          .update(payload)
                          .digest('hex');

  const res = await fetch('https://whitebit.com/api/v4/collateral-account/positions/open', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'X-TXC-APIKEY': API_KEY,
      'X-TXC-PAYLOAD': payload,
      'X-TXC-SIGNATURE': signature
    },
    body: bodyStr
  });

  const json = await res.json();
  return json;
}






















// Example usage:
(async () => {
  try {

	// var vsymb=["BTC_PERP"];
	// var vsymb=["BTC_USDT","ETH_USDT"];
	// var vsymb=["BTC_PERP","ETH_PERP"];
	var vsymb=["BTC_PERP","ETH_PERP","XRP_PERP","SOL_PERP"];
	// var vsymb=["BTC_PERP","ETH_PERP","XRP_PERP","SOL_PERP","DOGE_PERP","AVAX_PERP","SUI_PERP","TIA_PERP"];
	// var vsymb=["BTC_PERP","ETH_PERP","XRP_PERP","SOL_PERP","DOGE_PERP","AVAX_PERP","ADA_PERP","SUI_PERP","TIA_PERP"];



	// const opos=await getPositions();
	// console.log(opos);



// for(let si=0;si<2;si++){
for(let si=0;si<vsymb.length;si++){

	const symbol=vsymb[si];

    const { vclose, vhigh, vlow, lastDate } = await fetchCandles(symbol,hsize+1+lb);
    // console.log('vclose:', vclose);
    // console.log('vhigh:', vhigh);
    // console.log('vlow:', vlow);
	// console.log("pct",pctDiff(4674,4650));

	const now = new Date();
	// const day=now.getUTCDate();
	// const hour=now.getHours();
	// const ldate=lastDate.getUTCDate();
	const ldate=lastDate.getHours();
	const diffMs = now - lastDate;
	const diffMin = diffMs / (1000 * 60);
	// console.log(diffMin,now,lastDate);
	// return;


// const vwin = [
//    0.18, -0.16, -0.57,  1.24, -1.79,  0.89,  0.83, -0.53, -0.79,
//   -0.91,   1.4, -0.15, -0.17,  0.31,  -0.6,  0.64,  2.61, -0.35,
//    -1.6,  0.25, -0.69,  0.19, -0.18, -0.42, -0.71,  0.46,   0.6,
//     0.1, -0.69,   1.3, -0.47, -0.16,  0.11,  0.15, -0.72,  0.32,
//    2.21,  0.59, -0.24,  1.81, -0.63,  1.04,  0.26,    -1, -0.23,
//   -0.22,  -0.4, -0.13, -0.22,  0.95,  0.92, -1.04,  -0.1, -1.06,
//    0.41,  0.17, -1.62, -0.67,  0.56,  0.52,  -0.4, -0.07, -0.11,
//    0.36, -1.14, -0.84, -0.41, -1.87,  1.03,  1.38, -0.25,  -0.8,
//   -0.16,  0.42, -1.09, -1.97,   1.3,  2.99, -0.03, -0.56,  1.42,
//   -0.04, -0.22, -0.07,  0.25, -0.53,  -0.2,  0.71,  1.91, -0.81,
//   -0.05,  1.06,  1.05, ];

var vwin = fillper(vclose); 
const currentCandle = vwin[vwin.length - 0-lb];
if(lb>0){
	vwin = vwin.slice(0, -lb); 
	// console.log("currentCandle",vwin,currentCandle);
}

    // console.log(vwin.length, symbol,'Last candle date:', lastDate.toISOString());
// console.log(vwin);
// return;
// const fs   = require('fs')
// const vm   = require('vm')
// const path = require('path')

// // 1. Load brain.js into this context
// const brainPath = path.join(__dirname, 'brain.min.js')
// const brainCode = fs.readFileSync(brainPath, 'utf8')
// vm.runInThisContext(brainCode)   // exposes global `brain`

// // 2. Your raw series
// const vwin = [
//   -0.81, -0.46, -1.01,  3.88, -0.64, -2.03,  0.04,  2.52,  0.44,
//   -0.15,  0.13,  0.75,  1.34,  1.41,  0.55, -2.22,  -0.3,  2.12,
//   -2.97,  0.83,  0.46, -0.37,  0.35,  1.39, -1.95, -2.37,  -1.3
// ]

// 3. Min–max normalization helpers
const min = Math.min(...vwin)
const max = Math.max(...vwin)
const normalize   = x => (x - min) / (max - min)
const denormalize = y => y * (max - min) + min
const normSeries  = vwin.map(normalize)

// 4. Build train/test sets for a given window size
// Assumes you’ve already loaded brain.js, defined vwin, normalize/denormalize, and normSeries

/**
 * Build training+test sets for a given input size and train length
 * Training = first 80% (or up to trainLength)
 * Test     = last 20%  (remaining after train split)
 */
function buildTrainTest(inputSize, trainLength) {
  const seriesLen = normSeries.length;
  const splitIdx  = Math.floor(Math.min(trainLength, seriesLen) * 0.8);

  const train = [];
  for (let j = inputSize; j < splitIdx; j++) {
    train.push({
      input:  normSeries.slice(j - inputSize, j),
      output: [ normSeries[j] ]
    });
	// console.log("tr",normSeries[j]);
  }

  const test = [];
  for (let j = Math.max(inputSize, splitIdx); j < seriesLen; j++) {
    test.push({
      input:  normSeries.slice(j - inputSize, j),
      output: [ normSeries[j] ]
    });
	// console.log("t",normSeries[j]);
  }

  return { train, test };
}

/**
 * Train on train set, evaluate on test set, then predict next value
 * Returns: { mae, mse, predRaw }
 */
function trainTestPredict(inputSize, trainLength) {
  const { train, test } = buildTrainTest(inputSize, trainLength);
  const net = new brain.NeuralNetwork({ hiddenLayers: [5] });

  net.train(train, {
    iterations: 4000,
    errorThresh: 0.005,
    log: false
  });

  let sumAE = 0, sumSE = 0;
  // Evaluate on unseen test data
  test.forEach(sample => {
    const predNorm = net.run(sample.input)[0];
    const predRaw  = denormalize(predNorm);
    const actual   = denormalize(sample.output[0]);
    const err      = predRaw - actual;
    sumAE += Math.abs(err);
    sumSE += err * err;
  });

  const mae = sumAE / test.length;
  const mse = sumSE / test.length;

  // Now predict one more step using the final window from the full series
  const lastWindowNorm = normSeries.slice(-inputSize);
  const nextPredNorm   = net.run(lastWindowNorm)[0];
  const nextPredRaw    = denormalize(nextPredNorm);

  return { mae, mse, predRaw: nextPredRaw };
}

// 7. Loop over inputSizes, trainLengths, 2 runs each
// const inputSizes    = [9];
const inputSizes    = [4];
const trainLengths  = [hsize];
const runsPerConfig = 3;


let winnerPred   = 0;
let winnerScore  = Infinity; // lower is better

for (const size of inputSizes) {
  for (const tlen of trainLengths) {
    const allMAEs  = [];
    const allMSEs  = [];
    const allPreds = [];

    for (let run = 0; run < runsPerConfig; run++) {
      const { mae, mse, predRaw } = trainTestPredict(size, tlen);
      allMAEs.push(mae);
      allMSEs.push(mse);
      allPreds.push(predRaw);
    }

    // Median prediction
    allPreds.sort((a, b) => a - b);
    const mid     = Math.floor(allPreds.length / 2);
    const medianP = allPreds.length % 2
      ? allPreds[mid]
      : (allPreds[mid - 1] + allPreds[mid]) / 2;

    // Average MAE/MSE across runs
    const avgMAE = allMAEs.reduce((s, x) => s + x, 0) / allMAEs.length;
    const avgMSE = allMSEs.reduce((s, x) => s + x, 0) / allMSEs.length;

    // Balanced score: MAE + sqrt(MSE)
    // const score =   Math.sqrt(avgMSE);
    // const score = avgMAE ;
    const score = avgMAE + Math.sqrt(avgMSE);

    if (score < winnerScore) {
      winnerScore = score;
      winnerPred  = medianP.toFixed(4);
    }

    // Optional debug:
    // console.log(`Config: win=${size}, trainLen=${tlen}, score=${score.toFixed(6)}, pred=${medianP.toFixed(4)}`);
  }
}


if(Math.abs(winnerPred)<0.2 && tries<3){
	si--; tries++; 
	// cotm("recalc",tries);
	continue; 
}
tries=0;


console.log(symbol,currentCandle, "winner", Number(winnerPred).toFixed(2), winnerScore.toFixed(2), lastDate.getDate());
 










if(markets==0){
	const marketsRes = await fetch('https://whitebit.com/api/v4/public/markets');
	markets = await marketsRes.json();
}

async function open_order(symbol, side, entry, slPerc, tpPerc, riskUSDT) {
	// await sleep(2000);
	console.log("symb",symbol);return;
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
//   return;

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















var freeMargin=50;

// await open_order(symbol,winnerPred>=0?"buy":"sell",vclose[vclose.length-1],2.1,3.1,3);




























}



  } catch (err) {
    console.error('Error:', err);
  }
})();
