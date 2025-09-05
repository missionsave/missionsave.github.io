const fs = require('fs');
const vm = require('vm');

const inicio = Date.now();

// Carrega brain.min.js
const brainPath = __dirname + '/brain.min.js';
const stats = fs.statSync(brainPath);
console.log(`📦 brain.min.js encontrado com ${stats.size} bytes`);
const brainCode = fs.readFileSync(brainPath, 'utf8');
vm.runInThisContext(brainCode);

// Configurações para Binance
const interval = '1d';
const historyLength = 6;

async function fetchData(symbol) {
  const url = `https://api.mexc.com/api/v3/account`;
//   const url = `https://api.binance.com/`;
//   const url = `https://data-api.binance.vision/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${historyLength}`; //works
//   const url = `https://api.binance.com/api/v3/klines?symbol=${symbol}&interval=${interval}&limit=${historyLength}`;
  console.log("url",url);
  const res = await fetch(url);
  return res.json();
}

(async () => {
  const agora = new Date();
  console.log(`🕒 Script iniciado em: ${agora.toLocaleString('pt-PT', { timeZone: 'Europe/Lisbon' })}`);

  // Teste de fetch
  const data = await fetchData('BTCUSDT');
  console.log(`📊 Recebidas4 ${data.length} velas de BTCUSDT`);
//   const data = await fetchData('BTCUSDT');
console.log("📊 Tipo de dados recebidos:", Array.isArray(data) ? 'Array' : typeof data);
console.log("📊 Conteúdo recebido:", data);
console.log(`📊 Recebidas ${Array.isArray(data) ? data.length : 0} velas de BTCUSDT`);


  // Teste com Brain.js
  const net = new brain.NeuralNetwork();
  net.train([
    { input: [0, 0], output: [0] },
    { input: [0, 1], output: [1] },
    { input: [1, 0], output: [1] },
    { input: [1, 1], output: [0] }
  ]);

  const resultado = net.run([1, 0]);
  console.log("🧠 Resultado do teste:", resultado);

  const fim = Date.now();
  console.log(`⏱ Tempo total de execução: ${(fim - inicio) / 1000} segundos`);
  console.log("✅ Script concluído com sucesso");
})();
