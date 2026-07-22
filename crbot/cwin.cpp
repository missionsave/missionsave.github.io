/*
 * alpha_factory_final.cpp – 6 estratégias reais, taxas MEXC 0.08%/0.08%,
 *                             Kelly adaptativo (max 2.5x), correlation guard,
 *                             trailing stops, MIN_TRADES_HOLDOUT=5.
 *
 * Compile:
 *   g++ -std=c++20 -Os -s alpha_factory_final.cpp ../common/cJSON.c \
 *       -lcurl -lcrypto -lssl -lpthread -ldl -o alpha_factory
 *
 * Uso:
 *   ./alpha_factory hourly   (executa sinais, atualiza trailing, sincroniza)
 *   ./alpha_factory daily    (verifica equity e drawdown)
 *   ./alpha_factory weekly   (re‑otimização walk‑forward completa)
 *
 * Variáveis de ambiente:
 *   MEXC_API_KEY, MEXC_API_SECRET
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <ctime>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <curl/curl.h>
#include <cJSON.h>
#include <memory>
#include <openssl/hmac.h>
#include <openssl/evp.h>

// ----------------------------------------------------------------------
// Configuração global
// ----------------------------------------------------------------------
static constexpr double MAX_RISK_PER_TRADE       = 0.035;
static constexpr double MIN_RISK_PER_TRADE       = 0.01;
static constexpr double PORTFOLIO_DAILY_VOL_TARGET= 0.03;
static constexpr double MAX_PORTFOLIO_NOTIONAL   = 8.0;
static constexpr double CORRELATION_LIMIT        = 0.65;
static constexpr double TEST_FRACTION            = 0.25;
static constexpr double MIN_HOLDOUT_SHARPE       = 0.3;
static constexpr int    MIN_TRADES_HOLDOUT       = 5;
static constexpr int    MONTE_CARLO_ITERATIONS   = 2000;
static constexpr double MAX_MC_RUIN_PROB         = 25.0;
static constexpr int    MAX_HOLD_HOURS           = 72;
static constexpr int    MAX_POSITIONS            = 5;
static constexpr const char* STATE_FILE          = "state.json";
static constexpr int    KELLY_WINDOW             = 30;
static constexpr double KELLY_SHARPE_MAX         = 2.0;
static constexpr double KELLY_SHARPE_MIN         = 0.5;

// ----------------------------------------------------------------------
// Abstração de exchange
// ----------------------------------------------------------------------
enum class Exchange { MEXC, BINANCE, BYBIT, OKX };

struct ExchangeConfig {
    Exchange exchange;
    std::string api_key;
    std::string api_secret;
    std::string base_url;
    std::string name() const {
        switch(exchange) {
            case Exchange::MEXC:   return "MEXC";
            case Exchange::BINANCE:return "BINANCE";
            case Exchange::BYBIT:  return "BYBIT";
            case Exchange::OKX:    return "OKX";
        }
        return "UNKNOWN";
    }
};

static std::vector<ExchangeConfig> exchanges;

static void init_exchanges() {
    const char* mexc_key = std::getenv("MEXC_API_KEY");
    const char* mexc_secret = std::getenv("MEXC_API_SECRET");
    if (mexc_key && mexc_secret) {
        exchanges.push_back({Exchange::MEXC, mexc_key, mexc_secret, "https://contract.mexc.com"});
    }
}

// ----------------------------------------------------------------------
// Tipos base
// ----------------------------------------------------------------------
struct Candle { long long timestamp; double high, low, close; };

struct Signal { 
    std::string direction = "WAIT"; 
    double entry = 0, stop_loss = 0, take_profit = 0, confidence = 0, risk_percent = 0;
    std::string strategy_name;
    Exchange exchange = Exchange::MEXC;
};

struct OpenPos {
    std::string symbol, side, strategy, exchange;
    double quantity = 0, entry_price = 0, sl = 0, tp = 0, trailing = 0;
    std::int64_t timestamp = 0;
};

struct StrategyCfg {
    std::string name, symbol;
    Exchange exchange = Exchange::MEXC;
    std::map<std::string, double> params;
    bool active = true;
    double mc_ruin_prob = 0, holdout_sharpe = 0;
    int holdout_trades = 0;
};

struct BotState {
    double equity = 1000.0, starting_equity = 1000.0, peak_equity = 1000.0;
    double total_fees_paid = 0.0;
    std::vector<OpenPos> positions;
    std::vector<StrategyCfg> strategies;
    std::int64_t trading_halted_until = 0, last_run = 0;
    std::vector<double> equity_curve, recent_trade_returns;
};

struct Balance { double equity = 0.0, available = 0.0; };
struct ContractInfo { double priceScale = 0, contractSize = 0; };

// ----------------------------------------------------------------------
// Utilitários
// ----------------------------------------------------------------------
static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::stringstream ss; ss << f.rdbuf(); return ss.str();
}
static void write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path); f << content;
}
static std::time_t now_epoch() { return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); }
static long long now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count(); }

// ----------------------------------------------------------------------
// MEXC assinatura & HTTP
// ----------------------------------------------------------------------
static std::string get_mexc_signature(const std::string& apiKey, const std::string& secret,
                                       const std::string& timestamp, const std::string& requestBody) {
    std::string message = apiKey + timestamp + requestBody;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    HMAC(EVP_sha256(), secret.c_str(), secret.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(),
         hash, &length);
    std::stringstream ss;
    for (unsigned int i = 0; i < length; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    return ss.str();
}

static size_t write_cb(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static std::string http_request(const std::string& url, const std::string& method = "GET",
                                 const std::string& payload = "", struct curl_slist* headers = nullptr) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string buf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (!payload.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AlphaFactory/1.0");
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return buf;
}

static std::string mexc_request(const ExchangeConfig& cfg, const std::string& method,
                                 const std::string& endpoint, const std::string& payload = "") {
    long long ms = now_ms();
    std::string ts = std::to_string(ms);
    std::string sig = get_mexc_signature(cfg.api_key, cfg.api_secret, ts, payload);
    std::string url = cfg.base_url + endpoint;
    if (method == "GET" && !payload.empty()) url += "?" + payload;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("ApiKey: " + cfg.api_key).c_str());
    headers = curl_slist_append(headers, ("Request-Time: " + ts).c_str());
    headers = curl_slist_append(headers, ("Signature: " + sig).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    std::string result = http_request(url, method, (method == "POST" ? payload : ""), headers);
    curl_slist_free_all(headers);
    return result;
}

// ----------------------------------------------------------------------
// Funções auxiliares da exchange
// ----------------------------------------------------------------------
static ExchangeConfig* get_exchange(Exchange e) {
    for (auto& ex : exchanges) if (ex.exchange == e) return &ex;
    return nullptr;
}

static double get_current_price(Exchange e, const std::string& symbol) {
    auto* cfg = get_exchange(e);
    if (!cfg) return 0;
    if (e == Exchange::MEXC) {
        std::string resp = mexc_request(*cfg, "GET", "/api/v1/contract/ticker?symbol=" + symbol);
        size_t p = resp.find("\"lastPrice\":");
        if (p == std::string::npos) return 0;
        p += 12;
        size_t e2 = resp.find_first_of(",}", p);
        if (e2 == std::string::npos) return 0;
        try { return std::stod(resp.substr(p, e2 - p)); } catch(...) { return 0; }
    }
    return 0;
}

static std::vector<std::string> get_open_symbols(Exchange e) {
    std::vector<std::string> syms;
    auto* cfg = get_exchange(e);
    if (!cfg) return syms;
    if (e == Exchange::MEXC) {
        std::string resp = mexc_request(*cfg, "GET", "/api/v1/private/position/open_positions");
        size_t pos = 0;
        while (true) {
            size_t s = resp.find("\"symbol\":\"", pos);
            if (s == std::string::npos) break;
            s += 10;
            size_t end = resp.find("\"", s);
            if (end == std::string::npos) break;
            std::string sym = resp.substr(s, end - s);
            size_t hv = resp.find("\"holdVol\":", end);
            if (hv == std::string::npos) break;
            hv += 10;
            size_t hvEnd = resp.find(",", hv);
            if (hvEnd == std::string::npos) break;
            double holdVol = std::stod(resp.substr(hv, hvEnd - hv));
            if (holdVol > 0.0) syms.push_back(sym);
            pos = end + 1;
        }
    }
    return syms;
}

static ContractInfo get_contract_info(Exchange e, const std::string& symbol) {
    auto* cfg = get_exchange(e);
    if (!cfg || e != Exchange::MEXC) return {};
    std::string resp = mexc_request(*cfg, "GET", "/api/v1/contract/detail?symbol=" + symbol);
    auto extract = [&](const std::string& key) -> double {
        std::string tag = "\"" + key + "\":";
        size_t start = resp.find(tag);
        if (start == std::string::npos) return 0;
        start += tag.size();
        size_t end = resp.find_first_of(",}", start);
        if (end == std::string::npos) return 0;
        return std::stod(resp.substr(start, end - start));
    };
    return {extract("priceScale"), extract("contractSize")};
}

static Balance get_balance(Exchange e) {
    auto* cfg = get_exchange(e);
    if (!cfg) return {0,0};
    if (e == Exchange::MEXC) {
        std::string resp = mexc_request(*cfg, "GET", "/api/v1/private/account/asset/USDT");
        auto extract = [&](const std::string& key) -> double {
            size_t p = resp.find("\"" + key + "\":");
            if (p == std::string::npos) return 0;
            p += key.size() + 3;
            size_t e2 = resp.find_first_of(",}", p);
            if (e2 == std::string::npos) return 0;
            try { return std::stod(resp.substr(p, e2 - p)); } catch(...) { return 0; }
        };
        return {extract("equity"), extract("availableBalance")};
    }
    return {0,0};
}

static void cancel_all_orders(Exchange e) {
    auto* cfg = get_exchange(e);
    if (!cfg) return;
    if (e == Exchange::MEXC) mexc_request(*cfg, "POST", "/api/v1/private/order/cancel_all", "{}");
}

static void execute_bracket(Exchange e, const std::string& symbol, const std::string& side,
                              double qty, double entry, double sl, double tp, double& total_fees_paid) {
    auto* cfg = get_exchange(e);
    if (!cfg) return;
    if (e == Exchange::MEXC) {
        auto info = get_contract_info(e, symbol);
        int contracts = (info.contractSize > 0) ? (int)std::floor(qty / info.contractSize) : 0;
        if (contracts <= 0) return;
        
        const double maker_fee = 0.0008;
        const double taker_fee = 0.0008;
        
        int sideInt = (side == "BUY") ? 1 : 3;
        int bestLev = 1;
        const double MMR = 0.004;
        if (side == "BUY" && sl < entry) {
            double maxLev = entry / (entry - sl + entry * MMR);
            bestLev = std::min(50, (int)maxLev);
        } else if (side == "SELL" && sl > entry) {
            double maxLev = entry / (sl - entry + entry * MMR);
            bestLev = std::min(50, (int)maxLev);
        }
        if (bestLev < 1) bestLev = 1;
        
        double notional = contracts * info.contractSize * entry;
        double maker_fee_cost = notional * maker_fee;
        double taker_fee_cost = notional * taker_fee;
        total_fees_paid += maker_fee_cost + taker_fee_cost;
        
        double adjusted_entry = (side == "BUY") ? entry * (1.0 + maker_fee) : entry * (1.0 - maker_fee);
        double adjusted_sl = (side == "BUY") ? sl * (1.0 - taker_fee) : sl * (1.0 + taker_fee);
        double adjusted_tp = (side == "BUY") ? tp * (1.0 - taker_fee) : tp * (1.0 + taker_fee);
        
        std::stringstream order;
        order << "{"
              << "\"symbol\":\"" << symbol << "\","
              << "\"price\":" << adjusted_entry << ","
              << "\"vol\":" << contracts << ","
              << "\"side\":" << sideInt << ","
              << "\"type\":1,"
              << "\"openType\":1,"
              << "\"leverage\":" << bestLev << ","
              << "\"stopLossPrice\":" << adjusted_sl << ","
              << "\"takeProfitPrice\":" << adjusted_tp
              << "}";
        mexc_request(*cfg, "POST", "/api/v1/private/order/create", order.str());
        std::cout << "EXECUTE [" << cfg->name() << ":" << symbol << "] " << side 
                  << " qty=" << contracts << " entry=" << adjusted_entry 
                  << " sl=" << adjusted_sl << " tp=" << adjusted_tp 
                  << " lev=" << bestLev << "x fees=" << (maker_fee_cost+taker_fee_cost) << " USDT\n";
    }
}

static void close_position_order(Exchange e, const std::string& symbol, const std::string& side, double qty) {
    auto* cfg = get_exchange(e);
    if (!cfg) return;
    if (e == Exchange::MEXC) {
        auto info = get_contract_info(e, symbol);
        int contracts = (info.contractSize > 0) ? (int)std::floor(qty / info.contractSize) : 0;
        if (contracts <= 0) return;
        double price = get_current_price(e, symbol);
        int closeSide = (side == "LONG") ? 4 : 2;
        std::stringstream order;
        order << "{" << "\"symbol\":\"" << symbol << "\",\"price\":" << price 
              << ",\"vol\":" << contracts << ",\"side\":" << closeSide 
              << ",\"type\":1,\"openType\":1}";
        mexc_request(*cfg, "POST", "/api/v1/private/order/create", order.str());
        std::stringstream cancel;
        cancel << "{\"symbol\":\"" << symbol << "\"}";
        mexc_request(*cfg, "POST", "/api/v1/private/planorder/cancel_all", cancel.str());
    }
}

static void update_mexc_trailing_stop(Exchange e, const std::string& symbol,
                                       const std::string& side, double qty,
                                       double entry, double new_sl, double tp) {
    auto* cfg = get_exchange(e);
    if (!cfg || e != Exchange::MEXC) return;
    std::string cancel_payload = "{\"symbol\":\"" + symbol + "\"}";
    mexc_request(*cfg, "POST", "/api/v1/private/planorder/cancel_all", cancel_payload);
    auto info = get_contract_info(e, symbol);
    int contracts = (info.contractSize > 0) ? (int)std::floor(qty / info.contractSize) : 0;
    if (contracts <= 0) return;
    int exitSideInt = (side == "LONG") ? 4 : 2;
    double slOrderPrice = (side == "LONG") ? new_sl * 0.995 : new_sl * 1.005;
    int slTriggerType = (side == "LONG") ? 2 : 1;
    std::stringstream sl;
    sl << "{\"symbol\":\"" << symbol << "\",\"leverage\":1,\"openType\":1,"
       << "\"triggerPrice\":" << new_sl << ",\"triggerType\":" << slTriggerType
       << ",\"price\":" << slOrderPrice << ",\"vol\":" << contracts
       << ",\"side\":" << exitSideInt << ",\"orderType\":1,\"executeCycle\":1,\"trend\":1}";
    mexc_request(*cfg, "POST", "/api/v1/private/planorder/place/v2", sl.str());
    if (tp > 0) {
        int tpTriggerType = (side == "LONG") ? 1 : 2;
        std::stringstream tp_stream;
        tp_stream << "{\"symbol\":\"" << symbol << "\",\"leverage\":1,\"openType\":1,"
                  << "\"triggerPrice\":" << tp << ",\"triggerType\":" << tpTriggerType
                  << ",\"price\":" << tp << ",\"vol\":" << contracts
                  << ",\"side\":" << exitSideInt << ",\"orderType\":1,\"executeCycle\":1,\"trend\":1}";
        mexc_request(*cfg, "POST", "/api/v1/private/planorder/place/v2", tp_stream.str());
    }
}

// ----------------------------------------------------------------------
// Persistência do estado
// ----------------------------------------------------------------------
static bool load_state(BotState& s) {
    std::string raw = read_file(STATE_FILE);
    if (raw.empty()) return false;
    cJSON* root = cJSON_Parse(raw.c_str());
    if (!root) return false;
    auto get_double = [](cJSON* obj, const char* key, double def = 0) {
        cJSON* item = cJSON_GetObjectItem(obj, key);
        return (item && cJSON_IsNumber(item)) ? item->valuedouble : def;
    };
    auto get_string = [](cJSON* obj, const char* key, const char* def = "") -> std::string {
        cJSON* item = cJSON_GetObjectItem(obj, key);
        return (item && cJSON_IsString(item)) ? item->valuestring : def;
    };
    s.equity = get_double(root, "equity", 1000.0);
    s.starting_equity = get_double(root, "starting_equity", 1000.0);
    s.peak_equity = get_double(root, "peak_equity", 1000.0);
    s.total_fees_paid = get_double(root, "total_fees_paid");
    s.trading_halted_until = (std::int64_t)get_double(root, "trading_halted_until");
    s.last_run = (std::int64_t)get_double(root, "last_run");
    
    cJSON* pos_arr = cJSON_GetObjectItem(root, "positions");
    if (cJSON_IsArray(pos_arr)) {
        int n = cJSON_GetArraySize(pos_arr);
        for (int i = 0; i < n; ++i) {
            cJSON* item = cJSON_GetArrayItem(pos_arr, i);
            OpenPos p;
            p.symbol = get_string(item, "symbol");
            p.side = get_string(item, "side");
            p.strategy = get_string(item, "strategy");
            p.exchange = get_string(item, "exchange", "MEXC");
            p.quantity = get_double(item, "quantity");
            p.entry_price = get_double(item, "entry_price");
            p.sl = get_double(item, "sl");
            p.tp = get_double(item, "tp");
            p.trailing = get_double(item, "trailing");
            p.timestamp = (std::int64_t)get_double(item, "timestamp");
            if (!p.symbol.empty()) s.positions.push_back(p);
        }
    }
    
    cJSON* strat_arr = cJSON_GetObjectItem(root, "strategies");
    if (cJSON_IsArray(strat_arr)) {
        int n = cJSON_GetArraySize(strat_arr);
        for (int i = 0; i < n; ++i) {
            cJSON* item = cJSON_GetArrayItem(strat_arr, i);
            StrategyCfg cfg;
            cfg.name = get_string(item, "name");
            cfg.symbol = get_string(item, "symbol");
            cfg.exchange = (Exchange)(int)get_double(item, "exchange", (double)Exchange::MEXC);
            cfg.active = (bool)get_double(item, "active", 1.0);
            cfg.mc_ruin_prob = get_double(item, "mc_ruin_prob");
            cfg.holdout_sharpe = get_double(item, "holdout_sharpe");
            cfg.holdout_trades = (int)get_double(item, "holdout_trades");
            cJSON* params = cJSON_GetObjectItem(item, "params");
            if (cJSON_IsObject(params)) {
                cJSON* child = params->child;
                while (child) {
                    if (cJSON_IsNumber(child)) cfg.params[child->string] = child->valuedouble;
                    child = child->next;
                }
            }
            if (!cfg.name.empty()) s.strategies.push_back(cfg);
        }
    }
    
    cJSON* rets = cJSON_GetObjectItem(root, "recent_trade_returns");
    if (cJSON_IsArray(rets)) {
        int n = cJSON_GetArraySize(rets);
        for (int i = 0; i < n; ++i) {
            cJSON* v = cJSON_GetArrayItem(rets, i);
            if (cJSON_IsNumber(v)) s.recent_trade_returns.push_back(v->valuedouble);
        }
    }
    cJSON* eqc = cJSON_GetObjectItem(root, "equity_curve");
    if (cJSON_IsArray(eqc)) {
        int n = cJSON_GetArraySize(eqc);
        for (int i = 0; i < n; ++i) {
            cJSON* v = cJSON_GetArrayItem(eqc, i);
            if (cJSON_IsNumber(v)) s.equity_curve.push_back(v->valuedouble);
        }
    }
    cJSON_Delete(root);
    return true;
}

static void save_state(const BotState& s) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "equity", s.equity);
    cJSON_AddNumberToObject(root, "starting_equity", s.starting_equity);
    cJSON_AddNumberToObject(root, "peak_equity", s.peak_equity);
    cJSON_AddNumberToObject(root, "total_fees_paid", s.total_fees_paid);
    cJSON_AddNumberToObject(root, "trading_halted_until", (double)s.trading_halted_until);
    cJSON_AddNumberToObject(root, "last_run", (double)s.last_run);
    
    cJSON* pos = cJSON_CreateArray();
    for (auto& p : s.positions) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "symbol", p.symbol.c_str());
        cJSON_AddStringToObject(item, "side", p.side.c_str());
        cJSON_AddStringToObject(item, "strategy", p.strategy.c_str());
        cJSON_AddStringToObject(item, "exchange", p.exchange.c_str());
        cJSON_AddNumberToObject(item, "quantity", p.quantity);
        cJSON_AddNumberToObject(item, "entry_price", p.entry_price);
        cJSON_AddNumberToObject(item, "sl", p.sl);
        cJSON_AddNumberToObject(item, "tp", p.tp);
        cJSON_AddNumberToObject(item, "trailing", p.trailing);
        cJSON_AddNumberToObject(item, "timestamp", (double)p.timestamp);
        cJSON_AddItemToArray(pos, item);
    }
    cJSON_AddItemToObject(root, "positions", pos);
    
    cJSON* strats = cJSON_CreateArray();
    for (auto& cfg : s.strategies) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", cfg.name.c_str());
        cJSON_AddStringToObject(item, "symbol", cfg.symbol.c_str());
        cJSON_AddNumberToObject(item, "exchange", (int)cfg.exchange);
        cJSON_AddBoolToObject(item, "active", cfg.active);
        cJSON_AddNumberToObject(item, "mc_ruin_prob", cfg.mc_ruin_prob);
        cJSON_AddNumberToObject(item, "holdout_sharpe", cfg.holdout_sharpe);
        cJSON_AddNumberToObject(item, "holdout_trades", cfg.holdout_trades);
        cJSON* params = cJSON_CreateObject();
        for (auto& kv : cfg.params) cJSON_AddNumberToObject(params, kv.first.c_str(), kv.second);
        cJSON_AddItemToObject(item, "params", params);
        cJSON_AddItemToArray(strats, item);
    }
    cJSON_AddItemToObject(root, "strategies", strats);
    
    cJSON* rets = cJSON_CreateArray();
    for (double v : s.recent_trade_returns) cJSON_AddItemToArray(rets, cJSON_CreateNumber(v));
    cJSON_AddItemToObject(root, "recent_trade_returns", rets);
    
    cJSON* eqc = cJSON_CreateArray();
    for (double v : s.equity_curve) cJSON_AddItemToArray(eqc, cJSON_CreateNumber(v));
    cJSON_AddItemToObject(root, "equity_curve", eqc);
    
    char* raw = cJSON_Print(root);
    write_file(STATE_FILE, raw);
    cJSON_free(raw);
    cJSON_Delete(root);
}

// ----------------------------------------------------------------------
// Indicadores & Monte Carlo
// ----------------------------------------------------------------------
static double calc_atr(const std::vector<Candle>& candles, size_t idx, size_t period = 14) {
    if (idx < period) return 0.0;
    double sum = 0.0;
    for (size_t i = idx - period + 1; i <= idx; ++i) {
        double tr = std::max({candles[i].high - candles[i].low,
                              std::abs(candles[i].high - candles[i-1].close),
                              std::abs(candles[i].low - candles[i-1].close)});
        sum += tr;
    }
    return sum / period;
}
static double calc_zscore(const std::vector<Candle>& candles, size_t idx, size_t period) {
    if (idx < period) return 0.0;
    double sum = 0.0, sq = 0.0;
    for (size_t j = idx - period; j < idx; ++j) sum += candles[j].close;
    double mean = sum / period;
    for (size_t j = idx - period; j < idx; ++j) sq += (candles[j].close - mean)*(candles[j].close - mean);
    double stdev = std::sqrt(sq / period);
    return (stdev < 1e-9) ? 0.0 : (candles[idx].close - mean) / stdev;
}
static double calc_ema(const std::vector<Candle>& candles, size_t idx, size_t period) {
    if (idx < period) return candles[idx].close;
    double k = 2.0 / (period + 1);
    double ema = candles[0].close;
    for (size_t i = 1; i <= idx; ++i) ema = candles[i].close * k + ema * (1.0 - k);
    return ema;
}
static double calc_rsi(const std::vector<Candle>& candles, size_t idx, size_t period = 14) {
    if (idx < period + 1) return 50.0;
    double gain = 0, loss = 0;
    for (size_t i = idx - period + 1; i <= idx; ++i) {
        double diff = candles[i].close - candles[i-1].close;
        if (diff > 0) gain += diff; else loss -= diff;
    }
    if (loss == 0) return 100.0;
    double rs = gain / loss;
    return 100.0 - (100.0 / (1.0 + rs));
}
static std::vector<double> compute_returns(const std::vector<Candle>& candles, size_t lookback = 200) {
    std::vector<double> rets;
    if (candles.size() < 2) return rets;
    size_t start = (candles.size() > lookback) ? candles.size() - lookback : 1;
    for (size_t i = start; i < candles.size(); ++i)
        rets.push_back((candles[i].close - candles[i-1].close) / candles[i-1].close);
    return rets;
}
static double pearson_corr(const std::vector<double>& a, const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    if (n < 20) return 0.0;
    double ma = 0.0, mb = 0.0;
    for (size_t i = a.size()-n; i < a.size(); ++i) { ma += a[i]; mb += b[i]; }
    ma /= n; mb /= n;
    double cov = 0.0, va = 0.0, vb = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double da = a[a.size()-n+i] - ma, db = b[b.size()-n+i] - mb;
        cov += da*db; va += da*da; vb += db*db;
    }
    return (va < 1e-12 || vb < 1e-12) ? 0.0 : cov / std::sqrt(va * vb);
}
static double monte_carlo_ruin_prob(const std::vector<double>& trade_returns, double capital = 1000.0, double ruin_threshold = 0.70) {
    if (trade_returns.size() < 5) return 100.0;
    int ruin_count = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int sim = 0; sim < MONTE_CARLO_ITERATIONS; ++sim) {
        std::vector<double> shuffled = trade_returns;
        std::shuffle(shuffled.begin(), shuffled.end(), gen);
        double sim_cap = capital;
        for (double ret : shuffled) {
            sim_cap *= (1.0 + ret);
            if (sim_cap <= 0.01) sim_cap = 0.01;
            if (sim_cap <= capital * ruin_threshold) { ruin_count++; break; }
        }
    }
    return (double)ruin_count / MONTE_CARLO_ITERATIONS * 100.0;
}
static std::vector<Candle> fetch_candles(Exchange e, const std::string& symbol) {
    std::vector<Candle> out;
    auto* cfg = get_exchange(e);
    if (!cfg) return out;
    std::string url = cfg->base_url + "/api/v1/contract/kline/" + symbol + "?interval=Min60&limit=2000";
    std::string resp = http_request(url);
    if (resp.empty()) return out;
    auto extract_arr = [&resp](const std::string& key) -> std::vector<double> {
        std::vector<double> v;
        std::string tag = "\"" + key + "\":[";
        size_t start = resp.find(tag);
        if (start == std::string::npos) return v;
        start += tag.size();
        size_t end = resp.find("]", start);
        if (end == std::string::npos) return v;
        std::string arr = resp.substr(start, end - start);
        size_t pos = 0;
        while (pos < arr.size()) {
            size_t comma = arr.find(",", pos);
            std::string num = (comma == std::string::npos) ? arr.substr(pos) : arr.substr(pos, comma - pos);
            if (!num.empty()) v.push_back(std::stod(num));
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
        return v;
    };
    auto t = extract_arr("time");
    auto h = extract_arr("high");
    auto l = extract_arr("low");
    auto c = extract_arr("close");
    size_t n = std::min({t.size(), h.size(), l.size(), c.size()});
    for (size_t i = 0; i < n; ++i)
        out.push_back({(long long)(t[i]*1000), h[i], l[i], c[i]});
    return out;
}

// ======================================================================
// ESTRATÉGIAS (6 completas, sem resumos)
// ======================================================================
class StrategyBase {
public:
    virtual ~StrategyBase() = default;
    virtual std::string name() const = 0;
    virtual bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                          std::map<std::string,double>& best_params,
                          std::vector<double>& holdout_trade_rets,
                          double& holdout_sharpe, int& holdout_trades,
                          double& mc_ruin_prob) = 0;
    virtual Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                            size_t idx, double equity, const std::vector<OpenPos>& positions) = 0;
    void set_params(const std::map<std::string,double>& p) { params_ = p; }
protected:
    std::map<std::string,double> params_;
    virtual void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                                const std::map<std::string,double>& params,
                                std::vector<double>& trades) = 0;
};

// --- Estratégia 1: Trend Exhaustion ---
class TrendExhaustion : public StrategyBase {
public:
    std::string name() const override { return "trend_exhaustion"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap = 1000.0, pos = 0, entry = 0;
        size_t period = (size_t)params.at("period");
        double th = params.at("threshold");
        double am = params.at("atr_mult");
        for (size_t i = start; i < end && i < candles.size(); ++i) {
            if (i < period + 10) continue;
            double z = calc_zscore(candles, i, period);
            double atr = calc_atr(candles, i);
            if (atr <= 0) continue;
            if (pos == 0) {
                if (z < -th) { pos = (cap * 0.02) / (atr * am); entry = candles[i].close; }
                else if (z > th) { pos = -(cap * 0.02) / (atr * am); entry = candles[i].close; }
            } else {
                double pnl = pos * (candles[i].close - entry);
                double sl_dist = atr * am;
                bool exit = (pos > 0 && candles[i].low <= entry - sl_dist) ||
                            (pos < 0 && candles[i].high >= entry + sl_dist) ||
                            (pos > 0 && candles[i].high >= entry + sl_dist * 2) ||
                            (pos < 0 && candles[i].low <= entry - sl_dist * 2);
                if (exit || i == end - 1) { cap += pnl; trades.push_back(pnl / 1000.0); pos = 0; }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if (candles.size() < 200 || train_end < 200) return false;
        std::vector<int> periods = {8,10,13,20};
        std::vector<double> ths = {0.6,0.8,1.0,1.2};
        std::vector<double> ams = {1.4,1.7,2.0,2.3};
        double best_score = -1e9;
        for (int p : periods) for (double th : ths) for (double am : ams) {
            std::map<std::string,double> tp;
            tp["period"] = p; tp["threshold"] = th; tp["atr_mult"] = am;
            std::vector<double> fold_rets;
            size_t fold_sz = train_end / 4;
            for (size_t f = 0; f + 2 <= 4; ++f) {
                size_t ts = fold_sz*(f+1), te = std::min(train_end, fold_sz*(f+2));
                if (ts >= te) continue;
                std::vector<double> st;
                backtest_slice(candles, ts, te, tp, st);
                fold_rets.insert(fold_rets.end(), st.begin(), st.end());
            }
            if (fold_rets.size() < 5) continue;
            double avg = std::accumulate(fold_rets.begin(), fold_rets.end(), 0.0) / fold_rets.size();
            if (avg * fold_rets.size() > best_score) { best_score = avg * fold_rets.size(); bp = tp; }
        }
        if (bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles, train_end, candles.size()-1, bp, hrets);
        htrades = hrets.size();
        if (htrades < MIN_TRADES_HOLDOUT) return false;
        double mean_ret = std::accumulate(hrets.begin(), hrets.end(), 0.0) / htrades;
        double var = 0;
        for (double r : hrets) var += (r - mean_ret)*(r - mean_ret);
        double std_ret = std::sqrt(var / htrades);
        hsharpe = (std_ret > 0) ? mean_ret / std_ret * std::sqrt(htrades) : 0;
        mc_ruin = monte_carlo_ruin_prob(hrets);
        return hsharpe >= MIN_HOLDOUT_SHARPE && mc_ruin <= MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name = name(); s.exchange = e;
        if (idx < 101) return s;
        double z = calc_zscore(candles, idx, (size_t)params_["period"]);
        double atr = calc_atr(candles, idx);
        if (z < -params_["threshold"]) s.direction = "LONG";
        else if (z > params_["threshold"]) s.direction = "SHORT";
        else return s;
        double stop_dist = atr * params_["atr_mult"];
        s.entry = get_current_price(e, symbol);
        s.stop_loss = (s.direction=="LONG") ? s.entry - stop_dist : s.entry + stop_dist;
        s.take_profit = (s.direction=="LONG") ? s.entry + stop_dist * 2 : s.entry - stop_dist * 2;
        s.confidence = std::min(90.0, 40.0 + std::abs(z) * 10.0);
        s.risk_percent = std::min(MAX_RISK_PER_TRADE, 0.02);
        return s;
    }
};

// --- Estratégia 2: EMA Trend Follow ---
class EMATrendFollow : public StrategyBase {
public:
    std::string name() const override { return "ema_trend"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap = 1000.0, pos = 0, entry = 0;
        size_t fast = (size_t)params.at("fast_period"), slow = (size_t)params.at("slow_period");
        double am = params.at("atr_mult");
        for (size_t i = start; i < end && i < candles.size(); ++i) {
            if (i < slow + 5) continue;
            double ema_fast = calc_ema(candles, i, fast), ema_slow = calc_ema(candles, i, slow);
            double atr = calc_atr(candles, i);
            if (atr <= 0) continue;
            if (pos == 0) {
                if (ema_fast > ema_slow) { pos = (cap * 0.02) / (atr * am); entry = candles[i].close; }
            } else {
                double pnl = pos * (candles[i].close - entry);
                double sl_dist = atr * am;
                if (ema_fast < ema_slow || candles[i].low <= entry - sl_dist || i == end - 1 ||
                    candles[i].high >= entry + sl_dist * 3) {
                    cap += pnl; trades.push_back(pnl / 1000.0); pos = 0;
                }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if (candles.size() < 200 || train_end < 200) return false;
        std::vector<std::pair<int,int>> periods = {{10,30},{20,50},{30,80},{40,100}};
        std::vector<double> ams = {1.5,2.0,2.5};
        double best_score = -1e9;
        for (auto [f,s] : periods) for (double am : ams) {
            std::map<std::string,double> tp;
            tp["fast_period"]=f; tp["slow_period"]=s; tp["atr_mult"]=am;
            std::vector<double> fold_rets;
            size_t fold_sz = train_end / 4;
            for (size_t fold=0; fold+2<=4; ++fold) {
                size_t ts=fold_sz*(fold+1), te=std::min(train_end,fold_sz*(fold+2));
                if(ts>=te) continue;
                std::vector<double> st;
                backtest_slice(candles,ts,te,tp,st);
                fold_rets.insert(fold_rets.end(),st.begin(),st.end());
            }
            if(fold_rets.size()<5) continue;
            double avg = std::accumulate(fold_rets.begin(),fold_rets.end(),0.0)/fold_rets.size();
            if(avg*fold_rets.size()>best_score) { best_score=avg*fold_rets.size(); bp=tp; }
        }
        if(bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles,train_end,candles.size()-1,bp,hrets);
        htrades=hrets.size();
        if(htrades<MIN_TRADES_HOLDOUT) return false;
        double mean_ret = std::accumulate(hrets.begin(),hrets.end(),0.0)/htrades, var=0;
        for(double r:hrets) var+=(r-mean_ret)*(r-mean_ret);
        double std_ret=std::sqrt(var/htrades);
        hsharpe=(std_ret>0)?mean_ret/std_ret*std::sqrt(htrades):0;
        mc_ruin=monte_carlo_ruin_prob(hrets);
        return hsharpe>=MIN_HOLDOUT_SHARPE && mc_ruin<=MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name=name(); s.exchange=e;
        if(idx<60) return s;
        double fast=calc_ema(candles,idx,(size_t)params_["fast_period"]);
        double slow=calc_ema(candles,idx,(size_t)params_["slow_period"]);
        if(fast<=slow) return s;
        double atr=calc_atr(candles,idx), stop_dist=atr*params_["atr_mult"];
        s.entry=get_current_price(e,symbol);
        s.direction="LONG";
        s.stop_loss=s.entry-stop_dist; s.take_profit=s.entry+stop_dist*3;
        s.confidence=55+(fast-slow)/slow*100;
        s.risk_percent=std::min(MAX_RISK_PER_TRADE,0.02);
        return s;
    }
};

// --- Estratégia 3: RSI Mean Reversion ---
class RSIMeanRev : public StrategyBase {
public:
    std::string name() const override { return "rsi_mean_rev"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap=1000.0,pos=0,entry=0;
        size_t period=(size_t)params.at("rsi_period");
        double oversold=params.at("oversold"), overbought=params.at("overbought"), am=params.at("atr_mult");
        for(size_t i=start;i<end&&i<candles.size();++i) {
            if(i<period+5) continue;
            double rsi=calc_rsi(candles,i,period), atr=calc_atr(candles,i);
            if(atr<=0) continue;
            if(pos==0) {
                if(rsi<oversold) { pos=(cap*0.02)/(atr*am); entry=candles[i].close; }
                else if(rsi>overbought) { pos=-(cap*0.02)/(atr*am); entry=candles[i].close; }
            } else {
                double pnl=pos*(candles[i].close-entry), sl_dist=atr*am;
                bool exit=(pos>0&&(rsi>50||candles[i].low<=entry-sl_dist))||
                          (pos<0&&(rsi<50||candles[i].high>=entry+sl_dist))||
                          (pos>0&&candles[i].high>=entry+sl_dist*2)||
                          (pos<0&&candles[i].low<=entry-sl_dist*2);
                if(exit||i==end-1) { cap+=pnl; trades.push_back(pnl/1000.0); pos=0; }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if(candles.size()<200||train_end<200) return false;
        std::vector<int> periods={10,14,20};
        std::vector<double> overs={25,30,35}, overs_b={65,70,75}, ams={1.5,2.0};
        double best_score=-1e9;
        for(int p:periods) for(double ov:overs) for(double ob:overs_b) for(double am:ams) {
            std::map<std::string,double> tp;
            tp["rsi_period"]=p; tp["oversold"]=ov; tp["overbought"]=ob; tp["atr_mult"]=am;
            std::vector<double> fold_rets;
            size_t fold_sz=train_end/4;
            for(size_t f=0;f+2<=4;++f) {
                size_t ts=fold_sz*(f+1), te=std::min(train_end,fold_sz*(f+2));
                if(ts>=te) continue;
                std::vector<double> st;
                backtest_slice(candles,ts,te,tp,st);
                fold_rets.insert(fold_rets.end(),st.begin(),st.end());
            }
            if(fold_rets.size()<5) continue;
            double avg=std::accumulate(fold_rets.begin(),fold_rets.end(),0.0)/fold_rets.size();
            if(avg*fold_rets.size()>best_score) { best_score=avg*fold_rets.size(); bp=tp; }
        }
        if(bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles,train_end,candles.size()-1,bp,hrets);
        htrades=hrets.size();
        if(htrades<MIN_TRADES_HOLDOUT) return false;
        double mean_ret=std::accumulate(hrets.begin(),hrets.end(),0.0)/htrades, var=0;
        for(double r:hrets) var+=(r-mean_ret)*(r-mean_ret);
        double std_ret=std::sqrt(var/htrades);
        hsharpe=(std_ret>0)?mean_ret/std_ret*std::sqrt(htrades):0;
        mc_ruin=monte_carlo_ruin_prob(hrets);
        return hsharpe>=MIN_HOLDOUT_SHARPE && mc_ruin<=MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name=name(); s.exchange=e;
        if(idx<20) return s;
        double rsi=calc_rsi(candles,idx,(size_t)params_["rsi_period"]), atr=calc_atr(candles,idx);
        double stop_dist=atr*params_["atr_mult"];
        s.entry=get_current_price(e,symbol);
        if(rsi<params_["oversold"]) { s.direction="LONG"; s.stop_loss=s.entry-stop_dist; s.take_profit=s.entry+stop_dist*2; }
        else if(rsi>params_["overbought"]) { s.direction="SHORT"; s.stop_loss=s.entry+stop_dist; s.take_profit=s.entry-stop_dist*2; }
        else return s;
        s.confidence=std::min(85.0,40.0+(rsi<30?30-rsi:rsi-70));
        s.risk_percent=std::min(MAX_RISK_PER_TRADE,0.018);
        return s;
    }
};

// --- Estratégia 4: Bollinger Squeeze ---
class BollingerSqueeze : public StrategyBase {
public:
    std::string name() const override { return "bollinger_squeeze"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap=1000.0,pos=0,entry=0;
        size_t period=(size_t)params.at("period");
        double std_mult=params.at("std_mult"), am=params.at("atr_mult");
        for(size_t i=start;i<end&&i<candles.size();++i) {
            if(i<period+5) continue;
            double sum=0,sq=0;
            for(size_t j=i-period;j<=i;++j) sum+=candles[j].close;
            double mean=sum/(period+1);
            for(size_t j=i-period;j<=i;++j) sq+=(candles[j].close-mean)*(candles[j].close-mean);
            double std=std::sqrt(sq/(period+1));
            double upper=mean+std*std_mult, lower=mean-std*std_mult;
            double close=candles[i].close, atr=calc_atr(candles,i);
            if(atr<=0) continue;
            if(pos==0) {
                if(close<lower) { pos=(cap*0.02)/(atr*am); entry=close; }
                else if(close>upper) { pos=-(cap*0.02)/(atr*am); entry=close; }
            } else {
                double pnl=pos*(close-entry), sl_dist=atr*am;
                bool exit=(pos>0&&close>=mean)||(pos<0&&close<=mean)||
                          (pos>0&&candles[i].low<=entry-sl_dist)||(pos<0&&candles[i].high>=entry+sl_dist);
                if(exit||i==end-1) { cap+=pnl; trades.push_back(pnl/1000.0); pos=0; }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if (candles.size() < 200 || train_end < 200) return false;
        std::vector<int> periods = {15,20,25};
        std::vector<double> std_mults = {1.5,2.0,2.5};
        std::vector<double> ams = {1.5,2.0};
        double best_score = -1e9;
        for (int p : periods) for (double sm : std_mults) for (double am : ams) {
            std::map<std::string,double> tp;
            tp["period"] = p; tp["std_mult"] = sm; tp["atr_mult"] = am;
            std::vector<double> fold_rets;
            size_t fold_sz = train_end / 4;
            for (size_t f = 0; f + 2 <= 4; ++f) {
                size_t ts = fold_sz * (f+1), te = std::min(train_end, fold_sz * (f+2));
                if (ts >= te) continue;
                std::vector<double> st;
                backtest_slice(candles, ts, te, tp, st);
                fold_rets.insert(fold_rets.end(), st.begin(), st.end());
            }
            if (fold_rets.size() < 5) continue;
            double avg = std::accumulate(fold_rets.begin(), fold_rets.end(), 0.0) / fold_rets.size();
            if (avg * fold_rets.size() > best_score) { best_score = avg * fold_rets.size(); bp = tp; }
        }
        if (bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles, train_end, candles.size()-1, bp, hrets);
        htrades = hrets.size();
        if (htrades < MIN_TRADES_HOLDOUT) return false;
        double mean_ret = std::accumulate(hrets.begin(), hrets.end(), 0.0) / htrades;
        double var = 0;
        for (double r : hrets) var += (r - mean_ret) * (r - mean_ret);
        double std_ret = std::sqrt(var / htrades);
        hsharpe = (std_ret > 0) ? mean_ret / std_ret * std::sqrt(htrades) : 0;
        mc_ruin = monte_carlo_ruin_prob(hrets);
        return hsharpe >= MIN_HOLDOUT_SHARPE && mc_ruin <= MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name = name(); s.exchange = e;
        if (idx < 25) return s;
        size_t period = (size_t)params_["period"];
        double sum = 0, sq = 0;
        for (size_t i = idx - period; i <= idx; ++i) sum += candles[i].close;
        double mean = sum / (period + 1);
        for (size_t i = idx - period; i <= idx; ++i) sq += (candles[i].close - mean) * (candles[i].close - mean);
        double std = std::sqrt(sq / (period + 1));
        double upper = mean + std * params_["std_mult"], lower = mean - std * params_["std_mult"];
        double close = candles[idx].close, atr = calc_atr(candles, idx);
        double stop_dist = atr * params_["atr_mult"];
        s.entry = get_current_price(e, symbol);
        if (close < lower) { s.direction = "LONG"; s.stop_loss = s.entry - stop_dist; s.take_profit = mean; }
        else if (close > upper) { s.direction = "SHORT"; s.stop_loss = s.entry + stop_dist; s.take_profit = mean; }
        else return s;
        s.confidence = 50; s.risk_percent = std::min(MAX_RISK_PER_TRADE, 0.02);
        return s;
    }
};

// --- Estratégia 5: SuperTrend ---
class SuperTrend : public StrategyBase {
public:
    std::string name() const override { return "supertrend"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap = 1000.0, pos = 0, entry = 0;
        int atr_period = (int)params.at("atr_period");
        double multiplier = params.at("multiplier");
        double upper = 0, lower = 0;
        bool uptrend = false;
        for (size_t i = start; i < end && i < candles.size(); ++i) {
            if (i < (size_t)atr_period + 1) continue;
            double atr = calc_atr(candles, i, atr_period);
            double hl2 = (candles[i].high + candles[i].low) / 2.0;
            if (i == start + atr_period) {
                double basic_upper = hl2 + multiplier * atr;
                double basic_lower = hl2 - multiplier * atr;
                upper = basic_upper; lower = basic_lower;
                uptrend = (candles[i].close > upper);
            } else {
                double basic_upper = hl2 + multiplier * atr;
                double basic_lower = hl2 - multiplier * atr;
                upper = basic_upper < upper || candles[i-1].close > upper ? basic_upper : upper;
                lower = basic_lower > lower || candles[i-1].close < lower ? basic_lower : lower;
                if (candles[i].close > upper) uptrend = true;
                else if (candles[i].close < lower) uptrend = false;
            }
            if (pos == 0) {
                if (uptrend) { pos = (cap * 0.02) / (atr * multiplier); entry = candles[i].close; }
            } else {
                if (!uptrend) {
                    double pnl = pos * (candles[i].close - entry);
                    cap += pnl; trades.push_back(pnl / 1000.0); pos = 0;
                }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if (candles.size() < 200 || train_end < 200) return false;
        std::vector<int> atr_p = {10, 14};
        std::vector<double> mults = {2.0, 3.0};
        double best_score = -1e9;
        for (int ap : atr_p) for (double m : mults) {
            std::map<std::string,double> tp;
            tp["atr_period"] = ap; tp["multiplier"] = m;
            std::vector<double> fold_rets;
            size_t fold_sz = train_end / 4;
            for (size_t f = 0; f + 2 <= 4; ++f) {
                size_t ts = fold_sz * (f+1), te = std::min(train_end, fold_sz * (f+2));
                if (ts >= te) continue;
                std::vector<double> st;
                backtest_slice(candles, ts, te, tp, st);
                fold_rets.insert(fold_rets.end(), st.begin(), st.end());
            }
            if (fold_rets.size() < 5) continue;
            double avg = std::accumulate(fold_rets.begin(), fold_rets.end(), 0.0) / fold_rets.size();
            if (avg * fold_rets.size() > best_score) { best_score = avg * fold_rets.size(); bp = tp; }
        }
        if (bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles, train_end, candles.size()-1, bp, hrets);
        htrades = hrets.size();
        if (htrades < MIN_TRADES_HOLDOUT) return false;
        double mean_ret = std::accumulate(hrets.begin(), hrets.end(), 0.0) / htrades;
        double var = 0;
        for (double r : hrets) var += (r - mean_ret)*(r - mean_ret);
        double std_ret = std::sqrt(var / htrades);
        hsharpe = (std_ret > 0) ? mean_ret / std_ret * std::sqrt(htrades) : 0;
        mc_ruin = monte_carlo_ruin_prob(hrets);
        return hsharpe >= MIN_HOLDOUT_SHARPE && mc_ruin <= MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name = name(); s.exchange = e;
        if (idx < 20) return s;
        int atr_period = (int)params_["atr_period"];
        double multiplier = params_["multiplier"];
        double atr = calc_atr(candles, idx, atr_period);
        double hl2 = (candles[idx].high + candles[idx].low) / 2.0;
        double basic_upper = hl2 + multiplier * atr;
        if (candles[idx].close > basic_upper) {
            s.direction = "LONG";
            s.entry = get_current_price(e, symbol);
            s.stop_loss = s.entry - atr * multiplier;
            s.take_profit = s.entry + atr * multiplier * 3;
            s.confidence = 60;
            s.risk_percent = std::min(MAX_RISK_PER_TRADE, 0.02);
        }
        return s;
    }
};

// --- Estratégia 6: Momentum Scalper ---
class MomentumScalper : public StrategyBase {
public:
    std::string name() const override { return "momentum_scalper"; }
    void backtest_slice(const std::vector<Candle>& candles, size_t start, size_t end,
                        const std::map<std::string,double>& params,
                        std::vector<double>& trades) override {
        double cap = 1000.0, pos = 0, entry = 0;
        int period = (int)params.at("momentum_period");
        double am = params.at("atr_mult");
        for (size_t i = start; i < end && i < candles.size(); ++i) {
            if (i < (size_t)period + 2) continue;
            double momentum = candles[i].close - candles[i - period].close;
            double atr = calc_atr(candles, i, 7);
            if (atr <= 0) continue;
            if (pos == 0) {
                if (momentum > 0) { pos = (cap * 0.015) / (atr * am); entry = candles[i].close; }
            } else {
                if (i - (size_t)entry > 3 || candles[i].close <= entry - atr * am || candles[i].close >= entry + atr * am * 1.5) {
                    double pnl = pos * (candles[i].close - entry);
                    cap += pnl; trades.push_back(pnl / 1000.0); pos = 0;
                }
            }
        }
    }
    bool validate(Exchange e, const std::vector<Candle>& candles, size_t train_end,
                  std::map<std::string,double>& bp, std::vector<double>& hrets,
                  double& hsharpe, int& htrades, double& mc_ruin) override {
        if (candles.size() < 200 || train_end < 200) return false;
        std::vector<int> periods = {3,5,8};
        std::vector<double> ams = {1.0,1.5};
        double best_score = -1e9;
        for (int p : periods) for (double am : ams) {
            std::map<std::string,double> tp;
            tp["momentum_period"] = p; tp["atr_mult"] = am;
            std::vector<double> fold_rets;
            size_t fold_sz = train_end / 4;
            for (size_t f = 0; f + 2 <= 4; ++f) {
                size_t ts = fold_sz * (f+1), te = std::min(train_end, fold_sz * (f+2));
                if (ts >= te) continue;
                std::vector<double> st;
                backtest_slice(candles, ts, te, tp, st);
                fold_rets.insert(fold_rets.end(), st.begin(), st.end());
            }
            if (fold_rets.size() < 5) continue;
            double avg = std::accumulate(fold_rets.begin(), fold_rets.end(), 0.0) / fold_rets.size();
            if (avg * fold_rets.size() > best_score) { best_score = avg * fold_rets.size(); bp = tp; }
        }
        if (bp.empty()) return false;
        hrets.clear();
        backtest_slice(candles, train_end, candles.size()-1, bp, hrets);
        htrades = hrets.size();
        if (htrades < MIN_TRADES_HOLDOUT) return false;
        double mean_ret = std::accumulate(hrets.begin(), hrets.end(), 0.0) / htrades;
        double var = 0;
        for (double r : hrets) var += (r - mean_ret)*(r - mean_ret);
        double std_ret = std::sqrt(var / htrades);
        hsharpe = (std_ret > 0) ? mean_ret / std_ret * std::sqrt(htrades) : 0;
        mc_ruin = monte_carlo_ruin_prob(hrets);
        return hsharpe >= MIN_HOLDOUT_SHARPE && mc_ruin <= MAX_MC_RUIN_PROB;
    }
    Signal generate(Exchange e, const std::string& symbol, const std::vector<Candle>& candles,
                    size_t idx, double equity, const std::vector<OpenPos>& positions) override {
        Signal s; s.strategy_name = name(); s.exchange = e;
        if (idx < 10) return s;
        double momentum = candles[idx].close - candles[idx - (size_t)params_["momentum_period"]].close;
        if (momentum <= 0) return s;
        double atr = calc_atr(candles, idx, 7);
        s.entry = get_current_price(e, symbol);
        s.direction = "LONG";
        s.stop_loss = s.entry - atr * params_["atr_mult"];
        s.take_profit = s.entry + atr * params_["atr_mult"] * 1.5;
        s.confidence = 50;
        s.risk_percent = std::min(MAX_RISK_PER_TRADE, 0.015);
        return s;
    }
};

// ======================================================================
// PortfolioManager
// ======================================================================
class PortfolioManager {
    BotState* state_;
    std::vector<std::unique_ptr<StrategyBase>> strats_;
    std::map<std::string, std::vector<double>> returns_cache_;
    
    double compute_kelly_multiplier() const {
        if (state_->recent_trade_returns.size() < KELLY_WINDOW) return 1.0;
        std::vector<double> window(state_->recent_trade_returns.end() - KELLY_WINDOW,
                                   state_->recent_trade_returns.end());
        double mean = std::accumulate(window.begin(), window.end(), 0.0) / window.size();
        double var = 0;
        for (double r : window) var += (r - mean) * (r - mean);
        double std = std::sqrt(var / window.size());
        double sharpe = (std > 0) ? mean / std * std::sqrt(window.size()) : 0;
        if (sharpe >= KELLY_SHARPE_MAX) return 2.5;
        if (sharpe <= KELLY_SHARPE_MIN) return 0.5;
        return 0.5 + (sharpe - KELLY_SHARPE_MIN) / (KELLY_SHARPE_MAX - KELLY_SHARPE_MIN) * 2.0;
    }
    
    bool correlation_check(const std::string& new_symbol) const {
        if (state_->positions.empty()) return true;
        std::vector<std::pair<std::string, std::vector<double>>> series;
        auto it = returns_cache_.find(new_symbol);
        if (it == returns_cache_.end()) return true;
        series.push_back({new_symbol, it->second});
        for (auto& p : state_->positions) {
            auto it2 = returns_cache_.find(p.symbol);
            if (it2 != returns_cache_.end())
                series.push_back({p.symbol, it2->second});
        }
        double sum_corr = 0; int count = 0;
        for (size_t i = 0; i < series.size(); ++i) {
            for (size_t j = i+1; j < series.size(); ++j) {
                double corr = std::abs(pearson_corr(series[i].second, series[j].second));
                sum_corr += corr; count++;
                if (corr > 0.85) return false;
            }
        }
        if (count == 0) return true;
        return (sum_corr / count) <= CORRELATION_LIMIT;
    }
    
public:
    PortfolioManager(BotState* s) : state_(s) {
        strats_.push_back(std::make_unique<TrendExhaustion>());
        strats_.push_back(std::make_unique<EMATrendFollow>());
        strats_.push_back(std::make_unique<RSIMeanRev>());
        strats_.push_back(std::make_unique<BollingerSqueeze>());
        strats_.push_back(std::make_unique<SuperTrend>());
        strats_.push_back(std::make_unique<MomentumScalper>());
    }
    
    void weekly_reoptimize() {
        std::vector<std::string> symbols = {
            "BTC_USDT","ETH_USDT","SOL_USDT","BNB_USDT","XRP_USDT",
            "DOGE_USDT","AVAX_USDT","SUI_USDT","NEAR_USDT","ADA_USDT",
            "TRX_USDT","LINK_USDT","PEPE_USDT","SHIB_USDT","WLD_USDT",
            "OP_USDT","ARB_USDT","FET_USDT","HYPE_USDT","ORDI_USDT"
        };
        state_->strategies.clear();
        for (auto& ex : exchanges) {
            for (auto& sym : symbols) {
                auto candles = fetch_candles(ex.exchange, sym);
                if (candles.size() < 500) continue;
                size_t train_end = (size_t)(candles.size() * (1.0 - TEST_FRACTION));
                returns_cache_[sym] = compute_returns(candles, 200);
                for (auto& strat : strats_) {
                    std::map<std::string,double> best_params;
                    std::vector<double> holdout_rets;
                    double holdout_sharpe; int holdout_trades; double mc_ruin;
                    if (strat->validate(ex.exchange, candles, train_end, best_params,
                                        holdout_rets, holdout_sharpe, holdout_trades, mc_ruin)) {
                        StrategyCfg cfg;
                        cfg.name = strat->name();
                        cfg.symbol = sym;
                        cfg.exchange = ex.exchange;
                        cfg.params = best_params;
                        cfg.active = true;
                        cfg.holdout_sharpe = holdout_sharpe;
                        cfg.holdout_trades = holdout_trades;
                        cfg.mc_ruin_prob = mc_ruin;
                        state_->strategies.push_back(cfg);
                        std::cout << "  [+] " << ex.name() << ":" << cfg.name << " on " << sym 
                                  << " | Sharpe=" << holdout_sharpe << " | Trades=" << holdout_trades 
                                  << " | MC Ruin=" << mc_ruin << "%\n";
                    }
                }
            }
        }
        std::cout << "Weekly re-opt done. " << state_->strategies.size() << " active configs.\n";
    }
    
    void hourly() {
        sync_positions_with_exchange();
        close_expired_positions();
        update_trailing_stops();
        
        double kelly_mult = compute_kelly_multiplier();
        
        std::map<std::string, StrategyBase*> active_map;
        for (auto& s : strats_)
            for (auto& cfg : state_->strategies)
                if (cfg.name == s->name() && cfg.active) {
                    s->set_params(cfg.params);
                    active_map[cfg.name] = s.get();
                    break;
                }
        
        std::set<std::string> symbols;
        for (auto& cfg : state_->strategies) if (cfg.active) symbols.insert(cfg.symbol);
        
        std::map<std::string, std::vector<Candle>> candles_map;
        for (auto& sym : symbols) {
            for (auto& ex : exchanges) {
                candles_map[sym] = fetch_candles(ex.exchange, sym);
                if (!candles_map[sym].empty()) break;
            }
        }
        
        std::vector<std::pair<std::string, Signal>> raw_signals;
        for (auto& cfg : state_->strategies) {
            if (!cfg.active) continue;
            auto it = active_map.find(cfg.name);
            if (it == active_map.end()) continue;
            auto& candles = candles_map[cfg.symbol];
            if (candles.empty()) continue;
            
            bool has_position = false;
            for (auto& p : state_->positions)
                if (p.symbol == cfg.symbol) { has_position = true; break; }
            if (has_position) continue;
            
            Signal sig = it->second->generate(cfg.exchange, cfg.symbol, candles, candles.size()-1,
                                               state_->equity, state_->positions);
            if (sig.direction != "WAIT") {
                sig.risk_percent = std::min(MAX_RISK_PER_TRADE, sig.risk_percent * kelly_mult);
                sig.risk_percent = std::max(MIN_RISK_PER_TRADE, sig.risk_percent);
                raw_signals.push_back({cfg.symbol, sig});
            }
        }
        
        std::map<std::string, Signal> combined;
        for (auto& [sym, sig] : raw_signals) {
            auto& existing = combined[sym];
            if (existing.direction == "WAIT" || sig.confidence > existing.confidence)
                existing = sig;
        }
        
        int slots_filled = state_->positions.size();
        double total_notional = 0;
        for (auto& p : state_->positions) total_notional += p.quantity * p.entry_price;
        
        for (auto& [sym, sig] : combined) {
            if (slots_filled >= MAX_POSITIONS) break;
            if (!correlation_check(sym)) continue;
            
            double price = sig.entry;
            double stop_pct = std::abs(sig.entry - sig.stop_loss) / price;
            if (stop_pct < 0.002) continue;
            double risk_amount = state_->equity * sig.risk_percent;
            double qty = risk_amount / (stop_pct * price);
            double notional = qty * price;
            if (total_notional + notional > state_->equity * MAX_PORTFOLIO_NOTIONAL) continue;
            
            auto* cfg = get_exchange(sig.exchange);
            if (!cfg) continue;
            
            double trade_fee = 0.0;
            execute_bracket(sig.exchange, sym, (sig.direction=="LONG"?"BUY":"SELL"),
                            qty, sig.entry, sig.stop_loss, sig.take_profit, trade_fee);
            state_->total_fees_paid += trade_fee;
            
            OpenPos pos;
            pos.symbol = sym;
            pos.side = sig.direction;
            pos.strategy = sig.strategy_name;
            pos.exchange = cfg->name();
            pos.quantity = qty;
            pos.entry_price = sig.entry;
            pos.sl = sig.stop_loss;
            pos.tp = sig.take_profit;
            pos.trailing = (sig.direction=="LONG") ? sig.entry - (sig.stop_loss - sig.entry) : sig.entry + (sig.entry - sig.stop_loss);
            pos.timestamp = now_epoch();
            state_->positions.push_back(pos);
            total_notional += notional;
            slots_filled++;
        }
        
        double total_equity = 0;
        for (auto& ex : exchanges) total_equity += get_balance(ex.exchange).equity;
        state_->equity = total_equity;
        if (state_->equity > state_->peak_equity) state_->peak_equity = state_->equity;
    }
    
    void close_expired_positions() {
        auto now = now_epoch();
        std::vector<OpenPos> surviving;
        for (auto& p : state_->positions) {
            if ((now - p.timestamp) > MAX_HOLD_HOURS * 3600) {
                Exchange e = Exchange::MEXC;
                for (auto& ex : exchanges) if (ex.name() == p.exchange) { e = ex.exchange; break; }
                close_position_order(e, p.symbol, p.side, p.quantity);
                double current = get_current_price(e, p.symbol);
                double pnl = (p.side=="LONG" ? 1.0 : -1.0) * (current - p.entry_price) * p.quantity;
                state_->recent_trade_returns.push_back(pnl / state_->equity);
            } else {
                surviving.push_back(p);
            }
        }
        state_->positions = surviving;
    }
    
    void update_trailing_stops() {
        for (auto& p : state_->positions) {
            Exchange e = Exchange::MEXC;
            for (auto& ex : exchanges) if (ex.name() == p.exchange) { e = ex.exchange; break; }
            double current = get_current_price(e, p.symbol);
            if (current <= 0) continue;
            bool moved = false;
            if (p.side == "LONG" && current > p.entry_price) {
                double orig_dist = p.entry_price - p.sl;
                double new_sl = current - orig_dist;
                if (new_sl > p.trailing) { p.trailing = new_sl; p.sl = new_sl; moved = true; }
            } else if (p.side == "SHORT" && current < p.entry_price) {
                double orig_dist = p.sl - p.entry_price;
                double new_sl = current + orig_dist;
                if (new_sl < p.trailing) { p.trailing = new_sl; p.sl = new_sl; moved = true; }
            }
            if (moved) {
                update_mexc_trailing_stop(e, p.symbol, p.side, p.quantity, p.entry_price, p.sl, p.tp);
            }
        }
    }
    
    void sync_positions_with_exchange() {
        std::map<std::string, std::set<std::string>> open_map;
        for (auto& ex : exchanges) {
            auto syms = get_open_symbols(ex.exchange);
            open_map[ex.name()] = std::set<std::string>(syms.begin(), syms.end());
        }
        std::vector<OpenPos> still_open;
        for (auto& p : state_->positions) {
            auto it = open_map.find(p.exchange);
            if (it != open_map.end() && it->second.count(p.symbol))
                still_open.push_back(p);
            else {
                Exchange e = Exchange::MEXC;
                for (auto& ex : exchanges) if (ex.name() == p.exchange) { e = ex.exchange; break; }
                double current = get_current_price(e, p.symbol);
                double pnl = (p.side=="LONG" ? 1.0 : -1.0) * (current - p.entry_price) * p.quantity;
                state_->recent_trade_returns.push_back(pnl / state_->equity);
            }
        }
        state_->positions = still_open;
    }
    
    void daily_update() {
        double total_equity = 0;
        for (auto& ex : exchanges) total_equity += get_balance(ex.exchange).equity;
        state_->equity = total_equity;
        state_->equity_curve.push_back(state_->equity);
        if (state_->equity > state_->peak_equity) state_->peak_equity = state_->equity;
        if (!state_->equity_curve.empty()) {
            double peak = *std::max_element(state_->equity_curve.begin(), state_->equity_curve.end());
            double dd = (peak - state_->equity) / peak;
            if (dd > 0.20) {
                state_->trading_halted_until = now_epoch() + 14 * 86400;
                for (auto& ex : exchanges) cancel_all_orders(ex.exchange);
                for (auto& p : state_->positions) {
                    Exchange e = Exchange::MEXC;
                    for (auto& ex : exchanges) if (ex.name() == p.exchange) { e = ex.exchange; break; }
                    close_position_order(e, p.symbol, p.side, p.quantity);
                }
                state_->positions.clear();
                std::cout << "!!! DRAWDOWN HALT !!! 20% DD. Pausing 14 days.\n";
            }
        }
        if (state_->equity_curve.size() > 90) state_->equity_curve.erase(state_->equity_curve.begin());
        if (state_->recent_trade_returns.size() > 100) state_->recent_trade_returns.erase(state_->recent_trade_returns.begin());
    }
};

// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: alpha_factory {hourly|daily|weekly}\n";
        return 1;
    }
    std::string cmd = argv[1];
    init_exchanges();
    if (exchanges.empty()) {
        std::cerr << "No exchanges configured. Set MEXC_API_KEY and MEXC_API_SECRET.\n";
        return 1;
    }
    BotState state;
    if (!load_state(state)) {
        double total_equity = 0;
        for (auto& ex : exchanges) total_equity += get_balance(ex.exchange).equity;
        state.starting_equity = state.equity = state.peak_equity = total_equity;
        save_state(state);
    }
    if (state.trading_halted_until > 0) {
        if (now_epoch() < state.trading_halted_until) {
            std::cout << "Trading halted. Skipping.\n";
            return 0;
        }
        state.trading_halted_until = 0;
    }
    PortfolioManager pm(&state);
    if (cmd == "hourly") pm.hourly();
    else if (cmd == "daily") pm.daily_update();
    else if (cmd == "weekly") pm.weekly_reoptimize();
    state.last_run = now_epoch();
    save_state(state);
    return 0;
}