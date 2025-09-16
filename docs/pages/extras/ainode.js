const fs = require('fs');
const path = require('path');
const vm = require('vm');
vm.runInThisContext(
  fs.readFileSync(path.join(__dirname, 'brain.min.js'), 'utf8')
);
const { LSTMTimeStep } = brain.recurrent;

// ─── CONFIG ────────────────────────────────────────────────────────────────────
const LIMIT       = [60];                  // total candles to fetch
const INTERVAL    = '1d';                        // candle interval
const INPUT_SIZES = [5]; 
// const INPUT_SIZES = [2,2,3,3,4,4,5,5,5,6,6,7,7,8,8]; 
// ───────────────────────────────────────────────────────────────────────────────
var lb = 0;
var vsum = 0;

// safe normalization (avoid div/0 + keep numbers inside [0,1])
function makeNormalizer(arr) {
  const max = Math.max(...arr);
  const min = Math.min(...arr);
  const range = max - min || 1; // avoid division by zero
  const normalize = x => (x - min) / range;
  const denormalize = y => (y * range) + min;
  return { normalize, denormalize };
}

async function run(symbol, histsize) {
  // fetch and extract closes
  const url  = `https://whitebit.com/api/v1/public/kline?market=${symbol}&interval=${INTERVAL}&limit=${histsize}`;
  const res  = await fetch(url);
  const json = await res.json();
  var closes = json.result.map(c => +c[2]);

  if (lb > 0) closes = closes.slice(0, -lb);
  console.log(closes);

  // normalization
  const { normalize, denormalize } = makeNormalizer(closes);
  const closesN = closes.map(normalize);

  // run through each window size
  for (const windowSize of INPUT_SIZES) {
    const splitIdx = Math.floor(closesN.length * 0.6);
    const train = closesN.slice(0, splitIdx);
    const test  = closesN.slice(splitIdx - windowSize);

    const dataset = [];
    for (let i = windowSize; i < train.length; i++) {
      const seqIn  = train.slice(i - windowSize, i).map(v => [v]);
      const seqOut = [[train[i]]];
      dataset.push({ input: seqIn, output: seqOut });
    }

    const net = new LSTMTimeStep({
      inputSize: 1,
      hiddenLayers: [10],
      outputSize: 1
    });
    net.train(dataset, {
      learningRate: 0.001,
      errorThresh: 0.0003,
      iterations: 8000
    });

    let wins = 0, total = 0;
    for (let i = windowSize; i < test.length - 1; i++) {
      const seqIn   = test.slice(i - windowSize, i).map(v => [v]);
      const pred    = denormalize(Math.min(Math.max(net.run(seqIn)[0], 0), 1)); // clamp
      const actual  = denormalize(test[i]);
      const next    = denormalize(test[i + 1]);
      if ((pred - actual) * (next - actual) > 0) wins++;
      total++;
    }

    // --- predict next candle from latest data ---
    const lastSeqN = closesN.slice(-windowSize).map(v => [v]);
    const rawPred  = net.run(lastSeqN)[0];
    const predNext = denormalize(Math.min(Math.max(rawPred, 0), 1)); // clamp again
    const lastClose = closes[closes.length - 1];
    const pctChange = ((predNext - lastClose) / lastClose) * 100;

    console.log(
      `inputSize=${windowSize} → Wins: ${wins}/${total} ` +
      `Rate: ${(wins / total * 100).toFixed(2)}%  ` +
      `pred -> ${pctChange >= 0 ? '+' : ''}${pctChange.toFixed(2)}%`
    );

    if ((wins / total * 100) >= 50) {
      if (pctChange >= 0) vsum++; else vsum--;
    } else {
      if (pctChange < 0) vsum++; else vsum--;
    }
	return;
  }
}

(async () => {
  lb = 0;
  const symbols = ['BTC_USDT','ETH_USDT','SOL_USDT','XRP_USDT'];
  var symbol = symbols[3];
  
  vsum = 0;
  for (let i = 0; i < LIMIT.length; i++) {
    var res = await run(symbol, LIMIT[i]);
    console.log("run", symbol, res, vsum);
    if (vsum >= 3 || vsum <= -3) break;
  }
  console.log("run",lb, symbol, vsum);
})();
