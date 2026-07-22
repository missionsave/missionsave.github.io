// sudo apt update && sudo apt install libcurl4-openssl-dev pkg-config libcjson-dev
// g++ -std=c++20 cwin.cpp -o cwin ../common/cJSON.c -I../common $(pkg-config --libs libcurl) -lcrypto -lssl -lpthread -ldl -Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-rtti -flto && ./cwin
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <map>
#include <ctime>
#include <random>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <cJSON.h>
#include "cmexc.cpp"
using namespace std;

struct Candle { long long timestamp; double high, low, close; };
struct StrategyParams { size_t period; double threshold, atr_mult; };
struct FeeConfig {
    // borrow_daily defaults to a conservative non-zero funding assumption; it gets
    // overwritten per-symbol with the real MEXC funding rate in fetch_funding_rate().
    // Leaving this at 0 (as before) silently deletes a real, recurring perp cost from every backtest.
    double maker_fee = 0.0008, taker_fee = 0.0008, borrow_daily = 0.0003, candle_hours = 1.0;
    double borrow_per_candle() const { return borrow_daily * (candle_hours / 24.0); }
};
static constexpr double RISK_PER_TRADE = 0.03;
static constexpr double SLIPPAGE_PCT = 0.0005;
static constexpr double CORRELATION_THRESHOLD = 0.60;
static constexpr double MAX_CORRELATED_RISK = 0.06; // e.g. 2x a single RISK_PER_TRADE

// Fraction of each symbol's candle history reserved as a true holdout. This slice is
// NEVER touched by walk-forward folds, the period/threshold/atr_mult grid search, or the
// outer MIN_SL_PCT/tp_factor scan. It's evaluated exactly once, after every parameter
// has already been locked in, and its result — not the walk-forward number — is what
// gets reported as the "expected" return and used to veto live deployment.
// 25% (not 30%) so the train region still has enough candles per fold to clear the
// trade-count bar below on real, not-always-cooperative market data.
static constexpr double TEST_FRACTION = 0.25;

double MIN_SL_PCT = 0.03;
double tp_factor = 2.0;

// Trimmed from the original 5x4=20 combos to 4x3=12 — still fewer degrees of freedom
// than the original scan, but wide enough to actually find a working combo on real data.
const std::vector<double> SL_CANDIDATES = {0.02, 0.025, 0.03, 0.035};
const std::vector<double> TP_CANDIDATES = {1.5, 2.0, 2.5};

static constexpr double MIN_WINS_PER_DAY = 0.10;

enum class PositionType { NONE, LONG, SHORT };

struct BacktestResult {
    StrategyParams params; double total_return, dollar_gain, total_fees_paid, profit_factor;
    int total_trades, long_trades, short_trades, winning_longs, winning_shorts;
    double win_rate, fitness_score, max_drawdown;
    std::map<std::string, double> daily_profit; std::vector<double> daily_returns;
    std::vector<double> trade_pcts;
    std::vector<std::string> trade_dirs;
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
struct AnalysisResult {
    // oos_* fields below are the TRUE holdout (last TEST_FRACTION of history, never
    // used in any fold or grid search) — this is the only genuinely out-of-sample
    // measurement in the pipeline.
    double oos_return = 0.0;
    double oos_drawdown = 0.0;
    int oos_trades = 0;
    int oos_wins = 0;
    double oos_days = 0.0;
    double holdout_daily_return = 0.0;      // true-holdout return / holdout window days

    // walk_forward_* fields are TRAIN-only (in-sample by construction, used for
    // parameter/SL-TP selection and for live gating that must not touch the holdout).
    double walk_forward_daily_return = 0.0;
    double walk_forward_score = -9999.0;
    double train_wins_per_day = 0.0;

    LiveSignal signal;
    MonteCarloResult mc; // computed from TRAIN walk-forward trades, used for selection/gating
    bool success = false;
    std::vector<double> oos_trade_pcts;
    std::vector<std::string> oos_trade_dirs;
    std::vector<double> recent_returns;
};
struct symbolstruct {
    string mexc;
};
std::vector<symbolstruct> symbols;

struct OpenPosition {
    std::string symbol;
    std::string direction;
    double notional_usd = 0.0;
	std::string createTime;
	float unRealizedPnl=0;
	float entry=0;
	float sl=0;
	float tp=0;
};
struct PortfolioState {
    std::vector<OpenPosition> positions;
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb); return size * nmemb;
}

