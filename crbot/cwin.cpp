// sudo apt update && sudo apt install libcurl4-openssl-dev pkg-config
// mkdir -p include/nlohmann && wget -O include/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
// g++ -std=c++20 cwin.cpp  -o cwin -I./include  $(pkg-config  --libs libcurl) -lcrypto -lssl  -lpthread -ldl -Os -s  -ffunction-sections -fdata-sections -Wl,--gc-sections  -fno-rtti -flto && ./cwin

#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <map>
#include <ctime>
#include <random>
#include <curl/curl.h>

#include "cmexc.cpp"

using namespace std;

struct Candle { long long timestamp; double high, low, close; };
struct StrategyParams { size_t period; double threshold, atr_mult; };
struct FeeConfig {
    double maker_fee = 0.0008, taker_fee = 0.0008, borrow_daily = 0.000, candle_hours = 1.0;      
    double borrow_per_candle() const { return borrow_daily * (candle_hours / 24.0); }
};

static constexpr double RISK_PER_TRADE = 0.03; 
static constexpr double SLIPPAGE_PCT = 0.0005;  
static constexpr double tp_factor = 2;
static constexpr double MIN_SL_PCT = 0.03; 
enum class PositionType { NONE, LONG, SHORT };

struct BacktestResult {
    StrategyParams params; double total_return, dollar_gain, total_fees_paid, profit_factor;
    int total_trades, long_trades, short_trades, winning_longs, winning_shorts;
    double win_rate, fitness_score, max_drawdown;
    std::map<std::string, double> daily_profit; std::vector<double> daily_returns;
    std::vector<double> trade_pcts;
	std::vector<std::string> trade_dirs;  // New: Parallel vector for directions
};

struct TrendReading { double ema_short, ema_medium, ema_long; int bullish_votes; bool is_bullish, is_trending; double trend_strength; };
struct LiveSignal {
    std::string direction, regime, warnings; double confidence, entry, stop_loss, take_profit, z_score, atr;
    double order_size_usd, order_size_btc, risk_amount, stop_distance, exact_position_btc, exact_position_usd;
    TrendReading trend; 
};

struct MonteCarloResult { double prob_of_ruin; double worst_case_drawdown; double median_final_equity; };

struct Indicators {
    std::vector<double> atr;
    std::vector<TrendReading> trend;
    std::vector<std::string> regime;
};

// Decoupled structural report containing analysis and risk stress-test results
struct AnalysisResult {
    double oos_return = 0.0;
    double oos_drawdown = 0.0;
    int oos_trades = 0;
    LiveSignal signal;
    MonteCarloResult mc;
    bool success = false;
    std::vector<double> oos_trade_pcts; // Added this line
	std::vector<std::string> oos_trade_dirs; // New: Parallel vector
};

struct symbolstruct {
    // string name;
    // float stepsize;
    // int digits;
    string mexc;
};

vector<symbolstruct> symbols0 = {
    {"SPX500_USDT"},
    {"EUR_USDT"},
    {"JPY_USDT"},
    {"GBP_USDT"},
    {"TLM_USDT"},
    {"CHF_USDT"},
    {"SHIB_USDT"},
    {"HOT_USDT"}
};
vector<symbolstruct> symbols = {
    {"BTC_USDT"},
    {"ETH_USDT"},
    {"TRX_USDT"},
    {"XRP_USDT"},
    {"ADA_USDT"},
    {"SOL_USDT"},
    { "AVAX_USDT"},
    {"SUI_USDT"},
    {"TIA_USDT"},
    {"LINK_USDT"},
    {"DOGE_USDT"},
    {"LTC_USDT"},
    {"NEAR_USDT"},
    {"BNB_USDT"},
    {"USOIL_USDT"},
    {"XAU_USDT"},
    {"PEPE_USDT"}
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb); return size * nmemb;
}

double calculate_atr(const std::vector<Candle>& candles, size_t index, size_t period = 14) {
    if (index < period) return 0.0;
    double sum = 0.0;
    for (size_t i = index - period + 1; i <= index; ++i) {
        sum += std::max({candles[i].high - candles[i].low, std::abs(candles[i].high - candles[i-1].close), std::abs(candles[i].low  - candles[i-1].close)});
    }
    return sum / period;
}

