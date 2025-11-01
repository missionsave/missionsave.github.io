const fs = require("fs");
const path = require("path");
const fetch = global.fetch || require("node-fetch");

const CONFIG = {
    riskFraction: 0.08,     // Risk per trade
    debug: 0,               // Debug mode (set to 1 for detailed logging)
    cacheDuration: 7 * 24 * 60 * 60 * 1000, // 7 days in ms
    timeframe: "1h",
    historyLength: 1000,
    windowSize: 950,
    stepSize: 1
};

const SYMBOLS = [
    // { name: "Bitcoin", pair: "BTCUSDT", nc: "BTC_PERP", trade: 1 },
    { name: "Etherum", pair: "ETHUSDT", nc: "ETH_PERP", trade: 1 },
    // { name: "Etherum", pair: "SOLUSDT", nc: "ETH_PERP", trade: 1 },
    // { name: "Etherum", pair: "XRPUSDT", nc: "ETH_PERP", trade: 1 },
];

const groupsCache = new Map();
let currentGlobalEquity = 100;

const round = (value, decimals = 4) => Math.round(value * 10 ** decimals) / 10 ** decimals;
const pctDiff = (base, value) => ((value - base) / base) * 100;

async function fetchData_(sym, interval, historyLength) {
    const cacheFile = path.join(__dirname, `${sym}_${interval}_${historyLength}.json`);
    try {
        const stats = fs.statSync(cacheFile);
        if (Date.now() - stats.mtimeMs < CONFIG.cacheDuration) {
            return JSON.parse(fs.readFileSync(cacheFile, "utf8"));
        }
    } catch (e) {}

    try {
        const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
        const response = await fetch(url);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const data = await response.json();
        fs.writeFileSync(cacheFile, JSON.stringify(data));
        return data;
    } catch (error) {
        console.error(`Fetch error for ${sym}: ${error.message}`);
        return [];
    }
}
async function fetchData(sym, interval, historyLength, endDate = new Date("2025-2-2")) {
  const url = `https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}&endTime=${endDate.getTime()}`;
  return await (await fetch(url)).json();
}

function simulateOrder(equity, open, high, low, close, direction, sl, tp, riskFraction, ft = false) {
    if ( open <= 0 || sl <= 0 || tp <= 0 || equity <= 0) {
        if (ft) console.log("Invalid trade parameters: skipping trade");
        return 0;
    }

    const slPrice = direction === 1 ? open * (1 - sl / 100) : open * (1 + sl / 100);
    const tpPrice = direction === 1 ? open * (1 + tp / 100) : open * (1 - tp / 100);
    const maxLoss = equity * riskFraction;
    const distToSL = Math.abs(open - slPrice);
    const minDist = Math.max(distToSL, open * 0.001, 0.001); // Tighter minimum distance
    const size = Math.min(maxLoss / minDist, equity * 2); // Cap position size to 2x equity

    const hitSL = direction === 1 ? low <= slPrice : high >= slPrice;
    const hitTP = direction === 1 ? high >= tpPrice : low <= tpPrice;
    let exitPrice, exitType;

    if (hitSL && hitTP) {
        const distSL = Math.abs(open - slPrice);
        const distTP = Math.abs(tpPrice - open);
        exitPrice = distTP < distSL ? tpPrice : slPrice;
        exitType = distTP < distSL ? 'TP' : 'SL';
    } else {
        exitPrice = hitSL ? slPrice : hitTP ? tpPrice : close;
        exitType = hitSL ? 'SL' : hitTP ? 'TP' : 'MKT';
    }

    const pnl = (direction === 1 ? exitPrice - open : open - exitPrice) * size;
    const cappedPnl = Math.max(-equity * 0.5, Math.min(pnl, equity * 0.5)); // Cap PNL at ±50% of equity

    if (ft) {
        console.log(`${exitType.padEnd(6)} ${direction === 1 ? 'BUY ' : 'SELL'} `
            + `o:${round(open)} tp:${round(tpPrice)} sl:${round(slPrice)} `
            + `cl:${round(close)} pnl:${round(cappedPnl, 2)} size:${round(size, 2)}`);
    }

    return cappedPnl;
}

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

function findMatchAndDetermineTrade(coinRaw, params, groups) {
    const { slsalt, tpsalt, atleast, gtol, wsize } = params;
    if (!coinRaw || coinRaw.length < wsize + 1) return null;

    const raw = coinRaw;
    const lastBase = raw[0][1];
    const lastCandles = Array(wsize).fill().map((_, j) => {
        const c = raw[j];
        return {
            open: pctDiff(lastBase, c[1]),
            high: pctDiff(lastBase, c[2]),
            low: pctDiff(lastBase, c[3]),
            close: pctDiff(lastBase, c[4])
        };
    });

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

    let cw = match.output.close >= atleast ? 1 : match.output.close <= -atleast ? 0 : match.output.high >= Math.abs(match.output.low) ? 1 : 0;
    const sl = cw === 1 ? Math.abs(match.output.low) + slsalt : Math.abs(match.output.high) + slsalt;
    const tp = Math.max(0.01, cw === 1 ? Math.abs(match.output.high) - tpsalt : Math.abs(match.output.low) - tpsalt);

    const nc = raw[wsize];
    return {
        direction: cw,
        sl: sl,
        tp: tp,
        nopen: nc[1],
        nhigh: nc[2],
        nlow: nc[3],
        nclose: nc[4]
    };
}