// Pulls the current funding rate for a symbol from MEXC and converts it to a
// daily-equivalent cost. MEXC settles funding every 8h, so we approximate the
// daily drag as 3x the current rate's magnitude. Falls back to a conservative
// non-zero estimate if the request or parse fails, rather than falling back to 0.
double fetch_funding_rate(const std::string& symbol) {
    const double fallback_daily = 0.0003; // ~3bps/day conservative fallback
    CURL* curl = curl_easy_init();
    if (!curl) return fallback_daily;
    std::string readBuffer;
    std::string url = "https://contract.mexc.com/api/v1/contract/funding_rate/" + symbol;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    double result = fallback_daily;
    if (curl_easy_perform(curl) == CURLE_OK) {
        cJSON* root = cJSON_Parse(readBuffer.c_str());
        if (root) {
            cJSON* data = cJSON_GetObjectItem(root, "data");
            if (data) {
                cJSON* fr = cJSON_GetObjectItem(data, "fundingRate");
                if (fr && cJSON_IsNumber(fr)) result = std::abs(fr->valuedouble) * 3.0;
            }
            cJSON_Delete(root);
        }
    }
    curl_easy_cleanup(curl);
    return result;
}

std::vector<symbolstruct> get_hyperliquid_top_symbols(int top_n) {
    std::vector<symbolstruct> top_symbols;
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if(0)if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.hyperliquid.xyz/info");
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"type\":\"metaAndAssetCtxs\"}");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        if (curl_easy_perform(curl) == CURLE_OK) {
            cJSON* root = cJSON_Parse(readBuffer.c_str());
            if (root != NULL) {
                cJSON* meta = cJSON_GetArrayItem(root, 0);
                cJSON* ctxs = cJSON_GetArrayItem(root, 1);

                if (meta != NULL && ctxs != NULL) {
                    cJSON* universe = cJSON_GetObjectItem(meta, "universe");
                    if (universe != NULL && cJSON_IsArray(universe) && cJSON_IsArray(ctxs)) {
                        struct AssetInfo { std::string name; double volume; };
                        std::vector<AssetInfo> assets;

                        int num_assets = std::min(cJSON_GetArraySize(universe), cJSON_GetArraySize(ctxs));
                        for (int i = 0; i < num_assets; ++i) {
                            cJSON* asset = cJSON_GetArrayItem(universe, i);
                            cJSON* ctx = cJSON_GetArrayItem(ctxs, i);

                            cJSON* name_item = cJSON_GetObjectItem(asset, "name");
                            cJSON* vol_item = cJSON_GetObjectItem(ctx, "dayNtlVlm");

                            if (cJSON_IsString(name_item) && (cJSON_IsString(vol_item) || cJSON_IsNumber(vol_item))) {
                                std::string name = name_item->valuestring;
                                double volume = 0.0;
                                if (cJSON_IsString(vol_item)) {
                                    volume = std::stod(vol_item->valuestring);
                                } else if (cJSON_IsNumber(vol_item)) {
                                    volume = vol_item->valuedouble;
                                }
                                assets.push_back({name, volume});
                            }
                        }

                        std::sort(assets.begin(), assets.end(), [](const AssetInfo& a, const AssetInfo& b) {
                            return a.volume > b.volume;
                        });

                        for (int i = 0; i < std::min((int)assets.size(), top_n); ++i) {
                            top_symbols.push_back({assets[i].name + "_USDT"});
                        }
                    }
                }
                cJSON_Delete(root);
            } else {
                std::cerr << "Error parsing Hyperliquid API JSON.\n";
            }
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if (top_symbols.empty()) {
        top_symbols = {
{"BTC_USDT"},{"ETH_USDT"},{"SOL_USDT"},{"BNB_USDT"},{"XRP_USDT"},
{"DOGE_USDT"},{"AVAX_USDT"},{"SUI_USDT"},{"NEAR_USDT"},{"ADA_USDT"},
{"TRX_USDT"},{"LINK_USDT"},{"PEPE_USDT"},{"SHIB_USDT"},{"WLD_USDT"},
{"OP_USDT"},{"ARB_USDT"},{"FET_USDT"},{"HYPE_USDT"},{"ORDI_USDT"}
};
    }
    return top_symbols;
}