double calculate_z_score(const std::vector<Candle>& candles, size_t idx, size_t period) {
    if (idx < period) return 0.0;
    double sum = 0.0, sq_sum = 0.0;
    for (size_t j = idx - period; j < idx; ++j) sum += candles[j].close;
    double mean = sum / period;
    for (size_t j = idx - period; j < idx; ++j) sq_sum += (candles[j].close - mean) * (candles[j].close - mean);
    double stdev = std::sqrt(sq_sum / period);
    return stdev < 1e-9 ? 0.0 : (candles[idx].close - mean) / stdev;
}

double calculate_confidence(double z_score, double threshold, const TrendReading& trend, const std::string& regime, bool is_long) {
    double score = std::min(30.0, (std::abs(z_score) - threshold) * 15.0);
    score += (trend.bullish_votes == 3 && is_long) || (trend.bullish_votes == 0 && !is_long) ? 25.0 : 0.0;
    score += (trend.bullish_votes == 2 && is_long) || (trend.bullish_votes == 1 && !is_long) ? 15.0 : 0.0;
    if (trend.is_trending) score += 20.0;
    score += trend.trend_strength * 15.0;
    return std::max(0.0, std::min(100.0, score + (regime == "TRENDING" ? 15.0 : (regime == "VOLATILE" ? -15.0 : 0.0))));
}

struct EntryDecision { bool can_long, can_short; double stop_dist; };

EntryDecision evaluate_entry(double price, double z, double atr, double threshold, double atr_mult, const TrendReading& trend) {
    double stop_dist = std::max(atr * atr_mult, price * MIN_SL_PCT); 
    bool can_l = z < -threshold && trend.is_bullish && trend.is_trending && trend.bullish_votes >= 2;
    bool can_s = z >  threshold && !trend.is_bullish && trend.is_trending && trend.bullish_votes <= 1;
    return {can_l, can_s, stop_dist};
}

Indicators precompute_indicators(const std::vector<Candle>& candles, size_t short_p = 20, size_t medium_p = 50, size_t long_p = 100) {
    size_t n = candles.size(); Indicators ind; ind.atr.assign(n, 0.0); ind.trend.assign(n, TrendReading{}); ind.regime.assign(n, "UNKNOWN");
    if (n == 0) return ind;
    for (size_t i = 0; i < n; ++i) ind.atr[i] = calculate_atr(candles, i, 14);
    double ema_s = candles[0].close, ema_m = candles[0].close, ema_l = candles[0].close;
    const double k_s = 2.0 / (short_p + 1), k_m = 2.0 / (medium_p + 1), k_l = 2.0 / (long_p + 1);
    for (size_t i = 1; i < n; ++i) {
        ema_s = candles[i].close * k_s + ema_s * (1.0 - k_s); ema_m = candles[i].close * k_m + ema_m * (1.0 - k_m); ema_l = candles[i].close * k_l + ema_l * (1.0 - k_l);
        if (i < long_p) continue; 
        TrendReading t{}; double price = candles[i].close; t.ema_short = ema_s; t.ema_medium = ema_m; t.ema_long = ema_l;
        t.bullish_votes = (price > t.ema_short) + (price > t.ema_medium) + (price > t.ema_long); t.is_bullish = (t.bullish_votes >= 2);
        t.is_trending = (t.ema_short > t.ema_medium && t.ema_medium > t.ema_long) || (t.ema_short < t.ema_medium && t.ema_medium < t.ema_long);
        double atr = ind.atr[i]; if (atr > 0) t.trend_strength = std::min(1.0, std::abs(price - (t.ema_short + t.ema_medium + t.ema_long) / 3.0) / (atr * 3.0));
        ind.trend[i] = t;
    }
    for (size_t i = 0; i < n; ++i) {
        if (i < 114) { ind.regime[i] = "UNKNOWN"; continue; }
        double atr_pct = (ind.atr[i] / candles[i].close) * 100.0;
        if (atr_pct > 3.5) { ind.regime[i] = "VOLATILE"; continue; }
        ind.regime[i] = (ind.trend[i].is_trending && atr_pct < 2.5) ? "TRENDING" : "RANGING";
    }
    return ind;
}