async function runSimulation(params, symbolsData) {
    const { wsize, gtol } = params;
    let equity = 100;
    const groups = buildGroups(symbolsData, wsize, gtol);
    if (!groups.length) return equity;

    const rawLength = symbolsData[0].raw.length;
    for (let i = wsize; i < rawLength; i++) {
        for (const coin of symbolsData) {
            if (coin.trade !== 1 || !coin.raw || coin.raw.length <= i) continue;
            const pattern_and_trade_data = coin.raw.slice(i - wsize, i + 1);
            const trade = findMatchAndDetermineTrade(pattern_and_trade_data, params, groups);
            if (!trade) continue;
            const pnl = simulateOrder(equity, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp, CONFIG.riskFraction);
            equity += pnl;
            if (equity <= 0) return 0;
        }
    }
    return equity;
}

async function performForwardTrade(bestParams, forwardTradeData, historicalGroups, currentEquity) {
    let equity = currentEquity;
    const wsize = bestParams.wsize;
    const rawLength = forwardTradeData[0].raw.length;

    for (let i = wsize; i < rawLength; i++) {
        for (const coin of forwardTradeData) {
            if (coin.trade !== 1 || !coin.raw || coin.raw.length <= i) {
                if (CONFIG.debug) console.log(`Skipping ${coin.pair}: insufficient data or trade disabled at index ${i}`);
                continue;
            }
            const pattern_and_trade_data = coin.raw.slice(i - wsize, i + 1);
            const trade = findMatchAndDetermineTrade(pattern_and_trade_data, bestParams, historicalGroups);
            if (!trade) {
                if (CONFIG.debug) console.log(`No trade found for ${coin.pair} at index ${i}`);
                continue;
            }
            const pnl = simulateOrder(equity, trade.nopen, trade.nhigh, trade.nlow, trade.nclose, trade.direction, trade.sl, trade.tp, CONFIG.riskFraction, true);
            equity += pnl;
            if (equity <= 0) return 0;
        }
    }
    return equity;
}

function robustnessScore(results) {
    if (!results.length) return 0;
    const cappedResults = results.map(pnl => Math.max(-1000, Math.min(pnl, 1000))); // Cap PNLs
    const mean = cappedResults.reduce((a, b) => a + b, 0) / cappedResults.length;
    const variance = cappedResults.reduce((s, x) => s + (x - mean) ** 2, 0) / cappedResults.length;
    const stdDev = Math.sqrt(variance);
    return mean / (1 + stdDev + 0.01); // Increased denominator for stability
}

