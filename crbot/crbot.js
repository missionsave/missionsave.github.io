const fs = require("fs");
const path = require("path");
const fetch = global.fetch || require("node-fetch"); 

const WINDOW_SIZE_WEEKS = 4;
const CANDLES_PER_WEEK = 7 * (24 / 6); // 7 days * (24 hours / 6h timeframe) = 28 candles/week

const CONFIG = {
    stopLoss: 6.0,          // Stop loss percentage (UNUSED in final logic, kept for reference)
    takeProfit: 0.6,        // Take profit percentage (UNUSED in final logic, kept for reference)
    riskFraction: 0.08,     // Risk per trade (Updated from user's request)
    debug: 0,               // Debug mode (Set to 0 for less noise during optimization)
    cacheDuration: 7 * 24 * 60 * 60 * 1000, // 7 days in ms
    timeframe: "4h",
    historyLength: 1000,
    windowSize: 950, // CONSTANT Optimization window size: 4 weeks * 28 = 112 candles
    stepSize: 1           // How many candles to step forward after each trade (1 = single-candle forward test)
};

const SYMBOLS = [
    { name: "Bitcoin", pair: "BTCUSDT", nc: "BTC_PERP", trade: 1 },
    // { name: "Ethereum", pair: "ETHUSDT", nc: "ETH_PERP", trade: 1 },
    // { name: "Solana", pair: "SOLUSDT", nc: "SOL_PERP", trade: 1 },
    // { name: "XRP", pair: "XRPUSDT", nc: "XRP_PERP", trade: 1 }
];

// NOTE: groupsCache is now only used internally by buildGroups during the optimization pass
const groupsCache = new Map(); 
let currentGlobalEquity = 100; 

const round = (value, decimals = 4) => Math.round(value * 10 ** decimals) / 10 ** decimals;
const pctDiff = (base, value) => ((value - base) / base) * 100;

