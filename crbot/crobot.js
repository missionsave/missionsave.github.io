

let symbols = [
	{ name: "Bitcoin", pair: "BTCUSDT", nc: "BTC_PERP", trade:0 },
	{ name: "Ethereum", pair: "ETHUSDT", nc: "ETH_PERP" , trade:1 },
	{ name: "Solana", pair: "SOLUSDT", nc: "SOL_PERP", trade:0},
	{ name: "XRP", pair: "XRPUSDT", nc: "XRP_PERP" , trade:0},
	// { name: "BNB", pair: "BNBUSDT", nc: "XRP_PERP" , trade:0 },
	// { name: "ADA", pair: "ADAUSDT", nc: "XRP_PERP" , trade:0},
	// { name: "AVAX", pair: "AVAXUSDT", nc: "XRP_PERP", trade:0  },
	// { name: "SUI", pair: "SUIUSDT", nc: "XRP_PERP" , trade:0 },
	// { name: "Celestia", pair: "TIAUSDT", nc: "XRP_PERP", trade:0  },
];
const Math_Round = (v, d = 4) => {
  const p = 10 ** d;
  return Math.round((v ) * p) / p;
};

// const Math_Round = (v, d = 2) => {
//   switch (d) {
//     case 0:  return Math.round(v);
//     case 1:  return Math.round(v * 10)       * 0.1;
//     case 2:  return +Math.round(v * 100)      * 0.01;
//     case 3:  return Math.round(v * 1000)     * 0.001;
//     case 4:  return Math.round(v * 10000)    * 0.0001;
//     case 5:  return Math.round(v * 100000)   * 0.00001;
//     case 6:  return Math.round(v * 1000000)  * 0.000001;
//     case 7:  return Math.round(v * 10000000) * 0.0000001;
//     case 8:  return Math.round(v * 100000000) * 0.00000001;
//     case 9:  return Math.round(v * 1000000000) * 0.000000001;
//     case 10: return Math.round(v * 10000000000) * 0.0000000001;
//     default: {
//       const f = 10 ** d; // fallback for >10
//       return Math.round(v * f) * (1 / f);
//     }
//   }
// };

const sleepms = (ms) => new Promise(resolve => setTimeout(resolve, ms));

function scoreCloseness(arr) {
  const mean = arr.reduce((a, b) => a + b, 0) / arr.length;
  const variance = arr.reduce((sum, x) => sum + Math.pow(x - mean, 2), 0) / arr.length;
  return 1 / (1 + variance); // higher score = closer values
}
function robustnessScore(resultsPerCoin) {
  const mean = resultsPerCoin.reduce((a,b)=>a+b,0) / resultsPerCoin.length;
  const variance = resultsPerCoin.reduce((s,x)=>s+Math.pow(x-mean,2),0) / resultsPerCoin.length;
  return mean / (1 + Math.sqrt(variance)); 
  // maior = melhor equilíbrio entre média alta e consistência
}

async function fetchData_(sym,interval,historyLength) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  return await (await fetch(url)).json();
}

const fs = require("fs"), path = require("path");

async function fetchData(sym, interval, historyLength) {
  const f = path.join(__dirname, `${sym}_${interval}_${historyLength}.json`);
  try {
    const s = fs.statSync(f);
    if (Date.now() - s.mtimeMs < 90*52*7*24*60*60*1000)
    // if (Date.now() - s.mtimeMs < 7*24*60*60*1000)
      return JSON.parse(fs.readFileSync(f, "utf8"));
  } catch {}
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  const j = await (await fetch(url)).json();
  fs.writeFileSync(f, JSON.stringify(j));
  return j;
}
  // Helper: Calculate ATR
  const calculateATR = (rows, period = 14) => {
    if (rows.length < period) return 1;
    let trSum = 0;
    for (let i = 1; i < period; i++) {
      const high = rows[i][0], low = rows[i][1], prevClose = rows[i - 1][2];
      const tr = Math.max(high - low, Math.abs(high - prevClose), Math.abs(low - prevClose));
      trSum += tr;
    }
    return trSum / period;
  };

  // Helper: Calculate RSI
  const calculateRSI = (closes, period = 14) => {
    if (closes.length < period) return 50;
    let gains = 0, losses = 0;
    for (let i = 1; i < period; i++) {
      const diff = closes[i] - closes[i - 1];
      if (diff > 0) gains += diff;
      else losses -= diff;
    }
    const avgGain = gains / period;
    const avgLoss = losses / period || 1;
    const rs = avgGain / avgLoss;
    return 100 - (100 / (1 + rs));
  };


let lookhist=400;
let equity = 100;
let equitymax = -1;

let riskFrac = 0.2;
// let riskFrac = 0.22;
let side="";
let entry=0;
let sli = 6.0;   // stop distance %
let tpi = 0.6;
let param1=7;
let dbg=0;
function simulate_order(equity, open, high, low, close, closewin) {
  // Determine stop‐loss and take‐profit prices
  const slPrice = closewin === 1 ? open * (1 - sli / 100) : open * (1 + sli / 100);
  const tpPrice = closewin === 1 ? open * (1 + tpi / 100) : open * (1 - tpi / 100);

  // Calculate maximum risk and position size
  const maxLoss = equity * riskFrac;
  const distToSL = Math.abs(open - slPrice);
  const size = maxLoss / distToSL;

  // Figure out which barrier (SL or TP) is hit first
  let exitPrice, exitType;
  const hitSL = closewin === 1 ? low <= slPrice : high >= slPrice;
  const hitTP = closewin === 1 ? high >= tpPrice : low <= tpPrice;

  if (hitSL && hitTP) {
    // ===== SMART DECISION WHEN BOTH BARRIERS ARE INSIDE THE SAME CANDLE =====
    const distSL = Math.abs(open - slPrice);
    const distTP = Math.abs(tpPrice - open);

    // 1. If barriers are at similar distance (within ~20%), assume the closer one is first
    const ratio = distSL / distTP;
    if (ratio > 0.8 && ratio < 1.25) {
      if (distTP < distSL) {
        exitPrice = tpPrice;
        exitType = 'TP';
      } else {
        exitPrice = slPrice;
        exitType = 'SL';
      }
    } else {
      // 2. Otherwise use candle path assumption based on open→close movement
      if (closewin === 1) { // BUY
        if (close >= open) {
          // Bullish candle: assume price dipped to low before rising → SL first
          exitPrice = slPrice;
          exitType = 'SL';
        } else {
          // Bearish candle: assume price spiked up before dropping → TP first
          exitPrice = tpPrice;
          exitType = 'TP';
        }
      } else { // SELL
        if (close <= open) {
          // Bearish candle: assume price spiked up first → SL first
          exitPrice = slPrice;
          exitType = 'SL';
        } else {
          // Bullish candle: assume price dipped down before rising → TP first
          exitPrice = tpPrice;
          exitType = 'TP';
        }
      }
    }

  } else if (hitSL) {
    exitPrice = slPrice;
    exitType = 'SL';
  } else if (hitTP) {
    exitPrice = tpPrice;
    exitType = 'TP';
  } else {
    // neither hit: exit at close
    exitPrice = close;
    exitType = 'MKT';
  }

  // Profit & loss calculation
  const pnl = closewin === 1 ? (exitPrice - open) * size : (open - exitPrice) * size;

  // Debug helper: log cases where both hit
//   if (hitSL && hitTP ) {
//     console.log("Both hit -> decided:", exitType, "open:", open, "close:", close, "sl:", slPrice, "tp:", tpPrice);
//   }

  // Update equity and log
  equity += pnl;
  if (dbg) console.log(
    exitType,
    closewin === 1 ? 'BUY' : 'SELL',
    Math.round(open),
    "tp:", Math.round(tpPrice),
    "sl:", Math.round(slPrice),
    Number(pnl.toFixed(2)),
    Math.round(equity)
  );
  if (dbg) console.log("o:", open, "h:", high, "l:", low, "c:", close);

  return pnl;
}

function simulate_order_v1(equity,open, high, low, close, closewin) {
  // Determine stop‐loss and take‐profit prices
  const slPrice = closewin === 1
    ? open * (1 - sli / 100)
    : open * (1 + sli / 100);
  const tpPrice = closewin === 1
    ? open * (1 + tpi / 100)
    : open * (1 - tpi / 100);

  // Calculate maximum risk and position size
  const maxLoss = equity * riskFrac;
  const distToSL = Math.abs(open - slPrice);
  const size = maxLoss / distToSL;

  // Figure out which barrier (SL or TP) is hit first
  let exitPrice, exitType;
  const hitSL = closewin === 1 ? low <= slPrice : high >= slPrice;
  const hitTP = closewin === 1 ? high >= tpPrice : low <= tpPrice;

  if (hitSL && hitTP) {
	console.log("BH");
    // both hit: assume the closer barrier is touched first
    const diffSL = distToSL;
    const diffTP = Math.abs(tpPrice - open);
    if (diffSL < diffTP) {
      exitPrice = slPrice;
      exitType = 'SL';
    } else {
      exitPrice = tpPrice;
      exitType = 'TP';
    }
  } else if (hitSL) {
    exitPrice = slPrice;
    exitType = 'SL';
  } else if (hitTP) {
    exitPrice = tpPrice;
    exitType = 'TP';
  } else {
    // neither hit: exit at close
    exitPrice = close;
    exitType = 'MKT';
  }

  // Profit & loss calculation
  const pnl = closewin === 1
    ? (exitPrice - open) * size
    : (open - exitPrice) * size;

  // Update equity and log
  equity += pnl;
    if (hitSL && hitTP) {
		console.log("BH",pnl);
	}

  if(dbg)console.log(exitType,closewin === 1 ? 'BUY' : 'SELL', Math_Round(open),"tp:",Math_Round(tpPrice),"sl:", Math_Round(slPrice), Number(pnl.toFixed(2)),Math_Round(equity),hitSL && hitTP);
  if(dbg)console.log("o:",Math_Round(open),"h:",Math_Round(high),"l:",Math_Round(low),"c:",Math_Round(close));

//   console.log(
//     `[${exitType}] ${closewin === 1 ? 'BUY' : 'SELL'} ` +
//     `entry=${open.toFixed(2)} exit=${exitPrice.toFixed(2)} ` +
//     `size=${size.toFixed(4)} PnL=${pnl.toFixed(2)} ` +
//     `equity=${equity.toFixed(2)}`
//   );

  return pnl;
}
let neareast=0;
function seek(i,raw, rowsAll) {
	const row = rowsAll[i];
	const row1 = rowsAll[i - 1];
	const row2 = rowsAll[i - 2];
	const row3 = rowsAll[i - 3];
	const row4 = rowsAll[i - 4];
	neareast = Infinity;
	let closewin = -1;
	let winj = 0;
	let lh = i - lookhist;
	if (lh < 4) lh = 3;
	for (let j = i - 4; j > lh; j--) {
		// console.log(rowsAll[i][0], raw[i][2]);
		const rowb = rowsAll[j];
		const rowb1 = rowsAll[j - 1];
		const rowb2 = rowsAll[j - 2];
		const rowb3 = rowsAll[j - 3];
		const rowb4 = rowsAll[j - 4];
		let nr = 0;
		for (let ni = 6; ni < 7; ni++) {
		// for (let ni = 4; ni < row.length - 1; ni++) {
			nr += Math.abs(row[ni] - rowb[ni]);
			nr += Math.abs(row1[ni] - rowb1[ni]);
			nr += Math.abs(row2[ni] - rowb2[ni]);
			nr += Math.abs(row3[ni] - rowb3[ni]);
			nr += Math.abs(row4[ni] - rowb4[ni]);
		}
		if (nr < neareast) {
			neareast = +nr;
			winj = j;
			closewin = (raw[j - 0][4] > raw[j - 1][4]) ? 1 : 0;
			// console.log(j, neareast,closewin);
		}
	}
	return closewin;
}