async function rollingOptimizerWalkForward(symbolsData, paramGrid) {
    const numCandles = Math.min(...symbolsData.map(c => c.raw?.length || 0));
    const { windowSize, stepSize } = CONFIG;

    if (numCandles < windowSize + stepSize) {
        console.error(`Insufficient candles (${numCandles}) for windowSize ${windowSize} + stepSize ${stepSize}`);
        return [];
    }

    const results = [];
    let currentEquity = currentGlobalEquity;

    for (let i = windowSize; i < numCandles; i += stepSize) {
        const optimData = symbolsData.map(sym => ({
            ...sym,
            raw: sym.raw?.slice(i - windowSize, i) || []
        }));

        let bestScore = -Infinity, bestParams = null;
        groupsCache.clear();

        for (const params of paramGrid) {
            const pnls = [];
            for (const coin of optimData) {
                if (coin.raw.length < params.wsize + 1) continue;
                const equity = await runSimulation(params, [coin]);
                pnls.push(equity); // Convert to PNL
                // pnls.push(equity - 100); // Convert to PNL
            }
            // console.log("dbg", pnls[0] || 0, params);
            if (pnls.length > 0) {
                const score = pnls[0];
                // const score = robustnessScore(pnls);
				// if(score<20)continue;
                if (score>7000 && score > bestScore) { /////////////////////////////////////////////////////
                // if (score > bestScore) {
				console.log("bestScore",score);
                    bestScore = score;
                    bestParams = params;
                }
            }
        }

        if (!bestParams) {
            console.warn(`Period ${i}-${i + stepSize}: No valid parameters found. Skipping trade.`);
            continue;
        }

        const historicalGroups = buildGroups(optimData, bestParams.wsize, bestParams.gtol);
        const forwardTradeData = symbolsData.map(sym => ({
            ...sym,
            raw: sym.raw?.slice(i - bestParams.wsize, i + stepSize) || []
        }));

        const fordate = forwardTradeData[forwardTradeData.length - 1].raw;
        const dtraw = fordate[fordate.length - 1][0];
        const dt = `${new Date(dtraw).toISOString().split('T')[0]} ${String(new Date(dtraw).getUTCHours()).padStart(2, '0')}`;

        const equityBeforeTrade = currentEquity;
        const equityAfterTrade = await performForwardTrade(bestParams, forwardTradeData, historicalGroups, currentEquity);
        currentEquity = equityAfterTrade;
        const tradePnl = equityAfterTrade - equityBeforeTrade;

        results.push({
            period: `${i}-${i + stepSize}`,
            bestParams,
            robustness: bestScore,
            tradePnl,
            currentEquity
        });

        console.log(`✅${dt} Period ${i}-${i + stepSize} (opt ${windowSize} candles): PnL=${round(tradePnl, 2)}, eq=${round(currentEquity, 2)}, rob=${round(bestScore, 2)}`, bestParams);
    }

    return results;
}
function generateParamGrid_() {
    const sls = [1]; 
    // const tps = [ -0.9,-2]; 
    const tps = [-2,0.1]; 
    // const atls = [0.2]; 
    const atls = [0.04]; 
    // const atls = [0.05, 0.1, 0.2]; 
    // const atls = [0.05, 0.1, 0.15]; 
    const gtos = [1.0,  1.4,  2.2]; 
    // const gtos = [1.0,  1.4,   2.0, 2.4]; 
    const wsizes = [4,6, 8, 10,12]; 
    
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
function generateParamGrid() {
// 	const paramGrid = [
// //   { slsalt: 1, tpsalt: 0.1, atleast: 0.01, gtol: 2.8, wsize: 1, lb: 0 },
// //   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 1.8, wsize: 6, lb: 0 },
// //   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 1.8, wsize: 12, lb: 0 },
// //   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 6, lb: 0 },
// //   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 12, lb: 0 },
// //   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 3, lb: 0 },
//   { slsalt: 1, tpsalt: 0.1, atleast: 0.04, gtol: 1.8, wsize: 6, lb: 0 },
//   { slsalt: 1, tpsalt: 0.1, atleast: 0.04, gtol: 1.8, wsize: 12, lb: 0 },
//   { slsalt: 1, tpsalt: 0.1, atleast: 0.04, gtol: 0.5, wsize: 6, lb: 0 },
//   { slsalt: 1, tpsalt: 0.1, atleast: 0.04, gtol: 0.5, wsize: 12, lb: 0 },
//   { slsalt: 1, tpsalt: 0.1, atleast: 0.04, gtol: 0.5, wsize: 3, lb: 0 }
// ];
// return paramGrid;
    return [
        { slsalt: 0.77, tpsalt: 0.1, atleast: 0.01, gtol: 1.9, wsize: 14, lb: 0 },
        { slsalt: 0.77, tpsalt: 0.1, atleast: 0.01, gtol: 0.9, wsize: 14, lb: 0 },
        // { slsalt: 1, tpsalt: 0.12, atleast: 0.01, gtol: 0.9, wsize: 12, lb: 0 },
        { slsalt: 1, tpsalt: -10.12, atleast: 0.01, gtol: 0.9, wsize: 4, lb: 0 },
  { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 1.8, wsize: 6, lb: 0 },
  { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 1.8, wsize: 12, lb: 0 },
  { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 6, lb: 0 },
//   { slsalt: 1, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 12, lb: 0 },
//   { slsalt: 0.5, tpsalt: -10.1, atleast: 0.04, gtol: 0.5, wsize: 6, lb: 0 },
//   { slsalt: 0.5, tpsalt: -10.1, atleast: 0.04, gtol: 1.5, wsize: 8, lb: 0 }
    ];
}

(async () => {
    console.log("Starting Data Fetch...");
    for (const coin of SYMBOLS) {
        if (!coin.raw) {
            coin.raw = await fetchData(coin.pair, CONFIG.timeframe, CONFIG.historyLength);
            if (CONFIG.debug) console.log(`Fetched ${coin.raw.length} candles for ${coin.pair}`);
        }
    }

    const paramGrid = generateParamGrid();
    console.log("Starting Rolling Optimizer Walk-Forward Analysis...");

    const results = await rollingOptimizerWalkForward(SYMBOLS, paramGrid);

    console.log("\n========================================================");
    console.log("🏁 FINAL WALK-FORWARD RESULTS SUMMARY 🏁");
    console.log(`Optimization Window Size: ${CONFIG.windowSize} candles`);
    console.log(`Risk Per Trade: ${CONFIG.riskFraction * 100}%`);
    console.log(`Initial Equity: ${round(100, 2)}`);
    console.log(`Final Equity: ${results.length ? round(results[results.length - 1].currentEquity, 2) : round(currentGlobalEquity, 2)}`);
    console.log(`Total Periods Traded: ${results.length}`);
    console.log("========================================================\n");
})();