std::vector<double> precompute_zscore(const std::vector<Candle>& candles, size_t period) {
    size_t n = candles.size(); std::vector<double> z(n, 0.0);
    for (size_t i = 0; i < n; ++i) z[i] = calculate_z_score(candles, i, period);
    return z;
}
BacktestResult run_backtest(const std::vector<Candle>& candles, const StrategyParams& params, const Indicators& ind, const std::vector<double>& z_scores, size_t start_idx, size_t end_idx, const FeeConfig& fees = FeeConfig{}) {
    double capital = 1000.0, initial_capital = capital, position = 0.0, total_fees = 0.0, gross_wins = 0.0, gross_losses = 0.0;
    double entry_price = 0.0, fixed_sl = 0.0, dynamic_tp = 0.0, peak_capital = capital, current_trade_fee = 0.0;
    double current_trade_start_cap = capital; int long_trades = 0, short_trades = 0, winning_longs = 0, winning_shorts = 0;
    PositionType pos_type = PositionType::NONE; std::map<std::string, double> daily_profit; std::vector<double> daily_returns; std::vector<double> trade_pcts; std::vector<std::string> trade_dirs;

    auto log_daily = [&](long long ts, double pnl) {
        time_t t = ts / 1000; char buf[11]; std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::gmtime(&t));
        double net = pnl - current_trade_fee; daily_profit[buf] += net; daily_returns.push_back(net / initial_capital * 100.0); current_trade_fee = 0.0;
    };
    auto apply_fee = [&](double rate, double notional) { double fee = notional * rate; capital -= fee; total_fees += fee; current_trade_fee += fee; };
    size_t real_start = std::max({(size_t)101, params.period + 1, start_idx}), real_end = std::min(candles.size(), end_idx);

    for (size_t i = real_start; i < real_end; ++i) {
        double price = candles[i].close, atr = ind.atr[i]; const TrendReading& trend = ind.trend[i];
        if (pos_type != PositionType::NONE) apply_fee(fees.borrow_per_candle(), std::abs(position) * price);
        
        if (pos_type == PositionType::NONE) {
            EntryDecision dec = evaluate_entry(price, z_scores[i], atr, params.threshold, params.atr_mult, trend);
            if (dec.can_long) {
                current_trade_start_cap = capital; position = (capital * RISK_PER_TRADE) / dec.stop_dist; entry_price = price * (1.0 + SLIPPAGE_PCT);
                pos_type = PositionType::LONG; fixed_sl = entry_price - dec.stop_dist; dynamic_tp = entry_price + dec.stop_dist * tp_factor; apply_fee(fees.maker_fee, position * entry_price);
            } else if (dec.can_short) {
                current_trade_start_cap = capital; position = (capital * RISK_PER_TRADE) / dec.stop_dist; entry_price = price * (1.0 - SLIPPAGE_PCT);
                pos_type = PositionType::SHORT; fixed_sl = entry_price + dec.stop_dist; dynamic_tp = entry_price - dec.stop_dist * tp_factor; apply_fee(fees.maker_fee, position * entry_price);
            }
        } else {
            bool is_l = (pos_type == PositionType::LONG), exit = false; double pnl = 0.0, exit_price = 0.0;
            
            // --- STRICT BRACKET CAPPING ---
            if (is_l) {
                if (candles[i].low <= fixed_sl) {
                    exit_price = fixed_sl * (1.0 - SLIPPAGE_PCT); exit = true;
                } else if (candles[i].high >= dynamic_tp) {
                    exit_price = dynamic_tp; exit = true;
                }
            } else {
                if (candles[i].high >= fixed_sl) {
                    exit_price = fixed_sl * (1.0 + SLIPPAGE_PCT); exit = true;
                } else if (candles[i].low <= dynamic_tp) {
                    exit_price = dynamic_tp; exit = true;
                }
            }
            
            if (exit) {
                pnl = position * (is_l ? exit_price - entry_price : entry_price - exit_price);
                double net_pnl = pnl - current_trade_fee; capital += pnl; *(pnl >= 0 ? &gross_wins : &gross_losses) += std::abs(pnl);
                apply_fee(fees.taker_fee, std::abs(position) * exit_price); log_daily(candles[i].timestamp, pnl); 
                trade_pcts.push_back(net_pnl / current_trade_start_cap); 
                trade_dirs.push_back(is_l ? "LONG" : "SHORT");
                if (capital <= 0) capital = 0.01; pos_type = PositionType::NONE; (is_l ? long_trades : short_trades)++; if (pnl > 0) (is_l ? winning_longs : winning_shorts)++;
            }
        }
        if (capital > peak_capital) peak_capital = capital;
    }
    int total_trades = long_trades + short_trades, wins = winning_longs + winning_shorts;
    double win_rate = total_trades > 0 ? (double)wins / total_trades * 100.0 : 0.0, ret = (capital - initial_capital) / initial_capital * 100.0;
    double max_dd = peak_capital > 0 ? (peak_capital - capital) / peak_capital * 100.0 : 0.0;
    return {params, ret, capital - initial_capital, total_fees, gross_losses > 0 ? gross_wins / gross_losses : (gross_wins > 0 ? 99.0 : 0.0),
            total_trades, long_trades, short_trades, winning_longs, winning_shorts, win_rate,
            ret * (win_rate / 100.0) * std::log(total_trades + 1) * (1.0 - max_dd / 100.0 * 0.5), max_dd, daily_profit, daily_returns, trade_pcts, trade_dirs};
}
BacktestResult run_backtest1(const std::vector<Candle>& candles, const StrategyParams& params, const Indicators& ind, const std::vector<double>& z_scores, size_t start_idx, size_t end_idx, const FeeConfig& fees = FeeConfig{}) {
    double capital = 1000.0, initial_capital = capital, position = 0.0, total_fees = 0.0, gross_wins = 0.0, gross_losses = 0.0;
    double entry_price = 0.0, fixed_sl = 0.0, dynamic_tp = 0.0, peak_capital = capital, current_trade_fee = 0.0;
    double current_trade_start_cap = capital; int long_trades = 0, short_trades = 0, winning_longs = 0, winning_shorts = 0;
    PositionType pos_type = PositionType::NONE; std::map<std::string, double> daily_profit; std::vector<double> daily_returns; std::vector<double> trade_pcts;

    auto log_daily = [&](long long ts, double pnl) {
        time_t t = ts / 1000; char buf[11]; std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::gmtime(&t));
        double net = pnl - current_trade_fee; daily_profit[buf] += net; daily_returns.push_back(net / initial_capital * 100.0); current_trade_fee = 0.0;
    };
    auto apply_fee = [&](double rate, double notional) { double fee = notional * rate; capital -= fee; total_fees += fee; current_trade_fee += fee; };
    size_t real_start = std::max({(size_t)101, params.period + 1, start_idx}), real_end = std::min(candles.size(), end_idx);

    for (size_t i = real_start; i < real_end; ++i) {
        double price = candles[i].close, atr = ind.atr[i]; const TrendReading& trend = ind.trend[i];
        if (pos_type != PositionType::NONE) apply_fee(fees.borrow_per_candle(), std::abs(position) * price);
        if (pos_type == PositionType::NONE) {
            EntryDecision dec = evaluate_entry(price, z_scores[i], atr, params.threshold, params.atr_mult, trend);
            if (dec.can_long) {
                current_trade_start_cap = capital; position = (capital * RISK_PER_TRADE) / dec.stop_dist; entry_price = price * (1.0 + SLIPPAGE_PCT);
                pos_type = PositionType::LONG; fixed_sl = entry_price - dec.stop_dist; dynamic_tp = entry_price + dec.stop_dist * tp_factor; apply_fee(fees.maker_fee, position * entry_price);
            } else if (dec.can_short) {
                current_trade_start_cap = capital; position = (capital * RISK_PER_TRADE) / dec.stop_dist; entry_price = price * (1.0 - SLIPPAGE_PCT);
                pos_type = PositionType::SHORT; fixed_sl = entry_price + dec.stop_dist; dynamic_tp = entry_price - dec.stop_dist * tp_factor; apply_fee(fees.maker_fee, position * entry_price);
            }
        } else {
            bool is_l = (pos_type == PositionType::LONG), exit = false; double pnl = 0.0, exit_price = price;
            if ((is_l && candles[i].low <= fixed_sl) || (!is_l && candles[i].high >= fixed_sl)) {
                exit_price = is_l ? fixed_sl * (1.0 - SLIPPAGE_PCT) : fixed_sl * (1.0 + SLIPPAGE_PCT); pnl = position * (is_l ? exit_price - entry_price : entry_price - exit_price); exit = true;
            } else if ((is_l && candles[i].high >= dynamic_tp) || (!is_l && candles[i].low <= dynamic_tp)) {
                exit_price = dynamic_tp; pnl = position * (is_l ? dynamic_tp - entry_price : entry_price - dynamic_tp); exit = true;
            } else if ((is_l && !trend.is_bullish) || (!is_l && trend.is_bullish)) {
                exit_price = is_l ? price * (1.0 - SLIPPAGE_PCT) : price * (1.0 + SLIPPAGE_PCT); pnl = position * (is_l ? exit_price - entry_price : entry_price - exit_price); exit = true;
            } 
            if (exit) {
                double net_pnl = pnl - current_trade_fee; capital += pnl; *(pnl >= 0 ? &gross_wins : &gross_losses) += std::abs(pnl);
                apply_fee(fees.taker_fee, std::abs(position) * exit_price); log_daily(candles[i].timestamp, pnl); trade_pcts.push_back(net_pnl / current_trade_start_cap); 
                if (capital <= 0) capital = 0.01; pos_type = PositionType::NONE; (is_l ? long_trades : short_trades)++; if (pnl > 0) (is_l ? winning_longs : winning_shorts)++;
            }
        }
        if (capital > peak_capital) peak_capital = capital;
    }
    int total_trades = long_trades + short_trades, wins = winning_longs + winning_shorts;
    double win_rate = total_trades > 0 ? (double)wins / total_trades * 100.0 : 0.0, ret = (capital - initial_capital) / initial_capital * 100.0;
    double max_dd = peak_capital > 0 ? (peak_capital - capital) / peak_capital * 100.0 : 0.0;
    return {params, ret, capital - initial_capital, total_fees, gross_losses > 0 ? gross_wins / gross_losses : (gross_wins > 0 ? 99.0 : 0.0),
            total_trades, long_trades, short_trades, winning_longs, winning_shorts, win_rate,
            ret * (win_rate / 100.0) * std::log(total_trades + 1) * (1.0 - max_dd / 100.0 * 0.5), max_dd, daily_profit, daily_returns, trade_pcts};
}