// --- Data Fetching (Kept as is) ---
async function fetchData_(symbol, interval, limit) {
    const cacheFile = path.join(__dirname, `${symbol}_${interval}_${limit}.json`);
    try {
        const stats = fs.statSync(cacheFile);
        if (Date.now() - stats.mtimeMs < CONFIG.cacheDuration) {
            return JSON.parse(fs.readFileSync(cacheFile, "utf8"));
        }
    } catch (e) {}

    try {
        const response = await fetch(`https://data-api.binance.vision/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${limit}`);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const data = await response.json();
        fs.writeFileSync(cacheFile, JSON.stringify(data));
        return data;
    } catch (error) {
        console.error(`Fetch error for ${symbol}: ${error.message}`);
        return [];
    }
}
async function fetchData(sym, interval, historyLength, endDate = new Date("2023-12-6")) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}&endTime=${endDate.getTime()}`;
  return await (await fetch(url)).json();
}

async function fetchData__(sym,interval,historyLength) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  return await (await fetch(url)).json();
}

// --- Order Simulation (Kept as is) ---
function simulateOrder(equity, open, high, low, close, direction, sl, tp,riskFraction,ft) {
	if(direction==0)return 0;
    const slPrice = direction === 1 ? open * (1 - sl / 100) : open * (1 + sl / 100);
    const tpPrice = direction === 1 ? open * (1 + tp / 100) : open * (1 - tp / 100);
    
    const maxLoss = equity * riskFraction; 
    const distToSL = Math.abs(open - slPrice);
    
    const minDist = Math.max(distToSL, open * 1e-8, 1e-8);
    const size = maxLoss / minDist;

    const hitSL = direction === 1 ? low <= slPrice : high >= slPrice;
    const hitTP = direction === 1 ? high >= tpPrice : low <= tpPrice;
    let exitPrice, exitType;

    if (hitSL && hitTP) {
        const distSL = Math.abs(open - slPrice);
        const distTP = Math.abs(tpPrice - open);
        const ratio = distTP === 0 ? Infinity : distSL / distTP;
        
        const isTie = ratio > 0.8 && ratio < 1.25;

        if (isTie) {
            exitPrice = (distTP < distSL ? tpPrice : slPrice);
            exitType = (distTP < distSL ? 'TP' : 'SL');
        } else {
            exitPrice = (direction === 1) ? (close <= slPrice ? slPrice : tpPrice) : (close >= slPrice ? slPrice : tpPrice);
            exitType = (direction === 1) ? (close <= slPrice ? 'SL' : 'TP') : (close >= slPrice ? 'SL' : 'TP');
        }
    } else {
        exitPrice = hitSL ? slPrice : hitTP ? tpPrice : close;
        exitType = hitSL ? 'SL' : hitTP ? 'TP' : 'MKT';
    }

    const pnl = (direction === 1 ? exitPrice - open : open - exitPrice) * size;
    // if (CONFIG.debug) 
	if(ft)
	{
        console.log(`${exitType} ${direction === 1 ? 'BUY' : 'SELL'} o:${round(open)} tp:${round(tpPrice)} sl:${round(slPrice)} pnl:${round(pnl, 2)}`);
    }
    return pnl;
    // return equity + pnl;
}
function simulateOrder_v1(
    equity, open, high, low, close,
    direction, sl, tp, riskFraction, ft = false
) {
    // --- Setup ---
    const slPrice = direction === 1 ? open * (1 - sl / 100) : open * (1 + sl / 100);
    const tpPrice = direction === 1 ? open * (1 + tp / 100) : open * (1 - tp / 100);
    const maxLoss = equity * riskFraction;

    const distToSL = Math.abs(open - slPrice);
    const minDist = Math.max(distToSL, open * 1e-8, 1e-8);
    const size = maxLoss / minDist;


  // 3️⃣ Calculate qty based on max risk
  const priceDiff = Math.abs(open - sl);
  if (priceDiff <= 0) throw new Error('Stop loss inválido para o lado da ordem');

  let qty = (equity* riskFraction / priceDiff).toFixed(4);



    // --- Hit logic ---
    const hitSL = direction === 1 ? low <= slPrice : high >= slPrice;
    const hitTP = direction === 1 ? high >= tpPrice : low <= tpPrice;

    let exitPrice, exitType;

    if (hitSL && hitTP) {
        // --- Both extremes touched: ambiguity resolution ---
        // Use candle "bias": close proximity to high or low to infer which happened last
        const nearHigh = Math.abs(high - close);
        const nearLow = Math.abs(close - low);
        const biasToTP = nearHigh < nearLow;

        // Add proportional factor depending on how close the close is to each side
        const biasRatio = (nearLow + nearHigh === 0) ? 0.5 : nearLow / (nearLow + nearHigh);

        // If close is clearly near one extreme, assume that hit happened last
        if (biasRatio > 0.55) { // closer to low → SL
            exitPrice = slPrice;
            exitType = 'SL';
        } else if (biasRatio < 0.45) { // closer to high → TP
            exitPrice = tpPrice;
            exitType = 'TP';
        } else {
            // Tie: assume whichever side was closer to open hit first
            const distSL = Math.abs(open - slPrice);
            const distTP = Math.abs(tpPrice - open);
            exitPrice = (distTP < distSL ? tpPrice : slPrice);
            exitType = (distTP < distSL ? 'TP' : 'SL');
        }

    } else if (hitSL) {
        exitPrice = slPrice;
        exitType = 'SL';
    } else if (hitTP) {
        exitPrice = tpPrice;
        exitType = 'TP';
    } else {
            exitPrice = close;
            exitType = 'MKT';
        
    }

    // --- PnL ---
    const pnl = (direction === 1 ? exitPrice - open : open - exitPrice) * size;

    if (ft) {
        console.log(
            `${exitType.padEnd(6)} ${direction === 1 ? 'BUY ' : 'SELL'} `
            + `o:${round(open)} tp:${round(tpPrice)} sl:${round(slPrice)} `
            + `cl:${round(close)} pnl:${round(pnl, 2)} qty:${qty}`
        );
    }

    return  pnl;
    // return equity + pnl;
}

// Helper for clean number output
// function round(v, d = 4) {
//     return parseFloat(v.toFixed(d));
// }

// --- Group Building (Kept as is) ---
function buildGroups(symbolsData, wsize, gtol) {
    const cacheKey = `${wsize}_${gtol}`;
    if (groupsCache.has(cacheKey)) {
        if (CONFIG.debug) console.log(`Using cached groups for wsize=${wsize}, gtol=${gtol}, count=${groupsCache.get(cacheKey).length}`);
        return groupsCache.get(cacheKey);
    }
    
    const isSimilar = (a, b, tol = 0.1) => {
        if (a.length !== b.length) return false;
        let distance = 0;
        for (let i = 0; i < a.length; i++) {
            distance += (a[i].high - b[i].high) ** 2 + (a[i].low - b[i].low) ** 2 + (a[i].close - b[i].close) ** 2;
        }
        return Math.sqrt(distance) / Math.sqrt(a.length * 3) < tol;
    };

    const groups = [];
    for (const coin of symbolsData) {
        const raw = coin.raw;
        if (!raw || raw.length < wsize + 2) continue;

        for (let i = 0; i <= raw.length - (wsize + 1); i++) {
            const baseOpen = raw[i][1];
            const input = Array(wsize).fill().map((_, j) => {
                const c = raw[i + j];
                return {
                    open: pctDiff(baseOpen, c[1]),
                    high: pctDiff(baseOpen, c[2]),
                    low: pctDiff(baseOpen, c[3]),
                    close: pctDiff(baseOpen, c[4])
                };
            });
            const out = raw[i + wsize];
            const output = { open: 0, high: pctDiff(out[1], out[2]), low: pctDiff(out[1], out[3]), close: pctDiff(out[1], out[4]) };

            const group = groups.find(g => isSimilar(g.input, input, gtol)) || { input, outputs: [] };
            if (!group.outputs.length) groups.push(group);
            group.outputs.push(output);
        }
    }

    const median = arr => arr.length ? [...arr].sort((a, b) => a - b)[Math.floor(arr.length / 2)] : 0;
    const condensed = groups.map(g => ({
        input: g.input,
        output: {
            open: 0,
            high: median(g.outputs.map(o => o.high)),
            low: median(g.outputs.map(o => o.low)),
            close: median(g.outputs.map(o => o.close))
        },
        count: g.outputs.length
    }));

    if (CONFIG.debug) console.log(`Built ${condensed.length} groups for wsize=${wsize}, gtol=${gtol}`);
    groupsCache.set(cacheKey, condensed);
    return condensed;
}

// Helper function to find a match and determine trade parameters
function findMatchAndDetermineTrade(coinRaw, params, groups) {
    const { slsalt, tpsalt, atleast, gtol, wsize } = params;

    // Check if we have enough data to form the pattern + the trade candle
    if (!coinRaw || coinRaw.length < wsize + 1) return null;

    const raw = coinRaw; 
    // The pattern starts at raw[0] and ends at raw[wsize-1]. The trade candle is raw[wsize].
    const lastBase = raw[0][1];
    
    // 1. Build the pattern input from the WSIZE candles
    const lastCandles = Array(wsize).fill().map((_, j) => {
        const c = raw[j];
        return {
            open: pctDiff(lastBase, c[1]),
            high: pctDiff(lastBase, c[2]),
            low: pctDiff(lastBase, c[3]),
            close: pctDiff(lastBase, c[4])
        };
    });

    // 2. Find the best matching group using the historical groups
    const match = groups.find(g => {
        let distance = 0;
        for (let k = 0; k < wsize; k++) {
            distance += (g.input[k].high - lastCandles[k].high) ** 2 +
                       (g.input[k].low - lastCandles[k].low) ** 2 +
                       (g.input[k].close - lastCandles[k].close) ** 2;
        }
        return Math.sqrt(distance) / Math.sqrt(wsize * 3) < gtol;
    });

    if (!match || match.count === 1) return null;

    // 3. Determine trade direction (cw)
    let cw; // 1: BUY, 0: SELL
    
    if (match.output.close >= atleast) {
        cw = 1; // Strong conviction BUY
    } else if (match.output.close <= -atleast) {
        cw = 0; // Strong conviction SELL
    } else {
        // Low conviction: Trade the direction with the greatest predicted median move
        if (match.output.high >= Math.abs(match.output.low)) {
            cw = 1; // Mild BUY signal
        } else {
            cw = 0; // Mild SELL signal
        }
    }
    
    // 4. Determine SL/TP prices
    const sl = cw === 1 ? Math.abs(match.output.low) + slsalt : Math.abs(match.output.high) + slsalt;
    const tp = cw === 1 ? Math.abs(match.output.high) - tpsalt : Math.abs(match.output.low) - tpsalt;
    const finalTP = Math.max(0.01, tp); 

    // The candle to be traded (the next one)
    const nc = raw[wsize]; 

    return {
        direction: cw,
        sl: sl,
        tp: finalTP,
        nopen: nc[1],
        nhigh: nc[2],
        nlow: nc[3],
        nclose: nc[4]
    };
}


/**
 * PHASE 1: Backtests the entire optimization period using a single parameter set.
 * @param {object} params - Trade parameters
 * @param {Array<object>} symbolsData - Array of symbol objects, each with the RAW OPTIMIZATION DATA.
 * @returns {number} The final equity PnL (Equity - 1000) after the simulation period.
 */
async function runSimulation(params, symbolsData) {
    const { wsize, gtol } = params;
    let equity = 100;
    
    // Build groups based on the entire OPTIMIZATION window slice.
    const groups = buildGroups(symbolsData, wsize, gtol);
    if (!groups.length) return equity;

    const rawLength = symbolsData[0].raw.length;
    // Iterate over the optimization window trades: Start from the first tradeable candle
    for (let i = wsize; i < rawLength; i++) {
        
    let equitystart = equity;
        for (const coin of symbolsData) {
            if (coin.trade !== 1 || !coin.raw || coin.raw.length <= i) continue;
            
            // Slice the data to get the pattern + the trade candle (from i-wsize to i)
            const pattern_and_trade_data = coin.raw.slice(i - wsize, i + 1);

            const trade = findMatchAndDetermineTrade(pattern_and_trade_data, params, groups);
            
            if (!trade) continue;

            equity += simulateOrder(equitystart, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp,CONFIG.riskFraction);
            // equity = simulateOrder(equity, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp,CONFIG.riskFraction);
        }
    }
    return equity;
}

/**
 * PHASE 2: Performs the actual forward trade(s) using the best-found parameters and pre-built groups.
 * @param {object} bestParams - The best parameters from the optimization period.
 * @param {Array<object>} forwardTradeData - Array of symbol data, sliced to include WSIZE history + the trade candle(s).
 * @param {Array<object>} historicalGroups - The groups built from the optimization data slice.
 * @param {number} currentEquity - The global equity before this trade.
 * @returns {number} The updated equity after the trade(s).
 */
async function performForwardTrade(bestParams, forwardTradeData, historicalGroups, currentEquity) {
    let equity = currentEquity;

    // Iterate over the trade candles (starting at index wsize of the slice)
    for (let i = bestParams.wsize; i < forwardTradeData[0].raw.length; i++) {
        
    let equitystart = equity;
        for (const coin of forwardTradeData) {
            if (coin.trade !== 1 || !coin.raw || coin.raw.length <= i) continue;
            
            // Slice the data to get the pattern + the trade candle (from i-wsize to i)
            const pattern_and_trade_data = coin.raw.slice(i - bestParams.wsize, i + 1);

            // Use the historical groups found in Phase 1
            const trade = findMatchAndDetermineTrade(pattern_and_trade_data, bestParams, historicalGroups);
            
            if (!trade) continue;

            equity += simulateOrder(equitystart, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp,CONFIG.riskFraction,1);
            // equity = simulateOrder(equity, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp,CONFIG.riskFraction,1);
        }
    }
    return equity;
}
async function performForwardTrade_v1np(bestParams, forwardTradeData, historicalGroups, currentEquity) {
    let equity = currentEquity;
    const wsize = bestParams.wsize;
    const rawLength = forwardTradeData[0].raw.length;

    // Iterate over the trade candles (starting at index wsize of the slice)
    for (let i = wsize; i < rawLength; i++) {
        const tradesToExecute = [];

        // --- Pass 1: Count simultaneous trades ---
        for (const coin of forwardTradeData) {
            if (coin.trade !== 1 || !coin.raw || coin.raw.length <= i) continue;
            
            const pattern_and_trade_data = coin.raw.slice(i - wsize, i + 1);

            const trade = findMatchAndDetermineTrade(pattern_and_trade_data, bestParams, historicalGroups);
            
            if (trade) {
                tradesToExecute.push({ trade, coin });
            }
        }
        
        const numTrades = tradesToExecute.length;
        
        if (numTrades > 0) {
            // --- Pass 2: Calculate and execute with divided risk ---
            // Calculate the risk fraction for each of the trades
            const riskFracPerTrade = 0.32 / numTrades;

            for (const { trade, coin } of tradesToExecute) {
                // Pass the calculated riskFracPerTrade to simulateOrder
                equity = simulateOrder(
                    equity, 
                    trade.nopen, 
                    trade.nhigh, 
                    trade.nlow, 
                    trade.nclose, 
                    trade.direction, 
                    trade.sl, 
                    trade.tp,
                    riskFracPerTrade, // The new custom risk fraction
                );
            }
        }
    }
    return equity;
}

// --- Robustness Score (Kept as is) ---
function robustnessScore(results) {
    if (!results.length) return 0;
    const mean = results.reduce((a, b) => a + b, 0) / results.length;
    const variance = results.reduce((s, x) => s + (x - mean) ** 2, 0) / results.length;
    return mean / (1 + Math.sqrt(variance) + 1e-6); 
}

// --- Walk-Forward Optimizer (FIXED to use pre-built groups) ---
async function rollingOptimizerWalkForward(symbolsData, paramGrid) {
    const numCandles = Math.min(...symbolsData.map(c => c.raw?.length || 0));
    const { windowSize, stepSize } = CONFIG;
    
    if (numCandles < windowSize + stepSize) {
        console.error(`Insufficient candles (${numCandles}) for windowSize ${windowSize} + stepSize ${stepSize}`);
        return [];
    }

    const results = [];
    let currentEquity = currentGlobalEquity;

    for (let i = windowSize; i < numCandles - stepSize+1; i += stepSize) {
    // for (let i = windowSize; i < numCandles - stepSize; i += stepSize) {
        const optimStartIndex = i - windowSize;
        const optimEndIndex = i; 
        const tradeStartIndex = i;
        const tradeEndIndex = i + stepSize; 

        // --- PHASE 1: OPTIMIZATION (Fixed Window Size) ---
        const optimData = symbolsData.map(sym => ({
            ...sym,
            raw: sym.raw?.slice(optimStartIndex, optimEndIndex) || [] 
        }));

        let bestScore = -Infinity, bestParams = null;
        groupsCache.clear(); // Ensure groups are rebuilt for the new window

        for (const params of paramGrid) {
            const pnls = [];
            for (const coin of optimData) {
                if (coin.raw.length < params.wsize + 1) continue;
                
                // runSimulation performs backtest on the entire optimData slice
                const pnl = await runSimulation(params, [coin]); 
                pnls.push(pnl - 100); 
            }
            if (pnls.length > 0) { 
                const score = robustnessScore(pnls);
                if (score > bestScore) {
                    bestScore = score;
                    bestParams = params;
                }
            }
        }

        if (!bestParams) {
            console.warn(`Period ${tradeStartIndex}-${tradeEndIndex}: No valid parameters found. Skipping trade.`);
            continue;
        }
        
        // --- PHASE 2: FORWARD TRADING (Use bestParams) ---

        // 1. Build the FINAL groups using the bestParams and the full optimization data.
        // This is the source of all patterns for the forward trade.
        const historicalGroups = buildGroups(optimData, bestParams.wsize, bestParams.gtol);
        
        // 2. Slice the data for the forward trade: WSIZE history + the trade candle(s)
        const forwardTradeData = symbolsData.map(sym => ({
            ...sym,
            raw: sym.raw?.slice(tradeStartIndex - bestParams.wsize, tradeEndIndex) || []
        }));
        
		const fordate=forwardTradeData[forwardTradeData.length-1].raw;
		const dtraw=fordate[fordate.length-1][0];
		const dt = `${new Date(dtraw).toISOString().split('T')[0]} ${String(new Date(dtraw).getUTCHours()).padStart(2, '0')}`;
		// console.log(dt);

        const equityBeforeTrade = currentEquity;
		console.log("equityBeforeTrade",equityBeforeTrade);
        
        // 3. Perform the forward trade using the historical groups
        const equityAfterTrade = await performForwardTrade(
            bestParams, 
            forwardTradeData, 
            historicalGroups, 
            currentEquity
        );
        
        currentEquity = equityAfterTrade;
        const tradePnl = equityAfterTrade - equityBeforeTrade;

        results.push({
            period: `${tradeStartIndex}-${tradeEndIndex}`,
            bestParams,
            robustness: bestScore,
            tradePnl: tradePnl,
            currentEquity: currentEquity,
        });
        
        console.log(`✅${dt} Period ${tradeStartIndex}-${tradeEndIndex} (opt ${windowSize} candles): PnL=${round(tradePnl, 2)}, eq=${round(currentEquity, 2)}, rob=${round(bestScore, 2)}`,results[results.length-1].bestParams);
    }

    return results;
}

/**
 * Generates parameter combinations.
 */
function generateParamGrid() {
    const sls = [1]; 
    const tps = [ 0.1]; 
    // const tps = [0.1,0.2,0.4,0.7]; 
    // const atls = [0.2]; 
    const atls = [0.04]; 
    // const atls = [0.05, 0.1, 0.2]; 
    // const atls = [0.05, 0.1, 0.15]; 
    const gtos = [1.7]; 
    // const gtos = [1.0,  1.4,  2.2]; 
    // const gtos = [1.0,  1.4,   2.0, 2.4]; 
    // const wsizes = [3]; 
    const wsizes = [5]; 
    // const wsizes = [6, 8, 10,12]; 
    
    const paramGrid = [];

    for (const slsalt of sls) {
        for (const tpsalt of tps) {
            for (const atleast of atls) {
                for (const gtol of gtos) {
                    for (const wsize of wsizes) {
                        paramGrid.push({ slsalt, tpsalt, atleast, gtol, wsize, lb: 0 });
                    }
                }
            }
        }
    }
    console.log(`\nGenerated ${paramGrid.length} parameter grids for optimization.`);
    return paramGrid;
}


// --- Final Execution Block ---
(async () => {
    console.log("Starting Data Fetch...");
    const CANDLE_COUNT =  CONFIG.historyLength; // Fetch slightly more history to ensure rolling window works
    // const CANDLE_COUNT =  1000; // Fetch slightly more history to ensure rolling window works
    // const CANDLE_COUNT = CONFIG.windowSize + 2; // Fetch slightly more history to ensure rolling window works
    for (const coin of SYMBOLS) {
        if (!coin.raw) {
            coin.raw = await fetchData(coin.pair, CONFIG.timeframe, CANDLE_COUNT);
            if (CONFIG.debug) console.log(`Fetched ${coin.raw.length} candles for ${coin.pair}`);
        }
    }

    const paramGrid = generateParamGrid();
    
    console.log("Starting Rolling Optimizer Walk-Forward Analysis...");

    const results = await rollingOptimizerWalkForward(SYMBOLS, paramGrid);

	console.log("\n results",results[results.length-1].bestParams);
    
    console.log("\n========================================================");
    console.log("🏁 FINAL WALK-FORWARD RESULTS SUMMARY 🏁");
    console.log(`Optimization Window Size: ${CONFIG.windowSize} candles (4 weeks of 6h data)`);
    console.log(`Risk Per Trade: ${CONFIG.riskFraction * 100}%`);
    console.log(`Initial Equity: ${round(100, 2)}`);
    console.log(`Final Equity: ${round(results[results.length-1].currentEquity, 2)}`);
    // console.log(`Final Equity: ${round(currentGlobalEquity, 2)}`);
    console.log(`Total Periods Traded: ${results.length}`);
    console.log("========================================================\n");
})();