function findbest(idx, raw) {
	equitymax = -1;
	const equitybreak=300;

	let param1win = 6;
	let riskFracwin = 0;
	let tpwin = 0;
	let slwin = 0;
	let lookhistwin=0;
 
	param1=	11;
	for (param1 = 10; param1 < 17; param1++) 
		{


		// Prepare data: [high, low, close, volume, RSI, ATR]
		const rowsAll = raw.map((c, i) => {
			const closesh = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[2]);
			const rsih = calculateRSI(closesh, param1);
			const closesl = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[3]);
			const rsil = calculateRSI(closesl, param1);
			const closes = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[4]);
			const rsi = calculateRSI(closes, param1);
			const atr = 0;//calculateATR(raw.slice(Math.max(0, i - param1), i + 1));
			return [+c[2], +c[3], +c[4], +c[5], rsih, rsil, rsi, atr];
		});
		for (tpi = 5; tpi >1.3; tpi -= 0.2) {
		// for (tpi = 2.5; tpi < 4.4; tpi += 0.2) {
			for (sli = 4; sli >1; sli -= 0.2) {
			// for (sli = 2.2; sli < 4; sli += 0.2) {
				if((sli+0.3)>=tpi)continue;
				riskFrac = 0.03;
				// for (riskFrac = 0.9; riskFrac < 0.1; riskFrac += 0.01)
					 {
					// for (lookhist = 750; lookhist < 900; lookhist += 100) 
					// for (lookhist = 50; lookhist < 600; lookhist += 150) 
						{
						// lookhist=28;
						lookhist=3100;
						// lookhist=28;
						let equityl = 100;
						for (let i = idx -100; i < idx; i++) {


							let closewin = seek(i, raw, rowsAll);
							// const candle = raw[i];
							// const dt = `${new Date(candle[0]).toISOString().split('T')[0]} ${String(new Date(candle[0]).getUTCHours()).padStart(2, '0')}`;
 
							const nextcandle = raw[i + 1]; 
							const pnl=simulate_order(equityl,Math_Round(nextcandle[1]), Math_Round(nextcandle[2]), Math_Round(nextcandle[3]), Math_Round(nextcandle[4]), closewin);
							equityl+=pnl;

							// if(i>(idx-4)){
							// 	if(pnl<0){
							// 		equityl=0;
							// 		break;
							// 	}
							// }
							// if (equityl > equitymax) {
							// 	equitymax = equityl;
							// 	param1win = param1;
							// 	riskFracwin = riskFrac;
							// 	tpwin = tpi;
							// 	slwin = sli;
							// 	lookhistwin=lookhist;
							// 	// console.log(equityl, param1win, riskFracwin, tpwin, slwin,lookhistwin);
							// }
						}

							if (equityl > equitymax) {
								equitymax = equityl;
								param1win = param1;
								riskFracwin = riskFrac;
								tpwin = tpi;
								slwin = sli;
								lookhistwin=lookhist; 

								// console.log(Math_Round(equityl), param1win, riskFracwin.toFixed(2), "tpwin:",tpwin, "slwin:",slwin,lookhistwin);
							}
		if(equitymax>equitybreak)break;
					}
		if(equitymax>equitybreak)break;
				}
		if(equitymax>equitybreak)break;
			}
		if(equitymax>equitybreak)break;
		}
		if(equitymax>equitybreak)break;
	}
 
	param1 = param1win;
	riskFrac = riskFracwin;
	tpi = tpwin;
	sli = slwin;
	lookhist=lookhistwin;
	
	// if(dbg)
	console.log("--------------------");
	console.log(Math_Round(equitymax), param1win, riskFracwin.toFixed(2), "tpwin:",Math_Round(tpwin), "slwin:",Math_Round(slwin),lookhistwin);
}

async function simulate(raw) {

	let equitymax=100;
	let param1win=6;
	let riskFracwin=0;
	let tpwin=0;
	let slwin=0;
	// param1=21;
	// for(param1=6;param1<20;param1++)	
	
	{


		// Prepare data: [high, low, close, volume, RSI, ATR]
		const rowsAll = raw.map((c, i) => {
			const closesh = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[2]);
			const rsih = calculateRSI(closesh,param1);
			const closesl = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[3]);
			const rsil = calculateRSI(closesl,param1);
			const closes = raw.slice(Math.max(0, i - param1), i + 1).map(c => +c[4]);
			const rsi = calculateRSI(closes,param1);
			const atr = 0;//calculateATR(raw.slice(Math.max(0, i - param1), i + 1));
			return [+c[2], +c[3], +c[4], +c[5], rsih, rsil, rsi, atr];
		});
		// console.log(rowsAll.length);

	// for(tpi=1;tpi<4.5;tpi+=0.5)
	// for(sli=1;sli<3.5;sli+=0.5)
	// for(riskFrac=0.1;riskFrac<0.35;riskFrac+=0.2)
	// for(lookhist=200;lookhist<700;lookhist+=100)
	{


		// equity=100;

let closewin=1;
		//simulate
		let simulatesize=5;
		
		let lb=5;
		let i=rowsAll.length-2;
		// for (let i = rowsAll.length - lb; i < rowsAll.length - (lb-simulatesize); i++) 
		{
			// let closewin=seek(i,raw, rowsAll);
			// let closewin=1;
			// if(equitymax<150)closewin=!closewin;

			const candle = raw[i];
			const date = new Date(candle[0]);
			const dt = `${date.toISOString().split('T')[0]} ${String(date.getUTCHours()).padStart(2, '0')} ${date.toLocaleDateString('en-US', { weekday: 'short', timeZone: 'UTC' })}`;


			const open  = Math_Round(candle[1]);
			const high  = Math_Round(candle[2]);
			const low   = Math_Round(candle[3]);
			const close = Math_Round(candle[4]);

			if(dbg)console.log(raw.length,i,"neareast:",neareast.toFixed(2));
			if(dbg)console.log(dt,"o:",Math_Round(open),"h:",Math_Round(high),"l:",Math_Round(low),"c:",close,closewin);//,neareast.toFixed(0),winj);
			
			if(i<raw.length-1){
			const nextcandle=raw[i+1];
			const pnl= simulate_order(equity,Math_Round(nextcandle[1]), Math_Round(nextcandle[2]),Math_Round(nextcandle[3]), Math_Round(nextcandle[4]),closewin);
			equity+=pnl;
			if(pnl<-4)closewin=0;
			if(pnl>4)closewin=1;
			// simulate_order(open,high,low,close,closewin);
			}

		}
		// if(equity>equitymax){
		// 	equitymax=equity;
		// 	param1win=param1;
		// 	riskFracwin=riskFrac;
		// 	tpwin=tpi;
		// 	slwin=sli;
		// 	console.log(equity,param1win,riskFracwin,tpwin,slwin);
		// }
	}}

}

async function simulate_v1(raw, k = 1) {
  // Prepare feature rows
  const rowsAll = raw.map((c, i) => {
    const window = raw.slice(Math.max(0, i - 14), i + 1);
    const highs = window.map(c => +c[2]);
    const lows = window.map(c => +c[3]);
    const closes = window.map(c => +c[4]);
    return [
      +c[2], // high
      +c[3], // low
      +c[4], // close
      +c[5], // volume
      calculateRSI(highs),
      calculateRSI(lows),
      calculateRSI(closes),
      calculateATR(window)
    ];
  });

  console.log("Total rows:", rowsAll.length);

  const results = [];

  // Compare last 10 rows against history
  for (let i = rowsAll.length - 210; i < rowsAll.length - 200; i++) {
    const row = rowsAll[i];
    const distances = [];

    for (let j = 1; j < i - 1; j++) {
      const rowb = rowsAll[j];
      let dist = 0;
      for (let ni = 4; ni < row.length; ni++) {
        dist += Math.abs(row[ni] - rowb[ni]);
      }
      const outcome = raw[j][4] > raw[j - 1][4] ? 1 : 0;
      distances.push({ dist, outcome, j });
    }

    // Sort by distance and take k nearest
    distances.sort((a, b) => a.dist - b.dist);
    const nearest = distances.slice(0, k);

    // Majority vote outcome    
	const candle = raw[i];
    const dt = `${new Date(candle[0]).toISOString().split('T')[0]} ${String(new Date(candle[0]).getUTCHours()).padStart(2, '0')}`;
    const score = nearest.reduce((sum, d) => sum + d.outcome, 0) / k;
    results.push({dt, i,cl:row[2], pred: score > 0.5 ? 1 : 0, conf: score });
  }

  console.log("Predictions:", results);
  return results;
}

async function prodution(raw) {
	var idx=raw.length-2;
	findbest(idx,raw);
}