MonteCarloResult run_monte_carlo(const std::vector<double>& actual_trade_pcts, double start_capital, int iterations = 2000) {
    if (actual_trade_pcts.empty()) return {0.0, 0.0, start_capital};
    std::vector<double> final_equities; std::vector<double> max_drawdowns;
    int ruin_count = 0; 
    
    static std::random_device rd; 
    static std::mt19937 g(rd());
    
    for (int sim = 0; sim < iterations; ++sim) {
        std::vector<double> shuffled_pcts = actual_trade_pcts;
        std::shuffle(shuffled_pcts.begin(), shuffled_pcts.end(), g);
        double sim_capital = start_capital, sim_peak = start_capital, sim_max_dd = 0.0;
        bool ruined = false;
        
        for (double pct : shuffled_pcts) {
            sim_capital += (sim_capital * pct); 
            if (sim_capital <= 0.01) sim_capital = 0.01;
            
            if (sim_capital > sim_peak) sim_peak = sim_capital;
            double current_dd = (sim_peak - sim_capital) / sim_peak * 100.0;
            if (current_dd > sim_max_dd) sim_max_dd = current_dd;
            
            // "Ruin" defined as losing 30% or more of starting capital on a single asset stream
            if (sim_capital <= (start_capital * 0.70)) ruined = true;
        }
        if (ruined) ruin_count++;
        final_equities.push_back(sim_capital); max_drawdowns.push_back(sim_max_dd);
    }
    std::sort(final_equities.begin(), final_equities.end());
    std::sort(max_drawdowns.begin(), max_drawdowns.end());
    
    double prob_ruin = ((double)ruin_count / iterations) * 100.0;
    size_t dd_95_idx = static_cast<size_t>(iterations * 0.95);
    return { prob_ruin, max_drawdowns[dd_95_idx], final_equities[iterations / 2] };
}