double calculate_atr(const std::vector<Candle>& candles, size_t index, size_t period = 14) {
    if (index < period) return 0.0;
    double sum = 0.0;
    for (size_t i = index - period + 1; i <= index; ++i) {
        sum += std::max({candles[i].high - candles[i].low, std::abs(candles[i].high - candles[i-1].close), std::abs(candles[i].low - candles[i-1].close)});
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
    bool can_s = z > threshold && !trend.is_bullish && trend.is_trending && trend.bullish_votes <= 1;
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

            if (is_l) {
                if (candles[i].low <= fixed_sl) { exit_price = fixed_sl * (1.0 - SLIPPAGE_PCT); exit = true; }
                else if (candles[i].high >= dynamic_tp) { exit_price = dynamic_tp * (1.0 - SLIPPAGE_PCT); exit = true; } // TP fills slip too, not just SL
            } else {
                if (candles[i].high >= fixed_sl) { exit_price = fixed_sl * (1.0 + SLIPPAGE_PCT); exit = true; }
                else if (candles[i].low <= dynamic_tp) { exit_price = dynamic_tp * (1.0 + SLIPPAGE_PCT); exit = true; } // TP fills slip too, not just SL
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

struct WalkForwardResult { 
    double avg_oos_return = -9999.0; 
    int valid_folds = 0; 
    double avg_oos_dd = 0.0; 
    double avg_wins_per_day = 0.0;
    double avg_test_days = 0.0;   // average duration of test folds
    std::vector<double> pooled_trade_pcts; // trades pooled across all valid folds, for MC on TRAIN data only
    std::vector<std::string> pooled_trade_dirs;
};

// data_end restricts folds to candles[0, data_end) so this function can be run purely
// against the TRAIN slice, keeping the held-out TEST slice completely unseen during
// both parameter selection and the outer SL/TP scan.
WalkForwardResult evaluate_walk_forward(const std::vector<Candle>& candles, const StrategyParams& params,
                                         const Indicators& ind, const std::vector<double>& z_scores,
                                         size_t n_folds = 4, int min_trades_per_fold = 5,
                                         size_t data_end = 0) {
    WalkForwardResult wf;
    size_t limit = (data_end == 0 || data_end > candles.size()) ? candles.size() : data_end;
    if (limit < n_folds * 150) return wf;
    size_t fold_size = limit / n_folds;
    double total_ret = 0.0, total_dd = 0.0, total_wpd = 0.0, total_days = 0.0;
    for (size_t f = 0; f + 2 <= n_folds; ++f) {
        size_t test_start = fold_size * (f + 1);
        size_t test_end = std::min(limit, fold_size * (f + 2));
        if (test_start >= test_end) continue;
        BacktestResult r = run_backtest(candles, params, ind, z_scores, test_start, test_end);
        if (r.total_trades >= min_trades_per_fold) {
            total_ret += r.total_return;
            total_dd += r.max_drawdown;
            long long fold_duration_ms = candles[test_end-1].timestamp - candles[test_start].timestamp;
            double fold_days = std::max(1.0, fold_duration_ms / (1000.0 * 60.0 * 60.0 * 24.0));
            total_days += fold_days;
            double wins = r.winning_longs + r.winning_shorts;
            total_wpd += wins / fold_days;
            wf.valid_folds++;
            wf.pooled_trade_pcts.insert(wf.pooled_trade_pcts.end(), r.trade_pcts.begin(), r.trade_pcts.end());
            wf.pooled_trade_dirs.insert(wf.pooled_trade_dirs.end(), r.trade_dirs.begin(), r.trade_dirs.end());
        }
    }
    if (wf.valid_folds >= 2) {
        wf.avg_oos_return = total_ret / wf.valid_folds;
        wf.avg_oos_dd = total_dd / wf.valid_folds;
        wf.avg_wins_per_day = total_wpd / wf.valid_folds;
        wf.avg_test_days = total_days / wf.valid_folds;
    }
    return wf;
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

std::vector<double> compute_returns(const std::vector<Candle>& candles, size_t lookback = 200) {
    std::vector<double> rets;
    if (candles.size() < 2) return rets;
    size_t start = candles.size() > lookback ? candles.size() - lookback : 1;
    for (size_t i = start; i < candles.size(); ++i) {
        double prev = candles[i - 1].close;
        if (prev != 0.0) rets.push_back((candles[i].close - prev) / prev);
    }
    return rets;
}

double pearson_correlation(const std::vector<double>& a, const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    if (n < 20) return 0.0;
    double mean_a = 0.0, mean_b = 0.0;
    for (size_t i = 0; i < n; ++i) { mean_a += a[a.size() - n + i]; mean_b += b[b.size() - n + i]; }
    mean_a /= n; mean_b /= n;
    double cov = 0.0, var_a = 0.0, var_b = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double da = a[a.size() - n + i] - mean_a, db = b[b.size() - n + i] - mean_b;
        cov += da * db; var_a += da * da; var_b += db * db;
    }
    if (var_a < 1e-12 || var_b < 1e-12) return 0.0;
    return cov / std::sqrt(var_a * var_b);
}

 
std::vector<OpenPosition> parse_open_positions(const std::string& json_str) {
    std::vector<OpenPosition> out;

    cJSON* json = cJSON_Parse(json_str.c_str());
    if (!json) {
        std::cerr << "Error: Failed to parse positions JSON payload." << std::endl;
        return out;
    }

    cJSON* data = cJSON_GetObjectItem(json, "data");
    if (!data || !cJSON_IsArray(data)) {
        cJSON_Delete(json);
        return out;
    }

    int arraySize = cJSON_GetArraySize(data);
    for (int i = 0; i < arraySize; ++i) {
        cJSON* item = cJSON_GetArrayItem(data, i);
        if (!item) continue;

        // 1. Parse Volume (Check "holdVol", fallback to "vol")
        double vol = 0.0;
        cJSON* holdVolObj = cJSON_GetObjectItem(item, "holdVol");
        if (holdVolObj) {
            vol = holdVolObj->valuedouble;
        } else {
            cJSON* volObj = cJSON_GetObjectItem(item, "vol");
            if (volObj) vol = volObj->valuedouble;
        }

        // Only bother evaluating further if there is an active volume balance
        if (vol > 0.0) {
            // 2. Parse Symbol
            cJSON* symbolObj = cJSON_GetObjectItem(item, "symbol");
            std::string symbol = symbolObj && symbolObj->valuestring ? symbolObj->valuestring : "";

            if (symbol.empty()) continue;

            // 3. Parse Direction (positionType: 1 = Long, 2 = Short)
            std::string direction = "LONG";
            cJSON* posTypeObj = cJSON_GetObjectItem(item, "positionType");
            if (posTypeObj && posTypeObj->valueint == 2) {
                direction = "SHORT";
            }

            // 4. Parse Average Entry Price (Check "openAvgPrice", fallback to "holdAvgPrice")
            double avg_price = 0.0;
            cJSON* openAvgPriceObj = cJSON_GetObjectItem(item, "openAvgPrice");
            if (openAvgPriceObj) {
                avg_price = openAvgPriceObj->valuedouble;
            } else {
                cJSON* holdAvgPriceObj = cJSON_GetObjectItem(item, "holdAvgPrice");
                if (holdAvgPriceObj) avg_price = holdAvgPriceObj->valuedouble;
            }

            // 5. Package the data identically to your old parser logic, but null-safe:
            // an unexpected/missing field from the exchange should not crash a live trading process.
            double positionValue = std::abs(vol * avg_price);
			cJSON* createTimeObj = cJSON_GetObjectItem(item, "createTime");
			std::time_t rawTime = createTimeObj ? (std::time_t)(createTimeObj->valuedouble / 1000) : std::time(nullptr);
			std::tm timeInfo{};
			localtime_r(&rawTime, &timeInfo);
			char buf[16];
			std::strftime(buf, sizeof(buf), "%Y-%m-%d %H", &timeInfo);
			std::string createTime(buf);

			cJSON* pnlObj = cJSON_GetObjectItem(item, "unRealizedPnl");
			float unrealized = pnlObj ? (float)pnlObj->valuedouble : 0.0f;
			float entryPrice = (float)avg_price;

            out.push_back({symbol, direction, positionValue, createTime, unrealized, entryPrice, 0, 0});
        }
    }

    // Always free memory allocated by cJSON before exiting
    cJSON_Delete(json);
    return out;
}

double get_correlated_risk_committed(const std::string& candidate_symbol,
                                      const std::vector<double>& candidate_returns,
                                      const PortfolioState& portfolio,
                                      const std::map<std::string, std::vector<double>>& returns_cache,
                                      double equity) {
    double committed = 0.0;
    for (const auto& pos : portfolio.positions) {
        if (pos.symbol == candidate_symbol) continue;
        auto it = returns_cache.find(pos.symbol);
        if (it == returns_cache.end()) continue;
        double corr = pearson_correlation(candidate_returns, it->second);
        if (std::abs(corr) >= CORRELATION_THRESHOLD) committed += RISK_PER_TRADE;
    }
    return committed;
}

std::vector<Candle> fetch_candles(const std::string& symbol) {
    std::vector<Candle> candles;
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        std::string url = "https://contract.mexc.com/api/v1/contract/kline/" + symbol + "?interval=Min60&limit=2000";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        if (curl_easy_perform(curl) == CURLE_OK) {
            std::vector<double> t = extractArray(readBuffer, "time"); std::vector<double> h = extractArray(readBuffer, "high");
            std::vector<double> l = extractArray(readBuffer, "low"); std::vector<double> c = extractArray(readBuffer, "close");
            size_t n = t.size(); candles.reserve(n);
            for (size_t i = 0; i < n; ++i) candles.push_back({(long long)t[i] * 1000LL, h[i], l[i], c[i]});
        }
        curl_easy_cleanup(curl);
    }
    return candles;
}

AnalysisResult analyze_symbol(int idsmb, double equity, const std::vector<Candle>& candles, bool verbose = true) {
    AnalysisResult res;
    string symbol = symbols[idsmb].mexc;
    if (candles.size() < 150) return res;

    // TRAIN / HOLDOUT split. Every fold, every grid-search candidate, and the outer
    // SL/TP scan may only look at candles[0, train_end). candles[train_end, end) is
    // touched exactly once below, after params are already locked in.
    size_t train_end = static_cast<size_t>(candles.size() * (1.0 - TEST_FRACTION));
    if (train_end < 150 || candles.size() - train_end < 40) {
        if (verbose) std::cout << "  " << symbol << ": not enough history for a clean train/holdout split, skipping.\n";
        return res;
    }

    size_t last_closed_idx = candles.size() - 2;
    Indicators ind = precompute_indicators(candles); // indicator math has no fitted params, safe to run on full series
    std::map<size_t, std::vector<double>> z_cache;

    FeeConfig fees;
    fees.borrow_daily = fetch_funding_rate(symbol);

    StrategyParams best_params = {10, 1.0, 1.5};
    double best_wf_score = -9999.0;
    WalkForwardResult best_wf;
    // Trimmed from the original 5x4x4=80 combos to 4x3x3=36 — still meaningfully fewer
    // degrees of freedom than the original (less room to fit train-region noise), but not
    // so few that a real edge in the data has nowhere to be found.
    for (size_t p : {8, 10, 13, 20}) {
        auto& z_scores = z_cache.try_emplace(p, precompute_zscore(candles, p)).first->second;
        for (auto t : {0.6, 0.8, 1.0}) {
            for (auto a : {1.4, 1.7, 2.0}) {
                StrategyParams cand = {p, t, a};
                // n_folds=4, min_trades_per_fold=5: same cadence as the original design
                // (3 candidate folds, 2 must clear the bar), just now confined to the
                // train region via data_end instead of spanning the whole series.
                WalkForwardResult wf = evaluate_walk_forward(candles, cand, ind, z_scores, 4, 5, train_end);
                if (wf.valid_folds < 2) continue;
                double score = wf.avg_oos_return * (1.0 - wf.avg_oos_dd / 100.0) + 0.5 * wf.avg_wins_per_day;
                if (score > best_wf_score) { best_wf_score = score; best_params = cand; best_wf = wf; }
            }
        }
    }
    if (best_wf.valid_folds < 2) {
        if (verbose) std::cout << "  " << symbol << ": insufficient train-region walk-forward data, skipping.\n";
        return res;
    }

    auto& best_z_scores = z_cache[best_params.period];

    // MC and wins/day used for SELECTION/GATING must stay TRAIN-only, or the outer
    // SL/TP scan would implicitly be fit against the same holdout it's meant to validate.
    MonteCarloResult mc_res = run_monte_carlo(best_wf.pooled_trade_pcts, 1000.0, 2000);
    double wf_daily_return = (best_wf.avg_test_days > 0) ? best_wf.avg_oos_return / best_wf.avg_test_days : 0.0;

    // --- TRUE holdout: candles[train_end, end) was never seen above. One shot, no cherry-picking. ---
    BacktestResult holdout = run_backtest(candles, best_params, ind, best_z_scores, train_end, candles.size() - 1, fees);
    long long span_ms = candles.back().timestamp - candles[train_end].timestamp;
    double holdout_days = std::max(1.0, span_ms / (1000.0 * 60.0 * 60.0 * 24.0));
    double holdout_daily_return = holdout.total_return / holdout_days;

    if (verbose) {
        std::cout << "Best Params " << symbol << ": Period=" << best_params.period << " Th=" << best_params.threshold << " ATR=" << best_params.atr_mult
                  << " | funding(daily)=" << std::setprecision(4) << fees.borrow_daily * 100.0 << "%\n"
                  << "TRAIN Walk-Forward Avg OOS: " << std::fixed << std::setprecision(2) << best_wf.avg_oos_return
                  << "% over " << best_wf.valid_folds << " folds (wins/day: " << best_wf.avg_wins_per_day
                  << ", daily return: " << wf_daily_return << "%) [in-sample — used for selection]\n"
                  << "TRUE HOLDOUT (" << holdout_days << "d, never touched during fitting): "
                  << holdout.total_return << "% | Drawdown: " << holdout.max_drawdown
                  << "% | daily: " << holdout_daily_return << "% | trades: " << holdout.total_trades << "\n"
                  << "Monte Carlo (train) -> Prob of Ruin: " << mc_res.prob_of_ruin << "% | Worst-Case DD: " << mc_res.worst_case_drawdown << "%\n\n";
    }

    res.oos_return = holdout.total_return;
    res.oos_drawdown = holdout.max_drawdown;
    res.oos_trades = holdout.total_trades;
    res.oos_wins = holdout.winning_longs + holdout.winning_shorts;
    res.oos_days = holdout_days;
    res.holdout_daily_return = holdout_daily_return;
    res.walk_forward_daily_return = wf_daily_return;
    res.walk_forward_score = best_wf.avg_oos_return;
    res.train_wins_per_day = best_wf.avg_wins_per_day;
    res.oos_trade_pcts = holdout.trade_pcts;
    res.oos_trade_dirs = holdout.trade_dirs;
    res.signal = generate_signal(candles, last_closed_idx, best_params, equity, ind);
    res.mc = mc_res;
    res.recent_returns = compute_returns(candles, 200);
    res.success = true;
    return res;
}

struct RankedSymbol { size_t index; AnalysisResult analysis; };

struct DeploymentResult {
    double cumulative_oos = 0.0;           // sum of TRAIN walk‑forward avg OOS returns (in-sample, for combo selection only)
    size_t slots_filled = 0;
    double avg_wins_per_day = 0.0;
    double projected_daily_return_wf = 0.0;      // sum of TRAIN walk-forward daily returns of deployed slots (in-sample)
    double projected_daily_return_holdout = 0.0; // sum of TRUE HOLDOUT daily returns of deployed slots (out-of-sample; only meaningful when enforce_holdout_gate=true)
};

// enforce_holdout_gate should be false during the SL/TP scanning phase (so the scan never
// touches holdout data) and true only for the final live pass, where a symbol whose true
// holdout performance collapsed gets vetoed even though it looked fine on train data.
DeploymentResult simulate_deployment(const std::vector<RankedSymbol>& pipeline,
                                      const std::map<std::string, std::vector<double>>& returns_cache,
                                      double equity,
                                      PortfolioState portfolio,
                                      bool execute_orders,
                                      bool verbose,
                                      const std::vector<symbolstruct>& syms,
                                      bool enforce_holdout_gate = false) {
    DeploymentResult dr;
    const size_t MAX_SIMULTANEOUS_ASSETS = 4;
    PortfolioState working_portfolio = portfolio;

    if (verbose) std::cout << "\n------------------ DEPLOYMENT MATRIX ------------------\n";
    double total_wpd = 0.0;
    double total_daily_return_wf = 0.0;
    double total_daily_return_holdout = 0.0;
    int accepted_count = 0;

    for (const auto& item : pipeline) {
        size_t idx = item.index;
        const auto& report = item.analysis;
        const std::string& sym = syms[idx].mexc;

        if (dr.slots_filled >= MAX_SIMULTANEOUS_ASSETS) {
            if (verbose) std::cout << "[SKIPPED] " << sym << " (WF: " << report.walk_forward_score << "%) - Exceeds portfolio limit slots.\n";
            continue;
        }
        if (report.walk_forward_score <= 0.0) {
            if (verbose) std::cout << "[FILTERED] " << sym << " (WF: " << report.walk_forward_score << "%) - No validated edge across folds.\n";
            continue;
        }
        if (report.mc.worst_case_drawdown > 12) {
            if (verbose) std::cout << "[RISK-REJECTED] " << sym << " | WF: " << report.walk_forward_score
                      << "% but Monte Carlo Worst-Case DD: " << report.mc.worst_case_drawdown << "%\n";
            continue;
        }

        // Wins/day gate uses TRAIN-derived frequency consistently, whether this is a scan
        // pass or the final pass — using holdout wins/day here would let the SL/TP scan
        // implicitly optimize against the holdout it's supposed to be blind to.
        if (report.train_wins_per_day < MIN_WINS_PER_DAY) {
            if (verbose) std::cout << "[LOW FREQ] " << sym << " | train wins/day=" << std::fixed << std::setprecision(3) << report.train_wins_per_day
                      << " below threshold " << MIN_WINS_PER_DAY << ", skipping.\n";
            continue;
        }

        // Holdout veto: only applied on the final live pass. A strategy that looked
        // good on train but lost money on data it never saw gets pulled here, without
        // that information ever having influenced parameter selection.
        if (enforce_holdout_gate && report.oos_return <= 0.0) {
            if (verbose) std::cout << "[HOLDOUT-REJECTED] " << sym << " | TRAIN WF looked fine (" << report.walk_forward_score
                      << "%) but true holdout return was " << report.oos_return << "% over " << report.oos_days << "d — not deploying.\n";
            continue;
        }

        double corr_risk = get_correlated_risk_committed(sym, report.recent_returns, working_portfolio, returns_cache, equity);
        if (corr_risk + RISK_PER_TRADE > MAX_CORRELATED_RISK) {
            if (verbose) std::cout << "[CORRELATION-REJECTED] " << sym << " | Correlated risk already committed: "
                      << (corr_risk * 100.0) << "% of equity, adding this would exceed the "
                      << (MAX_CORRELATED_RISK * 100.0) << "% correlated-bucket cap.\n";
            continue;
        }

        dr.cumulative_oos += report.walk_forward_score;
        total_wpd += report.train_wins_per_day;
        total_daily_return_wf += report.walk_forward_daily_return;
        total_daily_return_holdout += report.holdout_daily_return;
        accepted_count++;

        if (verbose) {
            std::cout << ">>> [VALIDATED] Slot #" << (dr.slots_filled + 1) << ": Deploying to " << sym
                      << " | TRAIN WF OOS: " << report.walk_forward_score << "% | train wins/day: " << std::fixed << std::setprecision(3) << report.train_wins_per_day
                      << " | daily return (train WF): " << std::setprecision(3) << report.walk_forward_daily_return << "%"
                      << " | daily return (TRUE holdout): " << report.holdout_daily_return << "%"
                      << " | Signal: " << report.signal.direction << "\n";
            std::cout << " -> Last Tested Trades (Holdout OOS): ";
            if (report.oos_trade_pcts.empty()) {
                std::cout << "None\n";
            } else {
                int start = std::max(0, (int)report.oos_trade_pcts.size() - 10);
                for (size_t t = start; t < report.oos_trade_pcts.size(); ++t) {
                    std::string dir = report.oos_trade_dirs[t];
                    double pct = report.oos_trade_pcts[t] * 100.0;
                    std::cout << dir << "(";
                    if (pct >= 0) std::cout << "+";
                    std::cout << std::fixed << std::setprecision(2) << pct << "%) ";
                }
                std::cout << "\n";
            }
        }

        if (report.signal.direction != "WAIT") {
            double qty = report.signal.exact_position_btc;
            double notional = qty * report.signal.entry;

            if (qty > 0) {
                if (execute_orders) {
                    printf(" -> EXECUTE ORDER: %s %s | Qty: %f | Entry: %f | SL: %f | TP: %f\n",
                           sym.c_str(), (report.signal.direction == "LONG" ? "BUY" : "SELL"),
                           qty, report.signal.entry, report.signal.stop_loss, report.signal.take_profit);
                    std::cout << " -> Required Leverage: " << (notional / equity) << "x\n";

                    openAtomicBracketFuturesPosition(
                        sym,
                        report.signal.direction == "LONG" ? "BUY" : "SELL",
                        qty,
                        report.signal.entry,
                        report.signal.stop_loss,
                        report.signal.take_profit
                    );
                } else if (verbose) {
                    std::cout << " -> [DRY RUN] Would execute " << report.signal.direction << " on " << sym << "\n";
                }
                working_portfolio.positions.push_back({sym, report.signal.direction, notional});
            }
        } else if (verbose) {
            std::cout << " -> Market pattern neutral. No live trade trigger generated.\n";
        }
        dr.slots_filled++;
    }
    if (accepted_count > 0) {
        dr.avg_wins_per_day = total_wpd / accepted_count;
        dr.projected_daily_return_wf = total_daily_return_wf;
        dr.projected_daily_return_holdout = total_daily_return_holdout;
    }
    if (verbose) std::cout << "--------------------------------------------------------\n";
    return dr;
}

int main() {
    symbols = get_hyperliquid_top_symbols(20);
    std::cout << "Successfully mapped " << symbols.size() << " top symbols by volume from Hyperliquid.\n\n";
    money moneyei = getUsdtFuturesBalance();
	getOpenPositionSymbols();
    std::string openPositions = opened_positions;
    std::cout << "Current Account Equity: " << moneyei.equity << " USDT\n";

    PortfolioState base_portfolio;
    base_portfolio.positions = parse_open_positions(openPositions);
    std::cout << "Reconstructed " << base_portfolio.positions.size() << " open position(s) from exchange:\n";
    for (const auto& p : base_portfolio.positions)
        std::cout << "  " << p.symbol << " " << p.direction << " notional~" << p.notional_usd << " USDT "
	<<" createTime:"<<p.createTime
	<<" entry:"<<p.entry
	<<" gain:"<<p.unRealizedPnl
	<<"\n";

    vector<std::string> gops = getOpenPositionSymbols();
    cout << "Active Structural Positions: " << gops.size() << "\n\n";
    for (size_t i = 0; i < gops.size(); i++) cout << "gops: " << gops[i] << "\n";
	

    std::cout << "--- Fetching candle history for " << symbols.size() << " symbols ---\n";
    std::vector<std::vector<Candle>> candle_cache(symbols.size());
    for (size_t i = 0; i < symbols.size(); i++) {
        candle_cache[i] = fetch_candles(symbols[i].mexc);
        std::cout << "  " << symbols[i].mexc << ": " << candle_cache[i].size() << " candles\n";
    }

	
	cancelAllPendingOrders();

    std::cout << "\n--- Scanning MIN_SL_PCT x tp_factor combinations ---\n";
    double best_combo_sl = SL_CANDIDATES[0], best_combo_tp = TP_CANDIDATES[0];
    double best_combo_score = -1e18;
    for (double sl : SL_CANDIDATES) {
        for (double tp : TP_CANDIDATES) {
            MIN_SL_PCT = sl;
            tp_factor = tp;

            std::vector<RankedSymbol> pipeline;
            std::map<std::string, std::vector<double>> returns_cache;
            for (size_t i = 0; i < symbols.size(); i++) {
                if (candle_cache[i].empty()) continue;
                AnalysisResult report = analyze_symbol(i, moneyei.equity, candle_cache[i], /*verbose=*/false);
                if (report.success) {
                    returns_cache[symbols[i].mexc] = report.recent_returns;
                    pipeline.push_back({i, report});
                }
            }
            std::sort(pipeline.begin(), pipeline.end(), [](const RankedSymbol& a, const RankedSymbol& b) {
                return a.analysis.walk_forward_score > b.analysis.walk_forward_score;
            });

            DeploymentResult dry = simulate_deployment(pipeline, returns_cache, moneyei.equity,
                                                        base_portfolio, /*execute_orders=*/false,
                                                        /*verbose=*/false, symbols);

            double combo_score = dry.cumulative_oos;   // only walk‑forward OOS; no extra win‑rate term
            std::cout << "  [SCAN] MIN_SL_PCT=" << (sl * 100.0) << "% tp_factor=" << tp
                      << "x -> Cumulative WF OOS: " << std::fixed << std::setprecision(2)
                      << dry.cumulative_oos << "% (slots=" << dry.slots_filled
                      << ", avg wins/day=" << std::setprecision(3) << dry.avg_wins_per_day
                      << ", combo_score=" << std::setprecision(2) << combo_score << ")\n";

            if (dry.slots_filled > 0 && combo_score > best_combo_score) {
                best_combo_score = combo_score;
                best_combo_sl = sl;
                best_combo_tp = tp;
            }
        }
    }

    if (best_combo_score <= -1e17) {
        std::cout << "\nNo (MIN_SL_PCT, tp_factor) combination produced any validated slots. Defensive phase, no trades.\n";
        return 0;
    }

    std::cout << "\n>>> Selected MIN_SL_PCT=" << (best_combo_sl * 100.0) << "% tp_factor=" << best_combo_tp
              << "x with combo_score " << best_combo_score << "\n";
    MIN_SL_PCT = best_combo_sl;
    tp_factor = best_combo_tp;

    std::vector<RankedSymbol> pipeline;
    std::map<std::string, std::vector<double>> returns_cache;
    std::cout << "\n--- Final Screening Pass (Selected Parameters) ---\n";
    for (size_t i = 0; i < symbols.size(); i++) {
        if (candle_cache[i].empty()) continue;
        AnalysisResult report = analyze_symbol(i, moneyei.equity, candle_cache[i], /*verbose=*/true);
        if (report.success) {
            returns_cache[symbols[i].mexc] = report.recent_returns;
            pipeline.push_back({i, report});
        }
    }
    std::sort(pipeline.begin(), pipeline.end(), [](const RankedSymbol& a, const RankedSymbol& b) {
        return a.analysis.walk_forward_score > b.analysis.walk_forward_score;
    });

    DeploymentResult live = simulate_deployment(pipeline, returns_cache, moneyei.equity,
                                                 base_portfolio, /*execute_orders=*/true,
                                                 /*verbose=*/true, symbols, /*enforce_holdout_gate=*/true);

    if (live.slots_filled > 0) {
        cout << "\nPortfolio Deployed Slots: " << live.slots_filled << " / 4\n";
        cout << "Cumulative TRAIN Walk-Forward OOS Return: " << live.cumulative_oos << "% [in-sample, used only for parameter selection]\n";
        cout << "Average wins/day (train): " << live.avg_wins_per_day << "\n";
        cout << "Projected daily return (TRAIN walk-forward, in-sample): " << live.projected_daily_return_wf << "% per day\n";
        cout << "Projected daily return (TRUE HOLDOUT, out-of-sample, never used for fitting): "
             << live.projected_daily_return_holdout << "% per day\n";
        cout << "NOTE: the holdout figure above is a single ~" << (TEST_FRACTION * 100.0)
             << "% window per symbol, not a statistically robust estimate. Apply an additional "
             << "haircut for execution slippage, latency, and regime change not captured in any backtest — "
             << "treat both numbers as upper bounds, not guarantees, and confirm with live paper-trading before sizing real capital.\n";
    } else {
        cout << "Defensive Phase: 0 Slots Allocated. All assets filtered out.\n";
    }
    return 0;
}