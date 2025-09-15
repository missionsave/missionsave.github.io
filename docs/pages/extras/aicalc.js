const fs   = require('fs');
const vm   = require('vm');
const path = require('path');
const crypto = require('crypto');


var markets=0;

//Load brain.js into this context
const brainPath = path.join(__dirname, 'brain.min.js');
const brainCode = fs.readFileSync(brainPath, 'utf8');
vm.runInThisContext(brainCode); // exposes global `brain`

const symbols = [
  { name: "Bitcoin", pair: "BTCUSDT", nc: "BTC_PERP" },
  { name: "Ethereum", pair: "ETHUSDT", nc: "ETH_PERP"  },
  { name: "Solana", pair: "SOLUSDT", nc: "SOL_PERP" },
  { name: "XRP", pair: "XRPUSDT", nc: "XRP_PERP" }
];

const lb=0, interval="1d", historyLength=10+lb, RERUNS=8*2, TRAIN_ITER=200*2, atleast=0.01, budget=100, eps=1e-9;
const portfolio=[];

async function fetchData(sym){
  const url=`https://data-api.binance.vision/api/v3/klines?symbol=${sym}&interval=${interval}&limit=${historyLength}`;
  return (await fetch(url)).json();
}

function normalize(rows){
  const f=rows[0].length, mins=Array(f).fill(Infinity), maxs=Array(f).fill(-Infinity);
  rows.forEach(r=>r.forEach((v,i)=>{if(v<mins[i])mins[i]=v;if(v>maxs[i])maxs[i]=v}));
  const norm=rows.map(r=>r.map((v,i)=>(v-mins[i])/(maxs[i]-mins[i]||1)));
  return {norm,mins,maxs};
}
const denorm=(r,mins,maxs)=>r.map((v,i)=>v*(maxs[i]-mins[i])+mins[i]);

function trainOnce(rows){
  const {norm,mins,maxs}=normalize(rows);
  const net=new brain.recurrent.LSTMTimeStep({inputSize:4,hiddenLayers:[16],outputSize:4});
  net.train(norm,{iterations:TRAIN_ITER,learningRate:.01});
  return denorm(net.run(norm.slice(-1)),mins,maxs);
}
const median=a=>{const s=[...a].sort((x,y)=>x-y),m=Math.floor(s.length/2);return s.length%2?s[m]:(s[m-1]+s[m])/2};
function predict(rows,runs=RERUNS){
  const h=[],l=[],c=[],v=[];
  for(let i=0;i<runs;i++){const [H,L,C,V]=trainOnce(rows);h.push(H);l.push(L);c.push(C);v.push(V);}
  return [median(h),median(l),median(c),median(v)];
}
function signal(pred,prev){
  const diff=((pred-prev)/prev)*100;let s="HOLD",sl,tp;
  if(diff>atleast){s="buy";sl=prev*.98;tp=prev*1.03;}
  else if(diff<-atleast){s="sell";sl=prev*1.02;tp=prev*.97;}
  return {s,diff,sl,tp};
}

(async()=>{
  for(const coin of symbols){
    let raw=await fetchData(coin.pair);if(lb>0)raw=raw.slice(0,-lb);
    const hist=raw.slice(0,-1), cur=raw[raw.length-1];
    const rows=hist.map(c=>[+c[2],+c[3],+c[4],+c[5]]);
    const [ph,pl,pc]=predict(rows), prevClose=+hist.at(-1)[4], curClose=+cur[4];
    const {s,diff,sl,tp}=signal(pc,prevClose);
    const entry=prevClose;
    const maxWin=s==="buy"?((tp-entry)/entry)*100:s==="sell"?((entry-tp)/entry)*100:0;
    const maxLoss=s==="buy"?((entry-sl)/entry)*100:s==="sell"?((sl-entry)/entry)*100:0;
    const expRaw=s==="buy"?((pc-entry)/entry)*100:s==="sell"?((entry-pc)/entry)*100:0;
    const expWin=Math.max(0,Math.min(expRaw,maxWin));
    const rec={name:coin.name,pair:coin.pair,nc:coin.nc,signal:s,prevClose,pc,diff,entry,sl,tp,maxWin,maxLoss,expWin,pc,ph,pl,curClose};
    portfolio.push(rec);
    // console.log(rec);
  }
  const scores=portfolio.map(p=>(p.signal!=="HOLD"&&p.expWin>0&&p.maxLoss>0)?p.expWin/(p.maxLoss+eps):0);
  const sum=scores.reduce((a,b)=>a+b,0);
  const alloc=portfolio.map((p,i)=>sum? (scores[i]/sum)*budget:0);
  console.log("\nAllocation:");
  portfolio.forEach((p,i)=>{ 
	// console.log(p.name,p.signal,p.pc.toFixed(2),p.entry.toFixed(2),p.sl.toFixed(2),p.tp.toFixed(2),p.pl,p.ph,"alloc:",alloc[i].toFixed(2)+"%",p.curClose);

	// console.log(p.name,p.nc,p.signal,p.entry,p.sl,p.tp);








///Whitebit
const API_KEY = process.env.WB_KEY;
const API_SECRET = process.env.WB_SECRET; 

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

 
(async()=>{
	await console.log(p.name,p.nc,p.signal,p.entry,p.sl,p.tp);
	await getmarkets();
	await open_order(p.nc,p.signal,p.entry,2.1,3.1,3);
})();











});
})();