async function test() {
	for (let coin of symbols) {
		const rawtt = await fetchData(coin.pair,"1d",850);
		// let raw=rawtt;
		equity=1000;
		// for(let i=100;i>0;i--){
		for(let i=200;i>100;i--){
			let raw=rawtt;
			raw=raw.slice(0,-i);
			console.log(raw.length);
			dbg=0;
			await prodution(raw);
			dbg=1;
			// equity=100;
			await simulate(raw);
			console.log("equity:",Math_Round(equity,0));
		}
	}
} 
function pctDiff(b,a) {
  return ((a - b) / b) * 100;
}
/**
 * Weighted Moving Average (last value only)
 * @param {number[]} arr - input series
 * @param {number} size - window size
 * @param {"back"|"front"} mode - "back" = latest heavier, "front" = earliest heavier
 * @returns {number|undefined} last WMA value (undefined if not enough data)
 */
const weightedMA = (arr, size = 5, mode = "back") => {
  if (arr.length < size) return undefined;

  const weights = mode === "back"
    ? Array.from({ length: size }, (_, i) => i + 1)        // [1..size]
    : Array.from({ length: size }, (_, i) => size - i);    // [size..1]

  const denom = weights.reduce((a, b) => a + b, 0);
  const window = arr.slice(-size);

  const num = window.reduce((sum, v, j) => sum + v * weights[j], 0);
  return num / denom;
};
// equity: array of equity values
const equityTrendScore_v1 = (equity) => {
  const n = equity.length;
  const t = Array.from({length: n}, (_, i) => i+1);
  const meanT = t.reduce((a,b)=>a+b,0)/n;
  const meanE = equity.reduce((a,b)=>a+b,0)/n;

  // slope
  const num = t.reduce((s,ti,i)=>s+(ti-meanT)*(equity[i]-meanE),0);
  const den = t.reduce((s,ti)=>s+(ti-meanT)**2,0);
  const slope = num/den;

  // residuals
  const residuals = equity.map((e,i)=> e - (meanE + slope*(t[i]-meanT)));
  const std = Math.sqrt(residuals.reduce((s,r)=>s+r*r,0)/n);

  return slope/std;
};
const equityTrendScore = (equity, window = 50) => {
  const n = equity.length;
  if (n < 3) return { slope: 0, ts: 0, r2: 0, sharpe: 0, up: 0, total: 0, len: n, win: Math.min(window, n) };

  const win = Math.min(window, n);
  const start = n - win;
  const E = equity.slice(start);
  const T = Array.from({ length: win }, (_, i) => i + 1);

  const mean = (arr) => arr.reduce((a, b) => a + b, 0) / arr.length;
  const meanT = mean(T);
  const meanE = mean(E);

  // Regression slope
  const num = T.reduce((s, t, i) => s + (t - meanT) * (E[i] - meanE), 0);
  const den = T.reduce((s, t) => s + (t - meanT) ** 2, 0) || 1;
  const slope = num / den;
  const intercept = meanE - slope * meanT;

  // Residuals
  const residuals = E.map((e, i) => e - (intercept + slope * T[i]));
  const residVar = residuals.reduce((s, r) => s + r * r, 0) / win;
  const residStd = Math.sqrt(residVar) || 0;

  // R²
  const ssTot = E.reduce((s, e) => s + (e - meanE) ** 2, 0) || 1;
  const ssRes = residuals.reduce((s, r) => s + r * r, 0);
  const r2 = 1 - ssRes / ssTot;

  // Returns
  const R = [];
  for (let i = 1; i < E.length; i++) {
    const prev = E[i - 1];
    const ret = prev ? ((E[i] / prev) - 1) : 0;
    R.push(ret);
  }
  const meanR = R.length ? mean(R) : 0;
  const stdR = R.length ? Math.sqrt(R.reduce((s, r) => s + (r - meanR) ** 2, 0) / R.length) : 0;
  const sharpe = stdR ? meanR / stdR : 0;
  const up = R.length ? (R.filter(r => r > 0).length / R.length) : 0;

  // Trend score
  const ts = residStd ? (slope / residStd) : 0;

  // --- Composite Score ---
  // Normalize slope by last equity to get % slope
  const last = equity[equity.length - 1] || 1;
  const pctSlope = slope / last;

  // Weighted sum (tune weights as needed)
  const total =
    (pctSlope * 100) * 0.4 +   // slope in % terms
    ts * 0.3 +                 // trend score
    r2 * 0.2 +                 // fit quality
    sharpe * 0.05 +            // consistency of returns
    up * 0.05;                 // % of up periods

//   return { slope, pctSlope, ts, r2, sharpe, up, total, len: n, win };
  return total;
};
/**
 * Weighted Moving Average
 * @param {number[]} arr - input series
 * @param {number} size - window size
 * @param {"back"|"front"} mode - "back" = latest heavier, "front" = earliest heavier
 * @returns {number[]} array of WMAs (undefined for first size-1 entries)
 */
const weightedMAv = (arr, size = 5, mode = "back") => {
  const out = [];
  const weights = mode === "back"
    ? Array.from({ length: size }, (_, i) => i + 1)        // [1..size]
    : Array.from({ length: size }, (_, i) => size - i);    // [size..1]
  const denom = weights.reduce((a, b) => a + b, 0);

  for (let i = 0; i < arr.length; i++) {
    if (i < size - 1) {
      out.push(undefined); // not enough data yet
      continue;
    }
    const window = arr.slice(i - size + 1, i + 1);
    const num = window.reduce((sum, v, j) => sum + v * weights[j], 0);
    out.push(num / denom);
  }
  return out;
};
 

async function test2() {
	for (let coin of symbols) {
		const raw = await fetchData(coin.pair, "1d", 1000);

		let equitymax = 0;
		let scoremax = 0;
		let sib=2;
		// tpi= 0.5; sli= 2.0;
		riskFrac=0.2;
		// for(riskFrac=0.05;riskFrac<=0.3;riskFrac+=0.05)
		for(sli=6;sli<6.7;sli+=0.1)
		for(tpi=0.5;tpi<0.8;tpi+=0.1)
		for (let sib = 2; sib < 6; sib++) 
			{
				let sif=2;
			for (let sif = 2; sif < 6; sif++) 
				{
					let equityv=[];
				equity = 1000;
				let cw = 1;
				let pdr = []; 
				let until=900;
				for (let i = until-350; i < until-1; i++) {
				// for (let i = 0; i < 950-1; i++) {
				// for (let i = 0; i < raw.length-1; i++) { 

					const nextcandle = raw[i + 1];
					const pnl = simulate_order(equity, Math_Round(nextcandle[1]), Math_Round(nextcandle[2]), Math_Round(nextcandle[3]), Math_Round(nextcandle[4]), cw);
					equity += pnl;
					equityv.push(equity);
					const wmb = Math_Round(weightedMA(pdr, sib, "back"));
					const wmf = Math_Round(weightedMA(pdr, sif, "front"));
					if (wmb > 0 && wmf < 0) cw = 0;
					if (wmb < 0 && wmf > 0) cw = 1;
					// if(wmb>0 && wmf>0)cw=1;
					// if(wmb<0 && wmf<0)cw=0;
					let pd = pctDiff(raw[i][4], nextcandle[4]);
					pdr.push(pd);

					// const digits=4;
					// const candle = raw[i];
					// const date = new Date(candle[0]);
					// const dt = `${date.toISOString().split('T')[0]} ${String(date.getUTCHours()).padStart(2, '0')} ${date.toLocaleDateString('en-US', { weekday: 'short', timeZone: 'UTC' })}`;
					// console.log(dt,"equity:",Math_Round(equity,0),"pd:",Math_Round(pd),"pnl:",Math_Round(pnl),"wmb:",wmb,"wmf:",wmf,"cw:",cw);
					// console.log("ohlc:",Math_Round(candle[1],digits),Math_Round(candle[2],digits),Math_Round(candle[3],digits),Math_Round(candle[4],digits));
					// if(i==raw.length-2){
					// 	console.log("ohlc:",Math_Round(nextcandle[1]),Math_Round(nextcandle[2]),Math_Round(nextcandle[3]),Math_Round(nextcandle[4],digits));
					// }
				}
				const score=equityTrendScore_v1(equityv);
				// console.log("score:",score);
				if (score > scoremax) {
				// if (equity > equitymax) {
					scoremax=score;
					equitymax = equity;
					// console.log("equity:", Math_Round(equity, 0),"sib:",sib,"sif:",sif,"tpi:",Math_Round(tpi),"sli:",Math_Round(sli),"riskFrac=",Math_Round(riskFrac));
					console.log("equity:", Math_Round(equity, 0),"sib:",sib,"sif:",sif,"score:",Math_Round(score,4),"tpi:",Math_Round(tpi),"sli:",Math_Round(sli),"riskFrac=",Math_Round(riskFrac));
				}
			}
		}
	}
}


// #region pattern