LiveSignal generate_signal(const std::vector<Candle>& candles, size_t eval_idx, const StrategyParams& params, double free_margin, const Indicators& ind) {
    LiveSignal sig{}; if (eval_idx < 101 || eval_idx >= candles.size()) return sig;
    sig.trend = ind.trend[eval_idx]; sig.regime = ind.regime[eval_idx]; sig.atr = ind.atr[eval_idx]; sig.z_score = calculate_z_score(candles, eval_idx, params.period);
    double price = candles[eval_idx].close;
    EntryDecision dec = evaluate_entry(price, sig.z_score, sig.atr, params.threshold, params.atr_mult, sig.trend);
    sig.stop_distance = dec.stop_dist; sig.risk_amount = free_margin * RISK_PER_TRADE; 
    sig.exact_position_btc = sig.order_size_btc = (dec.stop_dist > 0.0) ? (sig.risk_amount / dec.stop_dist) : 0.0;
    sig.exact_position_usd = sig.order_size_usd = sig.exact_position_btc * price;
    if (sig.trend.bullish_votes == 1 || sig.trend.bullish_votes == 2) sig.warnings += "[WARN] EMA split. ";
    if (sig.regime == "VOLATILE") sig.warnings += "[WARN] Volatile. ";
    if (!sig.trend.is_trending) sig.warnings += "[INFO] Ranging. ";
    bool can_l = dec.can_long, can_s = dec.can_short;
    sig.direction = can_l ? "LONG" : (can_s ? "SHORT" : "WAIT"); sig.entry = price;
    sig.stop_loss = price + (can_l ? -1 : 1) * (can_l || can_s ? dec.stop_dist : 0);
    sig.take_profit = price + (can_l ? 1 : -1) * (can_l || can_s ? dec.stop_dist * tp_factor : 0);
    sig.confidence = (can_l || can_s) ? calculate_confidence(sig.z_score, params.threshold, sig.trend, sig.regime, can_l) : 0.0;
    return sig;
}