async function test3_v1() {
	async function test2() {
	for (let coin of symbols) {
		const raw = await fetchData(coin.pair, "4h", 1000);
		for(let i=0;i<raw.length;i++){
			let open = pctDiff(raw[i][1], raw[i+1][1]);
			let high = pctDiff(raw[i][2], raw[i+1][2]);
			let low = pctDiff(raw[i][3], raw[i+1][3]);
			let close = pctDiff(raw[i][4], raw[i+1][4]);
		}
	}
}
}
async function test3_v2() {
  function pctDiff(base, value) {
    return ((value - base) / base) * 100;
  }

  async function bm() {
    const map = [];

    for (let coin of symbols) {
      const raw = await fetchData(coin.pair, "4h", 1000);

      // raw[i] = [timestamp, open, high, low, close, volume]
      for (let i = 0; i < raw.length - 4; i++) {
        const baseOpen = raw[i][1]; // open of the first candle

        // Build 3 input candles relative to baseOpen
        const inputCandles = [];
        for (let j = 0; j < 3; j++) {
          const c = raw[i + j];
          inputCandles.push({
            open: pctDiff(baseOpen, c[1]) - pctDiff(baseOpen, baseOpen), // always 0 for first candle
            high: pctDiff(baseOpen, c[2]),
            low: pctDiff(baseOpen, c[3]),
            close: pctDiff(baseOpen, c[4])
          });
        }

        // Output candle = the next one after the 3 inputs
        const out = raw[i + 3];
		  const outOpen = out[1];

		  const outputCandle = {
			  open: 0, // always 0
			  high: pctDiff(outOpen, out[2]),
			  low: pctDiff(outOpen, out[3]),
			  close: pctDiff(outOpen, out[4])
		  };


        // Store mapping
        map.push({
          coin: coin.pair,
          input: inputCandles,
          output: outputCandle
        });
      }
    }

    return map;
  }

  var mp=await bm();
  console.log(mp[0]);
}
async function test3_v3() {
  function pctDiff(base, value) {
    return ((value - base) / base) * 100;
  }

  // helper: check if two 3‑candle arrays are "similar" within tolerance
  function isSimilar(a, b, tol = 0.1) {
    for (let i = 0; i < a.length; i++) {
      for (let key of ["open", "high", "low", "close"]) {
        if (Math.abs(a[i][key] - b[i][key]) > tol) return false;
      }
    }
    return true;
  }

  // helper: median of numeric array
  function median(arr) {
    if (!arr.length) return 0;
    const sorted = [...arr].sort((x, y) => x - y);
    const mid = Math.floor(sorted.length / 2);
    return sorted.length % 2 !== 0
      ? sorted[mid]
      : (sorted[mid - 1] + sorted[mid]) / 2;
  }

  async function bm() {
    const groups = [];

    for (let coin of symbols) {
      const raw = await fetchData(coin.pair, "1d", 1000);

      for (let i = 0; i < raw.length - 3; i++) {
        const baseOpen = raw[i][1];

        // build 3 input candles relative to baseOpen
        const inputCandles = [];
        for (let j = 0; j < 2; j++) {
          const c = raw[i + j];
          inputCandles.push({
            open: pctDiff(baseOpen, c[1]) - pctDiff(baseOpen, baseOpen), // always 0 for first candle
            high: pctDiff(baseOpen, c[2]),
            low: pctDiff(baseOpen, c[3]),
            close: pctDiff(baseOpen, c[4])
          });
        }

        // output candle relative to its own open
        const out = raw[i + 2];
        const outOpen = out[1];
        const outputCandle = {
          open: 0,
          high: pctDiff(outOpen, out[2]),
          low: pctDiff(outOpen, out[3]),
          close: pctDiff(outOpen, out[4])
        };

        // try to find an existing group
        let group = groups.find(g => isSimilar(g.input, inputCandles, 0.6));

        if (!group) {
          // create new group
          group = {
            input: inputCandles,
            outputs: [] // store all outputs for median calc
          };
          groups.push(group);
        }

        group.outputs.push(outputCandle);
      }
    }

    // finalize groups: compute median output + count
    return groups.map(g => {
      return {
        input: g.input,
        output: {
          open: 0,
          high: median(g.outputs.map(o => o.high)),
          low: median(g.outputs.map(o => o.low)),
          close: median(g.outputs.map(o => o.close))
        },
        count: g.outputs.length
      };
    });
  }

    // 🔑 new function: match last 3 candles
  async function matchLast() {
    const groups = await bm();

    // take last 3 candles from the *first coin* (adjust if you want all coins)
    const raw = await fetchData(symbols[1].pair, "1d", 10);
    const lastBase = raw[raw.length - 3][1];
    const last3 = [];
    for (let j = 0; j < 2; j++) {
      const c = raw[raw.length - 2 + j];
      last3.push({
            open: pctDiff(lastBase, c[1]) - pctDiff(lastBase, lastBase), // always 0 for first candle
            high: pctDiff(lastBase, c[2]),
            low: pctDiff(lastBase, c[3]),
            close: pctDiff(lastBase, c[4])
      });
    }

  console.log(groups.length);
    // find matching group
    const match = groups.find(g => isSimilar(g.input, last3, 0.4));
    if (!match) return null;

    return {
      count: match.count,
      output: match.output
    };
  }

  const result = await matchLast();
  console.log("Match result:", result);

//   const grouped = await bm();
//   console.log(grouped.length);
//   for(let i=0;i<grouped.length;i++){
//   console.log(grouped.length,grouped[i]); // example group
//   await sleepms(2000);
//   }
//   return grouped;
}
async function test3_v4() {
  function pctDiff(base, value) {
    return ((value - base) / base) * 100;
  }

  // helper: check if two N‑candle arrays are "similar" within tolerance
  function isSimilar(a, b, tol = 0.1) {
    if (a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) {
      for (let key of ["open", "high", "low", "close"]) {
        if (Math.abs(a[i][key] - b[i][key]) > tol) return false;
      }
    }
    return true;
  }

  function median(arr) {
    if (!arr.length) return 0;
    const sorted = [...arr].sort((x, y) => x - y);
    const mid = Math.floor(sorted.length / 2);
    return sorted.length % 2 !== 0
      ? sorted[mid]
      : (sorted[mid - 1] + sorted[mid]) / 2;
  }

  async function bm() {
    const groups = [];

    for (let coin of symbols) {
      const raw = await fetchData(coin.pair, "1d", 1000);

      for (let i = 0; i < raw.length - 3; i++) {
        const baseOpen = raw[i][1];

        // build 2 input candles relative to baseOpen
        const inputCandles = [];
        for (let j = 0; j < 2; j++) {
          const c = raw[i + j];
          inputCandles.push({
            open: 0,
            high: pctDiff(baseOpen, c[2]),
            low: pctDiff(baseOpen, c[3]),
            close: pctDiff(baseOpen, c[4])
          });
        }

        // output candle relative to its own open (the next one after the 2 inputs)
        const out = raw[i + 2];
        const outOpen = out[1];
        const outputCandle = {
          open: 0,
          high: pctDiff(outOpen, out[2]),
          low: pctDiff(outOpen, out[3]),
          close: pctDiff(outOpen, out[4])
        };

        // try to find an existing group
        let group = groups.find(g => isSimilar(g.input, inputCandles, 0.4));
        if (!group) {
          group = { input: inputCandles, outputs: [] };
          groups.push(group);
        }
        group.outputs.push(outputCandle);
      }
    }

    // finalize groups
    return groups.map(g => ({
      input: g.input,
      output: {
        open: 0,
        high: median(g.outputs.map(o => o.high)),
        low: median(g.outputs.map(o => o.low)),
        close: median(g.outputs.map(o => o.close))
      },
      count: g.outputs.length
    }));
  }

  async function matchLast() {
    const groups = await bm();

    // take last 2 candles from the chosen coin
    const raw = await fetchData(symbols[1].pair, "1d", 10);
    const lastBase = raw[raw.length - 3][1];
    const last2 = [];
    for (let j = 0; j < 2; j++) {
      const c = raw[raw.length - 3 + j];
      last2.push({
        open: 0,
        high: pctDiff(lastBase, c[2]),
        low: pctDiff(lastBase, c[3]),
        close: pctDiff(lastBase, c[4])
      });
    }

    const match = groups.find(g => isSimilar(g.input, last2, 0.4));
    if (!match) return null;

    return {
      count: match.count,
      output: match.output
    };
  }

  const result = await matchLast();
  console.log("Match result:", result);
}
async function test3_v5() {
  function pctDiff(base, value) {
    return ((value - base) / base) * 100;
  }

  // compare 2-candle sequences
  function isSimilar(a, b, tol = 0.1) {
    if (a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) {
      for (let key of ["open", "high", "low", "close"]) {
        if (Math.abs(a[i][key] - b[i][key]) > tol) return false;
      }
    }
    return true;
  }

  function median(arr) {
    if (!arr.length) return 0;
    const sorted = [...arr].sort((x, y) => x - y);
    const mid = Math.floor(sorted.length / 2);
    return sorted.length % 2 !== 0
      ? sorted[mid]
      : (sorted[mid - 1] + sorted[mid]) / 2;
  }

  async function bm() {
    const groups = [];

    for (let coin of symbols) {
      const raw = await fetchData(coin.pair, "1d", 1000);

      for (let i = 0; i < raw.length - 2; i++) {
        const baseOpen = raw[i][1];

        // build 2 input candles relative to the first open
        const inputCandles = [];
        for (let j = 0; j < 2; j++) {
          const c = raw[i + j];
          inputCandles.push({
            open: pctDiff(baseOpen, c[1]),
            high: pctDiff(baseOpen, c[2]),
            low: pctDiff(baseOpen, c[3]),
            close: pctDiff(baseOpen, c[4])
          });
        }

        // output = the candle *after* those 2
        const out = raw[i + 2];
        const outOpen = out[1];
        const outputCandle = {
          open: 0,
          high: pctDiff(outOpen, out[2]),
          low: pctDiff(outOpen, out[3]),
          close: pctDiff(outOpen, out[4])
        };

        let group = groups.find(g => isSimilar(g.input, inputCandles, 0.5));

        if (!group) {
          group = { input: inputCandles, outputs: [] };
          groups.push(group);
        }
        group.outputs.push(outputCandle);
      }
    }

    return groups.map(g => ({
      input: g.input,
      output: {
        open: 0,
        high: median(g.outputs.map(o => o.high)),
        low: median(g.outputs.map(o => o.low)),
        close: median(g.outputs.map(o => o.close))
      },
      count: g.outputs.length
    }));
  }

  async function matchLast() {
    const groups = await bm();

    // get last 2 candles from the coin
    const raw = await fetchData(symbols[1].pair, "1d", 10);
    const lastBase = raw[raw.length - 3][1];

    const last2 = [];
    for (let j = 0; j < 2; j++) {
      const c = raw[raw.length - 3 + j];
      last2.push({
        open: pctDiff(lastBase, c[1]),
        high: pctDiff(lastBase, c[2]),
        low: pctDiff(lastBase, c[3]),
        close: pctDiff(lastBase, c[4])
      });
    }

    console.log("Groups:", groups.length);
    const match = groups.find(g => isSimilar(g.input, last2, 0.3));
    if (!match) return null;

    return {
      count: match.count,
      output: match.output
    };
  }

  const result = await matchLast();
  console.log("Match result:", result);
}
async function test3_v8goodess12() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			//   for (let key of [ "close"]) {
			for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.33;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 4 * 7 * 4;
	let slsalt=0.2;
	let tpsalt=0.18;
	const atleast=0.2;
	const tf = "6h";
	const gtol = 0.79995;
	const wsize = 4;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		for (let si = 0; si < symbols.length; si++) {
			// for (let coin of symbols) {
			let coin = symbols[si];
			const raw = await fetchData(coin.pair, tf, 1000);
			symbols[si].raw = raw;
			// console.log("rl",raw.length,raw.length - (wsize + histminus));
			for (let i = 0; i < raw.length - (wsize + histminus); i++) {
				const baseOpen = raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;///////////////////////////////////////////////////////////
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

		let flagbreak = 0;
		pnlr = 0;

		for (let syi = 0; syi < symbols.length; syi++) {
			let coin = symbols[syi];
			symbols[syi].test=8;
			// for (let coin of symbols) {
			if (coin.trade == 0) continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

			if (res.count == 1) {
				flagbreak = 1; continue;
				// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
			}
			if (Math.abs(res.output.close) < 0.01) {
				flagbreak = 1; continue;
				// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
			}


			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt = `${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) {
					pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					// if (dt == "2025-09-18 06") console.log("Match result:", res);
					if (lb == 0 && cw >= 0) {
						coin.cw=cw;
						coin.side = cw == 1 ? "buy" : "sell";
						coin.entry = res.nopen;
						// entry=res.nclose;
						coin.sli = sli;
						coin.tpi = tpi;
						console.log("Match result:", res);
						console.log("cw:", cw, side);

					}
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if (flagbreak == 1) continue;
		console.log(dt, "pnltt:", Math_Round(pnltt, 0), "pnlr:", Math_Round(pnlr, 0), "equity:", Math_Round(equity, 0));
	}
}

let equitytt=1000;
async function test5() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			//   for (let key of [ "close"]) {
			for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.33;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 4 * 7 * 4;
	let slsalt=0.2;
	let tpsalt=0.18;
	const atleast=0.2;
	const tf = "6h";
	const gtol = 0.79995;
	const wsize = 4;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		for (let si = 0; si < symbols.length; si++) {
			// for (let coin of symbols) {
			let coin = symbols[si];
			const raw = await fetchData(coin.pair, tf, 1000);
			symbols[si].raw = raw;
			// console.log("rl",raw.length,raw.length - (wsize + histminus));
			for (let i = 0; i < raw.length - (wsize + histminus); i++) {
				const baseOpen = raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;///////////////////////////////////////////////////////////
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

	let flagbreak=0;
		pnlr=0;

		for (let coin of symbols) {
			if(coin.trade==0)continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

if (res.count==1){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}
if (Math.abs(res.output.close)<0.01){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}


			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt=`${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) 
					{
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					if(dt=="2025-09-18 06") console.log("Match result:", res);
					if(lb==0 && cw>=0){
						side=cw==1?"buy":"sell";
						entry=res.nopen;
						// entry=res.nclose;
						//  console.log("Match result:", res);
						//  console.log("cw:", cw,side);

					}
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if(flagbreak==1) continue;
		console.log(dt,"pnltt:", Math_Round(pnltt,0),"pnlr:", Math_Round(pnlr,0), "equity:", Math_Round(equity,0));
	}
	equitytt=equity;
}

async function test3_v8goodess12_v2() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			  for (let key of [ "close"]) {
			// for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.1;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 6 * 7 * 3;
	let slsalt=0.8;
	let tpsalt=0.35;
	const atleast=0.15;
	const tf = "4h";
	const gtol = 2.2;
	const wsize = 10;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		for (let si = 0; si < symbols.length; si++) {
			// for (let coin of symbols) {
			let coin = symbols[si];
			if(coin.raw==undefined)coin.raw = await fetchData_(coin.pair, tf, 500);
			// symbols[si].raw = raw;
			// console.log("rl",coin.raw.length,coin.raw.length - (wsize + histminus));
			for (let i = 0; i < coin.raw.length - (wsize + histminus); i++) {
				const baseOpen = coin.raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = coin.raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = coin.raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		// console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;///////////////////////////////////////////////////////////
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

	let flagbreak=0;
		pnlr=0;

		for (let coin of symbols) {
			if(coin.trade==0)continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

if (res.count==1){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}
if (Math.abs(res.output.close)<0.01){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}


			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt=`${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) 
					{
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					if(dt=="2025-09-18 06") console.log("Match result:", res);
					if(lb==0 && cw>=0){
						side=cw==1?"buy":"sell";
						entry=res.nopen;
						// entry=res.nclose;
						//  console.log("Match result:", res);
						//  console.log("cw:", cw,side);

					}
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if(flagbreak==1) continue;
		// console.log(dt,"pnltt:", Math_Round(pnltt,0),"pnlr:", Math_Round(pnlr,0), "equity:", Math_Round(equity,0));
	}
	equitytt=equity;
}
async function test3_v8goodess12_v0() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			//   for (let key of [ "close"]) {
			for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.33;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 4 * 7 * 4;
	let slsalt=0.2;
	let tpsalt=0.18;
	const atleast=0.2;
	const tf = "6h";
	const gtol = 0.79995;
	const wsize = 4;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		for (let si = 0; si < symbols.length; si++) {
			// for (let coin of symbols) {
			let coin = symbols[si];
			const raw = await fetchData(coin.pair, tf, 1000);
			symbols[si].raw = raw;
			// console.log("rl",raw.length,raw.length - (wsize + histminus));
			for (let i = 0; i < raw.length - (wsize + histminus); i++) {
				const baseOpen = raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;///////////////////////////////////////////////////////////
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

	let flagbreak=0;
		pnlr=0;

		for (let coin of symbols) {
			if(coin.trade==0)continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

if (res.count==1){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}
if (Math.abs(res.output.close)<0.01){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}


			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt=`${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) 
					{
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					if(dt=="2025-09-18 06") console.log("Match result:", res);
					if(lb==0 && cw>=0){
						side=cw==1?"buy":"sell";
						entry=res.nopen;
						// entry=res.nclose;
						 console.log("Match result:", res);
						 console.log("cw:", cw,side);

					}
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if(flagbreak==1) continue;
		console.log(dt,"pnltt:", Math_Round(pnltt,0),"pnlr:", Math_Round(pnlr,0), "equity:", Math_Round(equity,0));
	}
}

async function test3_v8goodess12_v4() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			//   for (let key of [ "close"]) {
			for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.33;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 4 * 7 * 4;
	let slsalt=0.2;
	let tpsalt=0.18;
	const atleast=0.2;
	const tf = "6h";
	const gtol = 0.79995;
	const wsize = 4;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		// for (let si = 0; si < symbols.length; si++) {
		for (let coin of symbols) {
			// for (let coin of symbols) {
			// let coin = symbols[si];
			// const raw = await fetchData(coin.pair, tf, 1000);
			// symbols[si].raw = raw;
			if(!coin.raw)coin.raw=fetchData(coin.pair, tf, 1000);
			// console.log("rl",raw.length,raw.length - (wsize + histminus));
			for (let i = 0; i < coin.raw.length - (wsize + histminus); i++) {
				const baseOpen = coin.raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = coin.raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = coin.raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		// console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined){
			coin.rawl=coin.raw.slice(-200);
		}
		// if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		// let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;///////////////////////////////////////////////////////////
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

	let flagbreak=0;
		pnlr=0;

		for (let coin of symbols) {
			if(coin.trade==0)continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

		if (res.count==1){
			flagbreak=1; continue;
			// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
		}
		if (Math.abs(res.output.close)<0.01){
			flagbreak=1; continue;
			// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
		}


			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt=`${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) 
					{
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					if(dt=="2025-09-18 06") console.log("Match result:", res);
					if(lb==0 && cw>=0){
						side=cw==1?"buy":"sell";
						entry=res.nopen;
						// entry=res.nclose;
						//  console.log("Match result:", res);
						//  console.log("cw:", cw,side);

					}
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if(flagbreak==1) continue;
		// console.log(dt,"pnltt:", Math_Round(pnltt,0),"pnlr:", Math_Round(pnlr,0), "equity:", Math_Round(equity,0));
	}
	equitytt=equity;
}

async function backtestSymbols(symbols, {
  tf = "6h",
  wsize = 4,
  histminus = 4 * 7 * 1,
  tol = 1.8,
  slSalt = 0.2,
  tpSalt = -0.18,
  atleast = 0.2,
  startingEquity = 1000
} = {}) {

  // --- Helpers ---
  const pctDiff = (base, value) => ((value - base) / base) * 100;

  const median = arr => {
    if (!arr.length) return 0;
    const s = [...arr].sort((a, b) => a - b);
    const m = Math.floor(s.length / 2);
    return s.length % 2 ? s[m] : (s[m - 1] + s[m]) / 2;
  };

  const isSimilar = (a, b, tolerance = tol) => {
    if (a.length !== b.length) return false;
    const keys = ["high", "low", "close"];
    let sumSq = 0;
    for (let i = 0; i < a.length; i++) {
      for (let k of keys) {
        sumSq += (a[i][k] - b[i][k]) ** 2;
      }
    }
    const dist = Math.sqrt(sumSq) / Math.sqrt(a.length * keys.length);
    return dist < tolerance;
  };

  // --- Load raw data ---
  for (const coin of symbols) {
    if (!coin.raw) {
      coin.raw = await fetchData_(coin.pair, tf, 1000);
    //   coin.raw = await fetchData_(coin.pair, tf, wsize + histminus + 1);
    }
    coin.equity = startingEquity;   // per‑coin equity
    coin.trades = [];               // trade history
    coin.curve = [];                // equity curve over time
  }

  // --- Build groups ---
  const groups = [];
  for (const coin of symbols) {
    const raw = coin.raw;
    for (let i = 0; i < raw.length - (wsize + histminus); i++) {
      const baseOpen = raw[i][1];
      const input = raw.slice(i, i + wsize).map(c => ({
        open: pctDiff(baseOpen, c[1]),
        high: pctDiff(baseOpen, c[2]),
        low: pctDiff(baseOpen, c[3]),
        close: pctDiff(baseOpen, c[4])
      }));
      const out = raw[i + wsize];
      const output = {
        open: 0,
        high: pctDiff(out[1], out[2]),
        low: pctDiff(out[1], out[3]),
        close: pctDiff(out[1], out[4])
      };
      let g = groups.find(g => isSimilar(g.input, input));
      if (!g) {
        g = { input, outputs: [] };
        groups.push(g);
      }
      g.outputs.push(output);
    }
  }

  const reducedGroups = groups.map(g => ({
    input: g.input,
    count: g.outputs.length,
    output: {
      open: 0,
      high: median(g.outputs.map(o => o.high)),
      low: median(g.outputs.map(o => o.low)),
      close: median(g.outputs.map(o => o.close))
    }
  }));

  // --- Match function ---
  function matchLast(coin, lb) {
    const raw = coin.raw.slice(0, -lb || undefined);
    if (raw.length < wsize + 1) return null;

    const baseOpen = raw[raw.length - (wsize + 1)][1];
    const input = raw.slice(raw.length - (wsize + 1), raw.length - 1).map(c => ({
      open: pctDiff(baseOpen, c[1]),
      high: pctDiff(baseOpen, c[2]),
      low: pctDiff(baseOpen, c[3]),
      close: pctDiff(baseOpen, c[4]),
      rclose: c[4]
    }));

    const grp = reducedGroups.find(g => isSimilar(g.input, input));
    if (!grp) return null;

    const nc = raw.at(-1);
    return {
      symbol: coin.pair,
      count: grp.count,
      output: grp.output,
      ndate: nc[0],
      nopen: nc[1],
      nhigh: nc[2],
      nlow: nc[3],
      nclose: nc[4]
    };
  }

  // --- Backtest loop ---
  const portfolioCurve = [];

  for (let lb = histminus; lb >= 0; lb--) {
    for (const coin of symbols.filter(c => c.trade)) {
      const res = matchLast(coin, lb);
      if (!res || res.count <= 1 || Math.abs(res.output.close) < atleast) continue;

      // Direction
      let side, sli, tpi;
      if (res.output.close >= atleast) {
        side = "buy";
        sli = Math.abs(res.output.low) + slSalt;
        tpi = Math.abs(res.output.high) - tpSalt;
      } else {
        side = "sell";
        sli = Math.abs(res.output.high) + slSalt;
        tpi = Math.abs(res.output.low) - tpSalt;
      }

      // Simulate trade
      const pnl = simulate_order(coin.equity, res.nopen, res.nhigh, res.nlow, res.nclose, side === "buy" ? 1 : 0);
      coin.equity += pnl;
      coin.trades.push({ date: new Date(res.ndate).toISOString(), side, entry: res.nopen, pnl, equity: coin.equity });
    }

    // record portfolio snapshot at this lb
    const snapshotDate = symbols[0].raw.at(-lb || -1)[0];
    const portfolioEquity = symbols.reduce((sum, c) => sum + c.equity, 0);
    portfolioCurve.push({ date: new Date(snapshotDate).toISOString(), equity: portfolioEquity });

    // record per‑coin equity curves
    for (const coin of symbols) {
      coin.curve.push({ date: new Date(snapshotDate).toISOString(), equity: coin.equity });
    }
  }

  // --- Results ---
  console.table(symbols.map(c => ({
    Symbol: c.pair,
    FinalEquity: c.equity.toFixed(2),
    Trades: c.trades.length
  })));

  console.log("Portfolio equity:", portfolioCurve.at(-1).equity.toFixed(2));

  return { symbols, portfolioCurve };
}

async function test3_v8goodess12_v1() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			distance += Math.pow(a[i].close - b[i].close, 2);
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = arr.slice().sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	const riskFrac = 0.1;
	const histminus = 6 * 7 * 1;
	const tf = "4h";
	const gtol = 0.3;
	const wsize = 5;
	const atleast = 0.15;
	let slsalt = 0.51, tpsalt = -0.45;
	let lb = 1;

	// ---- Build groups ----
	async function bm() {
		const groups = [];
		for (let coin of symbols) {
			if (!coin.raw){
				coin.raw = await fetchData(coin.pair, tf, 1000);
				coin.raw=coin.raw.slice(-500,-6);
			} 

			for (let i = 0; i < coin.raw.length - (wsize + histminus); i++) {
				const baseOpen = coin.raw[i][1];
				const inputCandles = coin.raw.slice(i, i + wsize).map(c => ({
					open: pctDiff(baseOpen, c[1]),
					high: pctDiff(baseOpen, c[2]),
					low: pctDiff(baseOpen, c[3]),
					close: pctDiff(baseOpen, c[4])
				}));

				const out = coin.raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};

				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}

		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	// ---- Match last ----
	async function matchLast(coin) {
		if (!coin.rawl) coin.rawl = coin.raw.slice(-histminus*2);
		// if (!coin.rawl) coin.rawl = await fetchData_(coin.pair, tf, 200);
		let raw = coin.rawl;
		const nc = raw[raw.length - lb - 1];
		if (lb > 0) raw = raw.slice(0, -lb);

		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = raw.slice(raw.length - (wsize + 1), raw.length - 1).map(c => ({
			open: pctDiff(lastBase, c[1]),
			high: pctDiff(lastBase, c[2]),
			low: pctDiff(lastBase, c[3]),
			close: pctDiff(lastBase, c[4]),
			rclose: c[4],
		}));

		const match = groups.find(g => isSimilar(g.input, last2, gtol));
		if (!match) return null;

		return {
			symbol: coin.pair,
			count: match.count,
			output: match.output,
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}

	// ---- Backtest loop ----
	let pnltt = 0, equity = 1000, pnlr = 0, dt = "";

	for (lb = histminus; lb >= 0; lb--) {
		let flagbreak = false;
		pnlr = 0;

		for (let coin of symbols) {
			if (!coin.trade) continue;
			const res = await matchLast(coin);
			if (!res) continue;

			// ignorar se poucos dados ou movimento irrelevante
			if (res.count === 1 || Math.abs(res.output.close) < 0.01) {
				flagbreak = true; 
				continue;
			}

			// ---- Definição de direção ----
			let cw = -1, sli, tpi;
			const { close, high, low } = res.output;

			if (close >= atleast || (close > -atleast && high >= Math.abs(low))) {
				cw = 1;
				sli = Math.abs(low) + slsalt;
				tpi = Math.abs(high) - tpsalt;
			} else if (close <= -atleast || (close < atleast && high < Math.abs(low))) {
				cw = 0;
				sli = Math.abs(high) + slsalt;
				tpi = Math.abs(low) - tpsalt;
			}

			// ---- PnL ----
			if (cw >= 0) {
				dt = `${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				let pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnltt += pnlr;
				equity += pnl;

				if (dt === "2025-09-18 06") console.log("Match result:", res);
				if (lb === 0) {
					const side = cw ? "buy" : "sell";
					entry = res.nopen;
					// console.log("Match result:", res, side);
				}
			}
		}
		if (flagbreak) continue;
	}

	equitytt = equity;
}

async function test3() {
	function pctDiff(base, value) {
		return ((value - base) / base) * 100;
	}

	// New similarity function using Euclidean distance
	function isSimilar(a, b, tol = 0.1) {
		if (a.length !== b.length) return false;
		let distance = 0;
		for (let i = 0; i < a.length; i++) {
			//   for (let key of [ "close"]) {
			for (let key of ["high", "low", "close"]) {
				//   for (let key of [ "low"]) {
				//   for (let key of [ "high"]) {
				//   for (let key of [ "high", "low"]) {
				//   for (let key of ["open", "high", "low", "close"]) {
				distance += Math.pow(a[i][key] - b[i][key], 2);
			}
		}
		distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
		return distance < tol;
	}

	function median(arr) {
		if (!arr.length) return 0;
		const sorted = [...arr].sort((x, y) => x - y);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 !== 0
			? sorted[mid]
			: (sorted[mid - 1] + sorted[mid]) / 2;
	}

	riskFrac = 0.33;
	//   const tf="4h";
	//   const gtol=0.4;
	//   const wsize=5; 
	// const histminus = 4 *7;
	const histminus = 6 * 7 * 2;
	let slsalt=0.2;
	let tpsalt=0.18;
	const atleast=0.6;
	const tf = "4h";
	const gtol = 1.0;
	const wsize = 3 ;
	let lb = 1;//////////////////////////////////////////////////////////////////////////////////////

	async function bm() {
		const groups = [];
		for (let si = 0; si < symbols.length; si++) {
			// for (let coin of symbols) {
			let coin = symbols[si];
			const raw = await fetchData(coin.pair, tf, 1000);
			symbols[si].raw = raw;
			// console.log("rl",raw.length,raw.length - (wsize + histminus));
			for (let i = 0; i < raw.length - (wsize + histminus); i++) {
				const baseOpen = raw[i][1];
				// build 2 input candles relative to the first open
				const inputCandles = [];
				for (let j = 0; j < wsize; j++) {
					const c = raw[i + j];
					inputCandles.push({
						open: pctDiff(baseOpen, c[1]),
						high: pctDiff(baseOpen, c[2]),
						low: pctDiff(baseOpen, c[3]),
						close: pctDiff(baseOpen, c[4])
					});
				}
				// output = the candle *after* those 2
				const out = raw[i + wsize];
				const outOpen = out[1];
				const outputCandle = {
					open: 0,
					high: pctDiff(outOpen, out[2]),
					low: pctDiff(outOpen, out[3]),
					close: pctDiff(outOpen, out[4])
				};
				// Try to find a similar group with increased tolerance
				let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
				if (!group) {
					group = { input: inputCandles, outputs: [] };
					groups.push(group);
				}
				group.outputs.push(outputCandle);
			}
		}
		// Log group sizes for debugging
		console.log("Group sizes:", groups.map(g => g.outputs.length));
		return groups.map(g => ({
			input: g.input,
			output: {
				open: 0,
				high: median(g.outputs.map(o => o.high)),
				low: median(g.outputs.map(o => o.low)),
				close: median(g.outputs.map(o => o.close))
			},
			count: g.outputs.length
		}));
	}

	const groups = await bm();

	async function matchLast(coin) {
		const symbpair = coin.pair;
		// get last 2 candles from the coin
		if(coin.rawl==undefined)coin.rawl=await fetchData_(symbpair, tf, 200);
		let raw = coin.rawl;

		// let raw = await fetchData(symbpair, tf, 100);
		// let raw = coin.raw;
		// let lb=32;
		const nc = raw[raw.length - lb - 1];
		if(lb>0)raw = raw.slice(0, -lb);
		//   console.log(coin.raw.length,raw.length);
		const lastBase = raw[raw.length - (wsize + 1)][1];
		const last2 = [];
		for (let j = 0; j < wsize; j++) {
			const c = raw[raw.length - (wsize + 1) + j];
			last2.push({
				open: pctDiff(lastBase, c[1]),
				high: pctDiff(lastBase, c[2]),
				low: pctDiff(lastBase, c[3]),
				close: pctDiff(lastBase, c[4]),
				rclose: c[4],
			});
		}
		// console.log("Groups:", groups.length,last2);
		// Increased tolerance for matching
		const match = groups.find(g => isSimilar(g.input, last2, gtol - 0.0));
		if (!match) return null;
		return {
			symbol: symbpair,
			count: match.count,
			output: match.output,
			//   input: last2
			close: last2[last2.length - 1].rclose,
			ndate: nc[0],
			nopen: nc[1],
			nhigh: nc[2],
			nlow: nc[3],
			nclose: nc[4],
			nhighper: pctDiff(nc[1], nc[2]),
			nlowper: pctDiff(nc[1], nc[3]),
			ncloseper: pctDiff(nc[1], nc[4]),
		};
	}
	let pnltt = 0;
	let pnlr = 0;
	let dt="";
	let equity = 1000;

	// for(lb=1;lb<histminus;lb++){
	for (lb = histminus; lb >= 0; lb--) {

	let flagbreak=0;
		pnlr=0;

		for (let coin of symbols) {
			if(coin.trade==0)continue;
			const res = await matchLast(coin);
			if (!res) continue;

			let cw = -1;

if (res.count<=3){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}
if (Math.abs(res.output.close)<0.01){
	flagbreak=1; continue;
	// if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.029)continue;
}
if(Math.abs(Math.abs(res.output.high)-Math.abs(res.output.low))<0.03)continue;



			if (res.output.close >= atleast) {
				cw = 1;
				sli = Math.abs(res.output.low) + slsalt;
				tpi = Math.abs(res.output.high) - tpsalt;
				// if(sli<1)sli=1;
			}
			else if (res.output.close <= -atleast) {
				cw = 0;
				sli = Math.abs(res.output.high) + slsalt;
				tpi = Math.abs(res.output.low) - tpsalt;
				// if(sli<1)sli=1;
			}
			else 
			if (res.output.close > -atleast && res.output.close < atleast) {
				if (res.output.high >= Math.abs(res.output.low)) {
					cw = 1;
					sli = Math.abs(res.output.low) + slsalt;
					tpi = Math.abs(res.output.high) - tpsalt;
					// if(sli<1)sli=1;
				} else {
					cw = 0;
					sli = Math.abs(res.output.high) + slsalt;
					tpi = Math.abs(res.output.low) - tpsalt;
					// if(sli<1)sli=1;
				}
			}
			let pnl = 0;

			if (lb >= 0) {
				dt=`${new Date(res.ndate).toISOString().split('T')[0]} ${String(new Date(res.ndate).getUTCHours()).padStart(2, '0')}`;

				if (coin.trade == 1) 
					{
				pnlr = simulate_order(100, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
				pnl = simulate_order(equity, res.nopen, res.nhigh, res.nlow, res.nclose, cw);
					pnltt += pnlr;
					equity += pnl;

					// if(dt=="2025-09-18 06") console.log("Match result:", res);
					// if(pnlr<-30)console.log("Match result:", res);
					// if(lb==0) console.log("Match result:", res);
				}
			}
			// console.log("pnl:",pnl,"pnltt:",pnltt);
			// console.log("pnlr:",pnl);

		}
		if(flagbreak==1) continue;
		console.log(dt,"pnltt:", Math_Round(pnltt,0),"pnlr:", Math_Round(pnlr,0), "equity:", Math_Round(equity,0));
	}
}
async function test3_v7() {
  function pctDiff(base, value) {
    return ((value - base) / base) * 100;
  }
  // New similarity function using Euclidean distance
  function isSimilar(a, b, tol = 0.1) {
    if (a.length !== b.length) return false;
    let distance = 0;
    for (let i = 0; i < a.length; i++) {
      for (let key of ["close"]) {
    //   for (let key of ["open", "high", "low", "close"]) {
        distance += Math.pow(a[i][key] - b[i][key], 2);
      }
    }
    distance = Math.sqrt(distance) / Math.sqrt(a.length * Object.keys(a[0]).length);
    return distance < tol;
  }
  function median(arr) {
    if (!arr.length) return 0;
    const sorted = [...arr].sort((x, y) => x - y);
    const mid = Math.floor(sorted.length / 2);
    return sorted.length % 2 !== 0
      ? sorted[mid]
      : (sorted[mid - 1] + sorted[mid]) / 2;
  }
  const tf="6h";
  const gtol=0.1;
  const wsize=4; 
  async function bm() {
    const groups = [];
    for (let coin of symbols) {
      const raw = await fetchData(coin.pair, tf, 1000);
      for (let i = 0; i < raw.length - wsize; i++) { // Updated loop condition
        const baseOpen = raw[i][1];
        // build 2 input candles relative to the first open
        const inputCandles = [];
        for (let j = 0; j < wsize; j++) {
          const c = raw[i + j];
          inputCandles.push({
            open: pctDiff(baseOpen, c[1]),
            high: pctDiff(baseOpen, c[2]),
            low: pctDiff(baseOpen, c[3]),
            close: pctDiff(baseOpen, c[4])
          });
        }
        // output = the candle *after* those 2
        const out = raw[i + wsize];
        const outputCandle = {
          open: pctDiff(baseOpen, out[1]), // Updated to use baseOpen
          high: pctDiff(baseOpen, out[2]),
          low: pctDiff(baseOpen, out[3]),
          close: pctDiff(baseOpen, out[4])
        };
        // Try to find a similar group with increased tolerance
        let group = groups.find(g => isSimilar(g.input, inputCandles, gtol));
        if (!group) {
          group = { input: inputCandles, outputs: [] };
          groups.push(group);
        }
        group.outputs.push(outputCandle);
      }
    }
    // Log group sizes for debugging
    console.log("Group sizes:", groups.map(g => g.outputs.length));
    return groups.map(g => ({
      input: g.input,
      output: {
        open: median(g.outputs.map(o => o.open)),
        high: median(g.outputs.map(o => o.high)),
        low: median(g.outputs.map(o => o.low)),
        close: median(g.outputs.map(o => o.close))
      },
      count: g.outputs.length
    }));
  }
    const groups = await bm();
  async function matchLast(symbpair) {
    // get last 2 candles from the coin
    let raw = await fetchData(symbpair, tf, 20);
	  raw=raw.slice(0,-5);
    const lastBase = raw[raw.length - (wsize+1)][1];
    const last2 = [];
    for (let j = 0; j < wsize; j++) {
      const c = raw[raw.length - (wsize+1) + j];
      last2.push({
        open: pctDiff(lastBase, c[1]),
        high: pctDiff(lastBase, c[2]),
        low: pctDiff(lastBase, c[3]),
        close: pctDiff(lastBase, c[4])
      });
    }
    // console.log("Groups:", groups.length,last2);
    // Increased tolerance for matching
    const match = groups.find(g => isSimilar(g.input, last2, gtol-0.0));
    if (!match) return null;
    return {
	  symbol: symbpair,
      count: match.count,
      output: match.output,
      input: last2
    //   input: match.input
    };
  }
	for (let coin of symbols) {
		const result = await matchLast(coin.pair);
		console.log("Match result:", result);
	}
}



// #endregion

// #region Whitebit
const crypto = require('crypto');

const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET;  
 
var markets=0;
var oqty=0;


async function getmarkets() {
	if (markets == 0) {
		const marketsRes = await fetch('https://whitebit.com/api/v4/public/markets');
		markets = await marketsRes.json();
	}
}

async function getaskbid() {
	const res = await fetch('https://whitebit.com/api/v4/public/orderbook/ETH_PERP?limit=1&level=0');
	return await res.json();
}

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
// get_equity().then(total => console.log(total));

 
async function signandsend(bodyObj,endpoint='/api/v4/order/collateral/limit') {  
//   return;
  // 6️⃣ Sign & send (unchanged)
  const bodyStr = JSON.stringify(bodyObj);
  const payload = Buffer.from(bodyStr).toString('base64');
  const signature = crypto.createHmac('sha512', API_SECRET)
						  .update(payload)
						  .digest('hex');

  const res = await fetch('https://whitebit.com'+bodyObj.request, {
//   const res = await fetch('https://whitebit.com/api/v4/order/collateral/limit', {
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

async function open_order_v1(symbol, side, entry, slPerc, tpPerc, riskUSDT) {
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

  let qty = (riskUSDT / priceDiff).toFixed(stockPrec);
  const price = parseFloat(entry).toFixed(moneyPrec);

  // 4️⃣ Leverage calculations
  const leverageRisk = (entry / priceDiff).toFixed(2);
  const positionValue = entry * qty;
  const marginUsed = riskUSDT;
  const leverageExchange = (positionValue / marginUsed).toFixed(2);

  console.log("Leverage risco (stop):", leverageRisk);
  console.log("Leverage corretora:", leverageExchange);

  console.log(`📊 Qty: ${qty} | Entry: ${price} | SL: ${sl.toFixed(moneyPrec)} | TP: ${tp ? tp.toFixed(moneyPrec) : '—'} | Lev: ${leverageExchange}x`);

  //debug
//   qty=0.01;
  
//   if(side=="buy"){
// 	qty=Math.abs(oqty-qty).toFixed(stockPrec);
// 	console.log("qty",qty);
//   }
//   if(side=="sell"){
// 	qty=Math.abs(-qty-oqty).toFixed(stockPrec);
// 	console.log("qty",qty);
//   }


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

  await signandsend(bodyObj);

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
  for(let i=0;i<json.length;i++){
  oqty+=json[i].amount;
  }
  return json;
}













async function open_order(symbol, side, entry, slPerc, tpPerc, riskUSDT, oqty) {
  const m = markets.find(m => m.name === symbol);
  if (!m) throw new Error(`Mercado ${symbol} não encontrado`);
  const stockPrec = +m.stockPrec, moneyPrec = +m.moneyPrec;

  // --- Price targets from percentages ---
  const isBuy = side.toLowerCase() === 'buy';
  const sl = isBuy ? entry * (1 - slPerc / 100) : entry * (1 + slPerc / 100);
  const tp = tpPerc ? (isBuy ? entry * (1 + tpPerc / 100) : entry * (1 - tpPerc / 100)) : undefined;

  // --- Risk to target size (final intended position size) ---
  const priceDiff = Math.abs(entry - sl);
  if (priceDiff <= 0) throw new Error('Stop loss inválido');
  const targetQty = parseFloat((riskUSDT / priceDiff).toFixed(stockPrec));
  const price = parseFloat(entry).toFixed(moneyPrec);

  const absO = Math.abs(oqty);
  const absT = Math.abs(targetQty);

  // --- Action selection ---
  const currentDir = oqty > 0 ? 'long' : (oqty < 0 ? 'short' : 'flat');
  const targetDir  = isBuy ? 'long' : 'short';

  console.log(`🧭 Context | symbol=${symbol} | current=${currentDir} ${absO} | target=${targetDir} ${absT} | entry=${price} | SL=${sl.toFixed(moneyPrec)} | TP=${tp ? tp.toFixed(moneyPrec) : '—'}`);

  if (oqty === 0) {
	// Flat → open new
	await logAction('open', `flat → ${targetDir}`, { from: 0, to: absT, side });
	await sendOrder(symbol, side, absT, price);

  } else if ((oqty > 0 && isBuy) || (oqty < 0 && !isBuy)) {
	// Same direction (already aligned with targetDir)
	if (absT > absO) {
	  const inc = +(absT - absO).toFixed(stockPrec);
	  await logAction('increase', `${currentDir} aligned`, { from: absO, to: absT, delta: inc, side });
	  await sendOrder(symbol, side, inc, price);
	} else if (absT < absO) {
	  const dec = +(absO - absT).toFixed(stockPrec);
	  const decSide = oqty > 0 ? 'sell' : 'buy';
	  await logAction('decrease', `${currentDir} aligned`, { from: absO, to: absT, delta: dec, side: decSide });
	  await sendOrder(symbol, decSide, dec, price);
	} else {
	  await logAction('redefine', 'size equal', { size: absO, side });
	  // no size change
	}

  } else {
	// Opposite direction (reversal or partial close)
	if (absT > absO) {
	  // Full close + open reverse
	  const closeQty = absO;
	  const openQty  = +(absT - absO).toFixed(stockPrec);
	  const closeSide = oqty > 0 ? 'sell' : 'buy';

	  await logAction('reverse', `${currentDir} → ${targetDir}`, { close: closeQty, open: openQty, closeSide, openSide: side });
	  if (closeQty > 0) await sendOrder(symbol, closeSide, closeQty, price);
	  if (openQty > 0)  await sendOrder(symbol, side, openQty, price);

	} else if (absT < absO) {
	  // Partial close only
	  const closeQty = absT;
	  const closeSide = oqty > 0 ? 'sell' : 'buy';
	  await logAction('partial_close', `${currentDir} opp to target`, { from: absO, to: absT, delta: closeQty, side: closeSide });
	  await sendOrder(symbol, closeSide, closeQty, price);

	} else {
	  // Exact close
	  const closeQty = absO;
	  const closeSide = oqty > 0 ? 'sell' : 'buy';
	  await logAction('close', `${currentDir} → flat`, { size: closeQty, side: closeSide });
	  await sendOrder(symbol, closeSide, closeQty, price);
	}
  }

  // --- Always upsert TP/SL to final target (create if missing, update if exists) ---
  if (absT > 0) {
	await upsertTPSL(symbol, side, absT, sl, tp, moneyPrec);
  } else {
	console.log('ℹ️ Final target is flat; no TP/SL set.');
  }
}

// --- Helpers ---

async function sendOrder(symbol, side, qty, price) {
  if (qty <= 0) return;
  const body = {
	request: '/api/v4/order/collateral/limit',
	nonce: Date.now(),
	market: symbol,
	side: side.toLowerCase(),
	amount: qty,
	price
  };
  console.log(`➡️ Order | ${symbol} | ${side} ${qty} @ ${price}`);
  await signandsend(body);
}

async function upsertTPSL(symbol, side, qty, sl, tp, moneyPrec) {
  const body = {
	request: '/api/v4/order/collateral/position/modify', // upsert: create if none, update if exists
	nonce: Date.now(),
	market: symbol,
	side: side.toLowerCase(),
	amount: qty,
	stopLoss: sl.toFixed(moneyPrec),
	takeProfit: tp ? tp.toFixed(moneyPrec) : undefined
  };
  console.log(`🔄 TP/SL upsert | ${symbol} | ${side} size=${qty} | SL=${body.stopLoss} | TP=${body.takeProfit ?? '—'}`);
  await signandsend(body);
}

async function upsertTPSL_v1(symbol, isBuy, targetQty, sl, tp, moneyPrec, stockPrec) {
    const closeSide = isBuy ? 'sell' : 'buy';

    // --- STOP LOSS ---
    if (sl) {
        const slBody = {
            request: '/api/v4/order/collateral/trigger-market',
            nonce: Date.now() + 1,
            market: symbol,
            side: closeSide,
            amount: parseFloat(targetQty).toFixed(stockPrec),
            activation_price: sl.toFixed(moneyPrec),
            trigger_type: 'stop_loss',           // ✅ correct
            trigger_price_type: 'last',          // ✅ 'mark' or 'last' (try 'last' first)
            reduce_only: true                    // ✅ optional but safe
        };
        console.log(`➡️ SL Order | ${symbol} | ${closeSide} ${targetQty} @ SL=${slBody.activation_price}`);
        await signandsend(slBody);
    }

    // --- TAKE PROFIT ---
    if (tp) {
        const tpBody = {
            request: '/api/v4/order/collateral/trigger-market',
            nonce: Date.now() + 2,
            market: symbol,
            side: closeSide,
            amount: parseFloat(targetQty).toFixed(stockPrec),
            activation_price: tp.toFixed(moneyPrec),
            trigger_type: 'take_profit',         // ✅ correct
            trigger_price_type: 'last',          // ✅ required
            reduce_only: true
        };
        console.log(`➡️ TP Order | ${symbol} | ${closeSide} ${targetQty} @ TP=${tpBody.activation_price}`);
        await signandsend(tpBody);
    }
}



// ----------------------------------------------------------------------
// HOW TO CALL THIS FUNCTION IN YOUR main logic:
// ----------------------------------------------------------------------
/*
if (absT > 0) {
    // You only need to pass the symbol, sl, tp, and precision
    await upsertTPSL(symbol, sl, tp, moneyPrec);
} else {
    console.log('ℹ️ Final target is flat; no TP/SL set.');
}
*/

async function logAction(type, reason, meta) {
  const base = `📝 ${type.toUpperCase()} | ${reason}`;
  const details = Object.entries(meta || {})
	.map(([k, v]) => `${k}=${v}`)
	.join(' | ');
  console.log(details ? `${base} | ${details}` : base);
}

async function cancelAllOrders(symbol = null) {
    const body = {
        request: '/api/v4/order/cancel/all',
        nonce: Date.now(),
    };
    if (symbol) body.market = symbol;

    console.log(symbol ? 
        `🧹 Canceling all open orders on ${symbol}` :
        `🧹 Canceling ALL open orders across all markets`);
    
    const res = await signandsend(body);
    // console.log('Cancel result:', res);
}

async function trade(){

	await getmarkets();
	const equity= await get_equity();
	console.log("equity", equity);

	await cancelAllOrders();

	let symbol=symbols[1];

	oqty=0;
	const gp=await getPositions(symbol.nc);
	// console.log("gp",gp);
	console.log("oqty",oqty);

	if(oqty==0 && symbol.entry != undefined) {
		// await open_order_v1(symbol.nc, symbol.side, symbol.entry, symbol.sli, symbol.tpi, equity * riskFrac);
		console.log(symbol.nc, symbol.side, symbol.entry, symbol.sli, symbol.tpi, equity * riskFrac);
		return;
	}
	if(oqty!=0){

	}


	const marketInfo = markets.find(m => m.name === symbol);
	if (!marketInfo) throw new Error(`Mercado ${symbol} não encontrado`);
	console.log("marketInfo",marketInfo);
	const stockPrec = parseInt(marketInfo.stockPrec);
	const moneyPrec = parseInt(marketInfo.moneyPrec);
	




	// await upsertTPSL(symbol,'buy',oqty,4513.41,4702.23,moneyPrec,stockPrec);
	// await upsertTPSL_v1(symbol,1,oqty,4523.41,4692.23,moneyPrec,stockPrec);
	return;

	// oqty=-0.02;
	// // oqty=0;
	// // await open_order(symbols[0].nc,"sell",4171,1.02,4.0,equity*0.25);
	// await open_order_v1(symbols[0].nc,"buy",4097,sli,tpi,equity*riskFrac);
	// await open_order_v1(symbols[0].nc,"sell",4142,sli,tpi,46*riskFrac);
	// tpi= 3;
	// sli=1.02;
	// riskFrac=0.1;
	// await open_order_v1(symbols[1].nc,"buy",4216,sli,tpi,equity*riskFrac);
	// await open_order_v1(symbols[0].nc,"buy",114340,sli,tpi,equity*riskFrac);
	// await open_order_v1(symbols[2].nc,"sell",210.20,sli,tpi,equity*riskFrac);
	// await open_order_v1(symbols[3].nc,"buy",2.83,sli,tpi,equity*riskFrac);
// 	await open_order_v1(symbols[1].nc,"buy",4140,sli,tpi,equity*riskFrac);
	// await open_order_v1(symbols[1].nc,"buy",4384,sli,tpi,equity*riskFrac);
// await open_order_v1(symbols[1].nc,side,entry,sli,tpi,equity*riskFrac);

	await cancelAllOrders();
	if (entry != 0) {

		await open_order_v1(symbols[1].nc, side, entry, sli, tpi, equity * riskFrac);
		// await open_order(symbols[1].nc, side, entry, sli, tpi, equity * riskFrac, oqty);
	}
	console.log(symbols[1].nc,side,entry,sli,tpi,equity*riskFrac);
}

//#endregion

(async()=>{
	// backtestSymbols(symbols);

	// for(let ei=4;ei<symbols.length;ei++){
	// // for(let i=0;i<symbols.length;i++)symbols[i].trade=0;
	// symbols[ei].trade=1;
	// equitytt=1000;
	// await test5();
	// console.log(symbols[ei].name,equitytt); 
	// break;
	// }

	await test3_v8goodess12();
	await trade();
})();