std::vector<double> extractArray(const std::string& src, const std::string& key) {
    std::vector<double> out; std::string tag = "\"" + key + "\":["; size_t start = src.find(tag); if (start == std::string::npos) return out;
    start += tag.size(); size_t end = src.find("]", start); if (end == std::string::npos) return out;
    std::string arr = src.substr(start, end - start); size_t pos = 0;
    while (true) {
        size_t comma = arr.find(",", pos); std::string num = (comma == std::string::npos) ? arr.substr(pos) : arr.substr(pos, comma - pos);
        if (!num.empty()) out.push_back(std::stod(num));
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return out;
}

AnalysisResult seek(int idsmb, double equity) {
    AnalysisResult res;
    string symbol = symbols[idsmb].mexc;
    CURL* curl = curl_easy_init();
    std::string readBuffer; std::vector<Candle> candles;

    if (curl) {
        std::string url = "https://contract.mexc.com/api/v1/contract/kline/" + symbols[idsmb].mexc + "?interval=Min60&limit=2000";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        if (curl_easy_perform(curl) == CURLE_OK) {
            std::vector<double> t = extractArray(readBuffer, "time"); std::vector<double> h = extractArray(readBuffer, "high");
            std::vector<double> l = extractArray(readBuffer, "low");  std::vector<double> c = extractArray(readBuffer, "close");
            size_t n = t.size(); candles.reserve(n);
            for (size_t i = 0; i < n; ++i) candles.push_back({(long long)t[i] * 1000LL, h[i], l[i], c[i]});
        } else {
            curl_easy_cleanup(curl); return res;
        }
        curl_easy_cleanup(curl);
    }
	cout<<candles.size()<<" candles of "<<symbol<<"\n";
    if (candles.size() < 150) return res;
    size_t last_closed_idx = candles.size() - 2;
    Indicators ind = precompute_indicators(candles);
    std::map<size_t, std::vector<double>> z_cache;

    StrategyParams best_params = {10, 1.0, 1.5}; 
    double best_robust_fitness = -9999.0; 
    size_t split_idx = static_cast<size_t>(candles.size() * 0.80);

    for (size_t p : {5, 8, 10, 13, 20}) {
        auto& z_scores = z_cache.try_emplace(p, precompute_zscore(candles, p)).first->second;
        for (auto t : {0.5, 0.7, 1.0, 1.2}) {
            for (auto a : {1.2, 1.5, 1.8, 2.2}) {
                BacktestResult is_res = run_backtest(candles, {p, t, a}, ind, z_scores, 0, split_idx);
                if (is_res.total_trades >= 15 && is_res.max_drawdown < 15.0) {
                    double robust_fitness = is_res.total_return * (1.0 - (is_res.max_drawdown / 100.0));
                    if (robust_fitness > best_robust_fitness) {
                        best_robust_fitness = robust_fitness; best_params = {p, t, a};
                    }
                }
            }
        }
    }

    auto& best_z_scores = z_cache[best_params.period];
    BacktestResult oos_metrics = run_backtest(candles, best_params, ind, best_z_scores, split_idx, candles.size() - 1);
    
    // Run the Monte Carlo simulation over the Out-Of-Sample trade returns distribution
    MonteCarloResult mc_res = run_monte_carlo(oos_metrics.trade_pcts, 1000.0, 2000);

    std::cout << "Best Params " << symbol << ": Period=" << best_params.period << " Th=" << best_params.threshold << " ATR=" << best_params.atr_mult << "\n"
              << "OOS Expected Return: " << std::fixed << std::setprecision(2) << oos_metrics.total_return << "% | OOS Drawdown: " << oos_metrics.max_drawdown << "%\n"
              << "Monte Carlo Risk Analysis -> Prob of Ruin: " << mc_res.prob_of_ruin << "% | Worst-Case DD: " << mc_res.worst_case_drawdown << "%\n\n";

	res.oos_return = oos_metrics.total_return;
    res.oos_drawdown = oos_metrics.max_drawdown;
    res.oos_trades = oos_metrics.total_trades;
    res.oos_trade_pcts = oos_metrics.trade_pcts; // Added this line
	res.oos_trade_dirs = oos_metrics.trade_dirs; // Pass directions along
    res.signal = generate_signal(candles, last_closed_idx, best_params, equity, ind);
    res.mc = mc_res;
    res.success = true;
    return res;
}

int main() {
    money moneyei = getUsdtFuturesBalance();
    std::string openPositions = getOpenedFuturesPositions();
    std::cout << "Current Account Equity: " << moneyei.equity << " USDT\n";
    
    vector<std::string> gops = getOpenPositionSymbols();
    cout << "Active Structural Positions: " << gops.size() << "\n\n";
    for(int i=0;i<gops.size();i++){
        cout<<"gops: "<<gops[i]<<"\n";
    }

    struct RankedSymbol { size_t index; AnalysisResult analysis; };
    vector<RankedSymbol> pipeline;

    std::cout << "--- Starting Parallel Screening Pipeline ---\n";
    for (size_t i = 0; i < symbols.size(); i++) {
        AnalysisResult report = seek(i, moneyei.equity);
        if (report.success) {
            pipeline.push_back({i, report});
        }
    }

    // Rank symbols globally based on true historical Out-of-Sample Alpha
    std::sort(pipeline.begin(), pipeline.end(), [](const RankedSymbol& a, const RankedSymbol& b) {
        return a.analysis.oos_return > b.analysis.oos_return;
    });

    const size_t MAX_SIMULTANEOUS_ASSETS = 4; 
    size_t slots_filled = 0;
    double cumulative_verified_oos = 0.0;

    std::cout << "\n------------------ DEPLOYMENT MATRIX ------------------\n";
    for (const auto& item : pipeline) {
        size_t idx = item.index;
        const auto& report = item.analysis;

        if (slots_filled >= MAX_SIMULTANEOUS_ASSETS) {
            std::cout << "[SKIPPED] " << symbols[idx].mexc << " (" << report.oos_return << "%) - Exceeds portfolio limit slots.\n";
            continue;
        }

        // Hard cutoff to completely protect capital from sub-optimal or flat equity curves
        if (report.oos_return <= 0.0) {
            std::cout << "[FILTERED] " << symbols[idx].mexc << " (" << report.oos_return << "%) - Negative or zero edge validation.\n";
            continue;
        }

        // --- MONTE CARLO PROTECTIVE GATE ---
        // If the simulation shows greater than a 5% probability of breaking your capital constraints under variance, walk away.
        if (report.mc.worst_case_drawdown > 12) {
            std::cout << "[RISK-REJECTED] " << symbols[idx].mexc << " | Nominal Return was " << report.oos_return 
                      << "% but Monte Carlo Risk is too high (Ruin Prob: " << report.mc.worst_case_drawdown << "%)\n";
            continue;
        }

        cumulative_verified_oos += report.oos_return;
        std::cout << ">>> [VALIDATED] Slot #" << (slots_filled + 1) << ": Deploying to " << symbols[idx].mexc 
                  << " | OOS: " << report.oos_return << "% | Signal: " << report.signal.direction << "\n";


		// --- PRINT ALL LAST TESTED TRADES ---
        std::cout << "    -> Last Tested Trades (OOS): ";
        if (report.oos_trade_pcts.empty()) {
            std::cout << "None\n";
        } else {
            int start = std::max(0, (int)report.oos_trade_pcts.size() - 10);
            for (size_t t = start; t < report.oos_trade_pcts.size(); ++t) {
                std::string dir = report.oos_trade_dirs[t]; // Pulls direction from parallel vector
                double pct = report.oos_trade_pcts[t] * 100.0; // Pulls PnL %
                
                std::cout << dir << "(";
                if (pct >= 0) std::cout << "+";
                std::cout << std::fixed << std::setprecision(2) << pct << "%) ";
            }
            std::cout << "\n";
        }




        // Execution safely takes place here AFTER passing all hurdles
        if (report.signal.direction != "WAIT") {
            // float stepsize = symbols[idx].stepsize;
            // int d = symbols[idx].digits;
            // double qty = std::floor(report.signal.exact_position_btc / stepsize) * stepsize;
            double qty = report.signal.exact_position_btc;
            double notional = qty * report.signal.entry;
            
            if (qty > 0) {
                printf("    -> EXECUTE ORDER: %s %s | Qty: %f | Entry: %f | SL: %f | TP: %f\n", 
                       symbols[idx].mexc.c_str(), (report.signal.direction == "LONG" ? "BUY" : "SELL"), 
                       qty, report.signal.entry, report.signal.stop_loss, report.signal.take_profit);
                std::cout << "    -> Required Leverage: " << (notional / moneyei.equity) << "x\n";

                if(0)openAtomicBracketFuturesPosition(
                    symbols[idx].mexc,
                    report.signal.direction == "LONG" ? "BUY" : "SELL",
                    qty,
                    report.signal.entry,
                    report.signal.stop_loss,
                    report.signal.take_profit
                );
            }
        } else {
            std::cout << "    -> Market pattern neutral. No live trade trigger generated.\n";
        }
        slots_filled++;
    }
    std::cout << "--------------------------------------------------------\n";
    if (slots_filled > 0) {
        cout << "Portfolio Deployed Slots: " << slots_filled << " / " << MAX_SIMULTANEOUS_ASSETS << "\n";
        cout << "Active Out-of-Sample Combined Expectation: " << cumulative_verified_oos << "%\n";
    } else {
        cout << "Defensive Phase: 0 Slots Allocated. All assets filtered out.\n";
    }
    return 0;
}