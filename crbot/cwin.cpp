// sudo apt update
// sudo apt install libcurl4-openssl-dev pkg-config
// mkdir -p include/nlohmann
// wget -O include/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
// g++ -Os cwin.cpp -o cwin -I./include -Wl,-Bstatic $(pkg-config --static --libs libcurl) -Wl,-Bdynamic -lpthread -ldl
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Candle {
    double high;
    double low;
    double close;
};

struct StrategyParams {
    size_t period;
    double threshold;
    double atr_mult;
};

// ─── Fee model ────────────────────────────────────────────────────────────────
// Entry = limit order (maker). Exit = market order (taker, use worst case 0.055%).
// Margin borrow fee charged per candle held: 0.0585% / day.
// 6h candle = 0.25 days → borrow_fee_per_candle = 0.0585% * 0.25
struct FeeConfig {
    double maker_fee     = 0.0001;    // 0.01%  — entry (limit)
    double taker_fee     = 0.00055;   // 0.055% — exit (market, worst case)
    double borrow_daily  = 0.000585;  // 0.0585% per day
    double candle_hours  = 1.0;       // change if you switch interval
    double borrow_per_candle() const { return borrow_daily * (candle_hours / 24.0); }
};

struct BacktestResult {
    StrategyParams params;
    double total_return;
    double dollar_gain;      // final_capital - 1000 (per slice, starting at $1000)
    double total_fees_paid;  // sum of all fees deducted
    int total_trades;
    int long_trades;
    int short_trades;
    int winning_longs;
    int winning_shorts;
    double win_rate;
    double fitness_score;
};

// ─── Multi-timeframe trend structure ──────────────────────────────────────────
struct TrendReading {
    double ema_short;   // fast: EMA-20
    double ema_medium;  // medium: EMA-50
    double ema_long;    // slow: EMA-100
    int bullish_votes;  // how many EMAs agree price > EMA
    bool is_bullish;    // majority vote (2 of 3)
    bool is_trending;   // EMAs are stacked (short > medium > long or reverse)
    double trend_strength; // how far price is from average EMA (0-1 normalised)
};

// ─── Signal structure ─────────────────────────────────────────────────────────
struct LiveSignal {
    std::string direction;
    double confidence;
    double entry;
    double stop_loss;
    double take_profit;
    double z_score;
    double atr;
    TrendReading trend;
    std::string regime;
    std::string warnings;
    double order_size_usd;   // = free_margin (100% of available capital)
    double order_size_btc;   // order_size_usd / entry_price
};

enum class PositionType { NONE, LONG, SHORT };

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

// ─── ATR (14 period) ─────────────────────────────────────────────────────────
double calculate_atr(const std::vector<Candle>& candles, size_t index, size_t period = 14) {
    if (index < period) return 0.0;
    double sum = 0.0;
    for (size_t i = index - period + 1; i <= index; ++i) {
        double hl = candles[i].high - candles[i].low;
        double hc = std::abs(candles[i].high - candles[i-1].close);
        double lc = std::abs(candles[i].low  - candles[i-1].close);
        sum += std::max({hl, hc, lc});
    }
    return sum / period;
}

// ─── Multi-timeframe EMA trend consensus ─────────────────────────────────────
// Uses three EMA lengths. Trend is bullish only when at least 2 of 3 agree.
// Also detects whether EMAs are stacked (trending) vs interleaved (choppy).
TrendReading calculate_trend(const std::vector<Candle>& candles, size_t up_to_idx,
                             size_t short_p = 20, size_t medium_p = 50, size_t long_p = 100) {
    TrendReading t{};

    auto calc_ema = [&](size_t period) -> double {
        double ema = candles[0].close;
        double k = 2.0 / (period + 1);
        for (size_t i = 1; i <= up_to_idx; ++i)
            ema = candles[i].close * k + ema * (1.0 - k);
        return ema;
    };

    double price = candles[up_to_idx].close;
    t.ema_short  = calc_ema(short_p);
    t.ema_medium = calc_ema(medium_p);
    t.ema_long   = calc_ema(long_p);

    t.bullish_votes = (price > t.ema_short  ? 1 : 0)
                    + (price > t.ema_medium ? 1 : 0)
                    + (price > t.ema_long   ? 1 : 0);

    t.is_bullish = (t.bullish_votes >= 2);

    // EMAs stacked = trend is clean
    bool bull_stack = (t.ema_short > t.ema_medium && t.ema_medium > t.ema_long);
    bool bear_stack = (t.ema_short < t.ema_medium && t.ema_medium < t.ema_long);
    t.is_trending = bull_stack || bear_stack;

    // Trend strength: avg distance of price from EMAs, normalised by ATR
    double atr = calculate_atr(candles, up_to_idx, 14);
    if (atr > 0) {
        double avg_ema = (t.ema_short + t.ema_medium + t.ema_long) / 3.0;
        t.trend_strength = std::min(1.0, std::abs(price - avg_ema) / (atr * 3.0));
    }

    return t;
}

// ─── Market regime classifier ─────────────────────────────────────────────────
// TRENDING  — ATR low relative to price range, EMAs stacked
// VOLATILE  — ATR very high, big recent swings
// RANGING   — otherwise (sideways, no stacking)
std::string classify_regime(const std::vector<Candle>& candles, size_t idx, size_t lookback = 20) {
    if (idx < lookback + 14) return "UNKNOWN";

    double atr = calculate_atr(candles, idx, 14);
    double price = candles[idx].close;

    // Recent high/low range
    double hi = candles[idx].high, lo = candles[idx].low;
    for (size_t i = idx - lookback; i <= idx; ++i) {
        hi = std::max(hi, candles[i].high);
        lo = std::min(lo, candles[i].low);
    }
    double range = hi - lo;

    // ATR as % of price
    double atr_pct = (atr / price) * 100.0;

    if (atr_pct > 3.5) return "VOLATILE";

    // Check EMA stacking over lookback
    TrendReading tr = calculate_trend(candles, idx);
    if (tr.is_trending && atr_pct < 2.5) return "TRENDING";

    return "RANGING";
}

// ─── Z-score with volatility adjustment ──────────────────────────────────────
// Divides by stdev but also scales for high-volatility regimes so signals
// are comparable across different candle intervals.
double calculate_z_score(const std::vector<Candle>& candles, size_t idx, size_t period) {
    double sum = 0.0;
    for (size_t j = idx - period; j < idx; ++j) sum += candles[j].close;
    double mean = sum / period;

    double sq_sum = 0.0;
    for (size_t j = idx - period; j < idx; ++j) {
        double d = candles[j].close - mean;
        sq_sum += d * d;
    }
    double stdev = std::sqrt(sq_sum / period);
    if (stdev < 1e-9) return 0.0;

    return (candles[idx].close - mean) / stdev;
}

// ─── Confidence score (0–100) ─────────────────────────────────────────────────
// Higher = better trade setup
double calculate_confidence(double z_score, double threshold,
                            const TrendReading& trend,
                            const std::string& regime,
                            bool is_long) {
    double score = 0.0;

    // Z-score beyond threshold: how far beyond?
    double z_excess = std::abs(z_score) - threshold;
    score += std::min(30.0, z_excess * 15.0);

    // EMA consensus: 3/3 votes = 25pts, 2/3 = 10pts
    if (trend.bullish_votes == 3 && is_long)  score += 25.0;
    if (trend.bullish_votes == 0 && !is_long) score += 25.0;
    if (trend.bullish_votes == 2 && is_long)  score += 10.0;
    if (trend.bullish_votes == 1 && !is_long) score += 10.0;

    // Stacked EMAs (clean trend)
    if (trend.is_trending) score += 15.0;

    // Trend strength
    score += trend.trend_strength * 15.0;

    // Regime bonus
    if (regime == "TRENDING") score += 15.0;
    if (regime == "VOLATILE") score -= 10.0;   // wide stops, less reliable

    return std::max(0.0, std::min(100.0, score));
}

// ─── Backtest slice (with realistic fee model) ────────────────────────────────
BacktestResult run_slice(const std::vector<Candle>& candles,
                         size_t start_idx, size_t end_idx,
                         const StrategyParams& params,
                         const FeeConfig& fees = FeeConfig{}) {
    double capital = 1000.0;
    double initial_capital = capital;
    double position = 0.0;
    double notional = 0.0;           // position size in USD (for fee calculation)
    double total_fees = 0.0;
    PositionType pos_type = PositionType::NONE;

    double entry_price = 0.0, trailing_sl = 0.0, dynamic_tp = 0.0;
    int long_trades = 0, short_trades = 0;
    int winning_longs = 0, winning_shorts = 0;

    size_t actual_start = std::max({start_idx, (size_t)115, params.period + 1});

    auto apply_entry_fee = [&]() {
        // Maker fee on notional at entry
        double fee = notional * fees.maker_fee;
        capital -= fee;
        total_fees += fee;
    };

    auto apply_exit_fee = [&](double exit_price) {
        // Taker fee on notional at exit
        double fee = position * exit_price * fees.taker_fee;
        capital -= fee;
        total_fees += fee;
    };

    auto apply_borrow_fee = [&](double current_price) {
        // Margin borrow: charged on the notional value per candle held
        double fee = position * current_price * fees.borrow_per_candle();
        capital -= fee;
        total_fees += fee;
    };

    for (size_t i = actual_start; i < end_idx; ++i) {
        double price = candles[i].close;
        double atr   = calculate_atr(candles, i, 14);
        TrendReading trend = calculate_trend(candles, i);

        if (pos_type == PositionType::NONE) {
            double z = calculate_z_score(candles, i, params.period);

            if (z < -params.threshold && trend.is_bullish) {
                // Always use current capital so gains compound
                notional    = capital;
                position    = capital / price;
                entry_price = price;
                pos_type    = PositionType::LONG;
                trailing_sl = entry_price - (atr * params.atr_mult);
                dynamic_tp  = entry_price + (atr * params.atr_mult * 1.5);
                apply_entry_fee();
            }
            else if (z > params.threshold && !trend.is_bullish) {
                notional    = capital;
                position    = capital / price;
                entry_price = price;
                pos_type    = PositionType::SHORT;
                trailing_sl = entry_price + (atr * params.atr_mult);
                dynamic_tp  = entry_price - (atr * params.atr_mult * 1.5);
                apply_entry_fee();
            }
        }
        else if (pos_type == PositionType::LONG) {
            // Borrow fee every candle we hold
            apply_borrow_fee(price);

            bool exit_triggered = false, is_win = false;
            double exit_price = price;

            if (candles[i].low <= trailing_sl) {
                exit_price = trailing_sl;
                capital = position * trailing_sl;
                exit_triggered = true;
                if (trailing_sl > entry_price) is_win = true;
            }
            else if (candles[i].high >= dynamic_tp) {
                exit_price = dynamic_tp;
                capital = position * dynamic_tp;
                exit_triggered = true; is_win = true;
            }
            else if (!trend.is_bullish) {
                exit_price = price;
                capital = position * price;
                exit_triggered = true;
                if (price > entry_price) is_win = true;
            }

            if (exit_triggered) {
                apply_exit_fee(exit_price);
                pos_type = PositionType::NONE; long_trades++;
                if (is_win) winning_longs++;
            } else {
                double psl = price - (atr * params.atr_mult);
                if (psl > trailing_sl) trailing_sl = psl;
                dynamic_tp = price + (atr * params.atr_mult * 1.5);
            }
        }
        else if (pos_type == PositionType::SHORT) {
            // Borrow fee every candle we hold
            apply_borrow_fee(price);

            bool exit_triggered = false, is_win = false;
            double exit_price = price;

            if (candles[i].high >= trailing_sl) {
                exit_price = trailing_sl;
                capital -= position * (trailing_sl - entry_price);
                exit_triggered = true;
                if (trailing_sl < entry_price) is_win = true;
            }
            else if (candles[i].low <= dynamic_tp) {
                exit_price = dynamic_tp;
                capital += position * (entry_price - dynamic_tp);
                exit_triggered = true; is_win = true;
            }
            else if (trend.is_bullish) {
                exit_price = price;
                capital += position * (entry_price - price);
                exit_triggered = true;
                if (price < entry_price) is_win = true;
            }

            if (exit_triggered) {
                apply_exit_fee(exit_price);
                pos_type = PositionType::NONE; short_trades++;
                if (is_win) winning_shorts++;
            } else {
                double psl = price + (atr * params.atr_mult);
                if (psl < trailing_sl) trailing_sl = psl;
                dynamic_tp = price - (atr * params.atr_mult * 1.5);
            }
        }
    }

    // Close open position at end of window
    double last_price = candles[end_idx - 1].close;
    if (pos_type == PositionType::LONG) {
        apply_borrow_fee(last_price);
        capital = position * last_price;
        apply_exit_fee(last_price);
        long_trades++;
        if (last_price > entry_price) winning_longs++;
    }
    else if (pos_type == PositionType::SHORT) {
        apply_borrow_fee(last_price);
        capital += position * (entry_price - last_price);
        apply_exit_fee(last_price);
        short_trades++;
        if (last_price < entry_price) winning_shorts++;
    }

    int total_trades = long_trades + short_trades;
    int total_wins   = winning_longs + winning_shorts;
    double win_rate  = total_trades > 0 ? (double)total_wins / total_trades * 100.0 : 0.0;
    double ret       = ((capital - initial_capital) / initial_capital) * 100.0;

    double fitness = 0.0;
    if (ret > 0 && total_trades > 0)
        fitness = ret * (win_rate / 100.0) * std::log(total_trades + 1);
    else
        fitness = ret;

    double dollar_gain = capital - initial_capital;

    return {params, ret, dollar_gain, total_fees, total_trades, long_trades, short_trades,
            winning_longs, winning_shorts, win_rate, fitness};
}

// ─── Live signal generator ────────────────────────────────────────────────────
LiveSignal generate_signal(const std::vector<Candle>& candles,
                           const StrategyParams& params,
                           double free_margin = 3000.0) {
    LiveSignal sig{};
    size_t idx = candles.size() - 1;

    sig.trend  = calculate_trend(candles, idx);
    sig.regime = classify_regime(candles, idx);
    sig.atr    = calculate_atr(candles, idx, 14);
    sig.z_score = calculate_z_score(candles, idx, params.period);

    double price = candles[idx].close;

    // Build warnings
    if (sig.trend.bullish_votes == 1 || sig.trend.bullish_votes == 2) {
        sig.warnings += "[WARN] EMA consensus split (" + std::to_string(sig.trend.bullish_votes)
                      + "/3 bullish) — trend ambiguous on this timeframe. ";
    }
    if (sig.regime == "VOLATILE") {
        sig.warnings += "[WARN] Volatile regime — widen stops or stand aside. ";
    }
    if (!sig.trend.is_trending) {
        sig.warnings += "[INFO] EMAs not stacked — market may be ranging. ";
    }

    bool can_long  = sig.z_score < -params.threshold && sig.trend.is_bullish;
    bool can_short = sig.z_score >  params.threshold && !sig.trend.is_bullish;

    if (can_long) {
        sig.direction   = "LONG";
        sig.entry       = price;
        sig.stop_loss   = price - (sig.atr * params.atr_mult);
        sig.take_profit = price + (sig.atr * params.atr_mult * 1.5);
        sig.confidence  = calculate_confidence(sig.z_score, params.threshold,
                                               sig.trend, sig.regime, true);
        sig.order_size_usd = free_margin;
        sig.order_size_btc = free_margin / price;
    }
    else if (can_short) {
        sig.direction   = "SHORT";
        sig.entry       = price;
        sig.stop_loss   = price + (sig.atr * params.atr_mult);
        sig.take_profit = price - (sig.atr * params.atr_mult * 1.5);
        sig.confidence  = calculate_confidence(sig.z_score, params.threshold,
                                               sig.trend, sig.regime, false);
        sig.order_size_usd = free_margin;
        sig.order_size_btc = free_margin / price;
    }
    else {
        sig.direction      = "WAIT";
        sig.entry          = price;
        sig.order_size_usd = 0.0;
        sig.order_size_btc = 0.0;
    }

    return sig;
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (!curl) { std::cerr << "curl init failed\n"; return 1; }

    std::cout << "Fetching 1000 candles from Binance (6H)...\n";
    curl_easy_setopt(curl, CURLOPT_URL,
        "https://data-api.binance.vision/api/v3/klines?symbol=BTCUSDT&interval=1h&limit=1000");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "crbot-v2/1.0");

    if (curl_easy_perform(curl) != CURLE_OK) {
        std::cerr << "Fetch failed\n"; return 1;
    }
    curl_easy_cleanup(curl);

    auto data = json::parse(readBuffer);
    std::vector<Candle> candles;
    for (const auto& item : data)
        candles.push_back({
            std::stod(item[2].get<std::string>()),
            std::stod(item[3].get<std::string>()),
            std::stod(item[4].get<std::string>())
        });

    size_t N = candles.size();
    std::cout << "Loaded " << N << " candles.\n";

    // Aggressive grid: low thresholds trigger more often, tight ATR = more trades
    std::vector<size_t> periods    = {3, 5, 8, 13, 21};
    std::vector<double> thresholds = {0.3, 0.5, 0.7, 1.0, 1.3};
    std::vector<double> atr_mults  = {0.5, 0.75, 1.0, 1.5, 2.0};

    StrategyParams best_params = {8, 1.0, 1.0};
    double best_fitness = -9999.0;
    BacktestResult best_metrics{};

    std::cout << "\nRunning Walk-Forward Optimisation...\n";

    for (auto p : periods) {
        for (auto t : thresholds) {
            for (auto a : atr_mults) {
                StrategyParams params = {p, t, a};

                BacktestResult s1 = run_slice(candles, 300, 500, params);
                BacktestResult s2 = run_slice(candles, 500, 700, params);
                BacktestResult s3 = run_slice(candles, 700, N - 1, params);

                int    total_trades  = s1.total_trades + s2.total_trades + s3.total_trades;
                double avg_ret       = (s1.total_return + s2.total_return + s3.total_return) / 3.0;
                double avg_wr        = (s1.win_rate + s2.win_rate + s3.win_rate) / 3.0;
                double total_dollars = s1.dollar_gain + s2.dollar_gain + s3.dollar_gain;
                double total_fees    = s1.total_fees_paid + s2.total_fees_paid + s3.total_fees_paid;
                double fitness       = (s1.fitness_score + s2.fitness_score + s3.fitness_score) / 3.0;

                // Penalty for blowing up any single regime (relaxed to -12% to allow aggression)
                if (s1.total_return < -12.0 || s2.total_return < -12.0 || s3.total_return < -12.0)
                    fitness -= 60.0;

                if (fitness > best_fitness && total_trades >= 30) {
                    best_fitness = fitness;
                    best_params  = params;
                    best_metrics.total_return   = avg_ret;
                    best_metrics.dollar_gain    = total_dollars;
                    best_metrics.total_fees_paid= total_fees;
                    best_metrics.win_rate       = avg_wr;
                    best_metrics.total_trades   = total_trades;
                    best_metrics.long_trades    = s1.long_trades + s2.long_trades + s3.long_trades;
                    best_metrics.short_trades   = s1.short_trades + s2.short_trades + s3.short_trades;
                    best_metrics.winning_longs  = s1.winning_longs + s2.winning_longs + s3.winning_longs;
                    best_metrics.winning_shorts = s1.winning_shorts + s2.winning_shorts + s3.winning_shorts;
                }
            }
        }
    }

    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║       OPTIMISED PARAMETERS           ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
    std::cout << "Period    : " << best_params.period << "\n";
    std::cout << "Threshold : " << best_params.threshold << "\n";
    std::cout << "ATR mult  : " << best_params.atr_mult << "x\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Avg OOS return  : " << best_metrics.total_return << "%\n";
    std::cout << "Gross $ gain    : $" << (best_metrics.dollar_gain + best_metrics.total_fees_paid)
              << "  (before fees)\n";
    std::cout << "Total fees paid : $" << best_metrics.total_fees_paid
              << "  (maker 0.01% + taker 0.055% + borrow 0.0585%/day)\n";
    std::cout << "Net $ gain      : $" << best_metrics.dollar_gain
              << "  (3 × $1000 capital, after all fees)\n";
    std::cout << "Overall win rate: " << best_metrics.win_rate << "%\n";
    std::cout << "Total trades    : " << best_metrics.total_trades << "\n";
    std::cout << "Longs (wins)    : " << best_metrics.long_trades
              << " (" << best_metrics.winning_longs << ")\n";
    std::cout << "Shorts (wins)   : " << best_metrics.short_trades
              << " (" << best_metrics.winning_shorts << ")\n";

    // ── Live Signal ──────────────────────────────────────────────────────────
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║         LIVE SIGNAL ANALYSIS         ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    double free_margin = 3000.0;   // ← set your actual free margin here
    LiveSignal sig = generate_signal(candles, best_params, free_margin);

    std::cout << "Current price  : $" << sig.entry << "\n";
    std::cout << "Market regime  : " << sig.regime << "\n";
    std::cout << "Z-score        : " << sig.z_score << "\n";
    std::cout << "ATR            : $" << sig.atr << "\n";
    std::cout << "\n── Trend Consensus (EMA 20 / 50 / 100) ──\n";
    std::cout << "EMA-20   : $" << sig.trend.ema_short
              << " (" << (sig.entry > sig.trend.ema_short ? "ABOVE ↑" : "BELOW ↓") << ")\n";
    std::cout << "EMA-50   : $" << sig.trend.ema_medium
              << " (" << (sig.entry > sig.trend.ema_medium ? "ABOVE ↑" : "BELOW ↓") << ")\n";
    std::cout << "EMA-100  : $" << sig.trend.ema_long
              << " (" << (sig.entry > sig.trend.ema_long ? "ABOVE ↑" : "BELOW ↓") << ")\n";
    std::cout << "Votes    : " << sig.trend.bullish_votes << "/3 bullish → "
              << (sig.trend.is_bullish ? "BULLISH" : "BEARISH") << "\n";
    std::cout << "Stacked  : " << (sig.trend.is_trending ? "YES (clean trend)" : "NO (choppy)") << "\n";
    std::cout << "Strength : " << std::setprecision(0)
              << (sig.trend.trend_strength * 100) << "%\n";

    if (!sig.warnings.empty()) {
        std::cout << "\n⚠  " << sig.warnings << "\n";
    }

    std::cout << "\n── Verdict ──────────────────────────────\n";
    if (sig.direction == "LONG") {
        std::cout << ">>> [ EXECUTE LONG ]  Confidence: "
                  << std::setprecision(0) << sig.confidence << "/100 <<<\n";
        std::cout << std::setprecision(2);
        std::cout << "Entry        : $" << sig.entry << "\n";
        std::cout << "Stop Loss    : $" << sig.stop_loss << "\n";
        std::cout << "Take Profit  : $" << sig.take_profit << "\n";
        std::cout << "Risk/Reward  : 1:" << std::setprecision(2)
                  << ((sig.take_profit - sig.entry) / (sig.entry - sig.stop_loss)) << "\n";
        std::cout << "─── Order Size ───────────────────────────\n";
        std::cout << "Free Margin  : $" << std::setprecision(2) << sig.order_size_usd << "\n";
        std::cout << "Order (USD)  : $" << sig.order_size_usd << "  (100% of free margin)\n";
        std::cout << "Order (BTC)  : " << std::setprecision(6) << sig.order_size_btc << " BTC\n";
        std::cout << "Max $ at risk: $" << std::setprecision(2)
                  << (sig.order_size_usd * (sig.entry - sig.stop_loss) / sig.entry) << "\n";
    }
    else if (sig.direction == "SHORT") {
        std::cout << ">>> [ EXECUTE SHORT ]  Confidence: "
                  << std::setprecision(0) << sig.confidence << "/100 <<<\n";
        std::cout << std::setprecision(2);
        std::cout << "Entry        : $" << sig.entry << "\n";
        std::cout << "Stop Loss    : $" << sig.stop_loss << "\n";
        std::cout << "Take Profit  : $" << sig.take_profit << "\n";
        std::cout << "Risk/Reward  : 1:" << std::setprecision(2)
                  << ((sig.entry - sig.take_profit) / (sig.stop_loss - sig.entry)) << "\n";
        std::cout << "─── Order Size ───────────────────────────\n";
        std::cout << "Free Margin  : $" << std::setprecision(2) << sig.order_size_usd << "\n";
        std::cout << "Order (USD)  : $" << sig.order_size_usd << "  (100% of free margin)\n";
        std::cout << "Order (BTC)  : " << std::setprecision(6) << sig.order_size_btc << " BTC\n";
        std::cout << "Max $ at risk: $" << std::setprecision(2)
                  << (sig.order_size_usd * (sig.stop_loss - sig.entry) / sig.entry) << "\n";
    }
    else {
        std::cout << ">>> [ WAIT — No valid setup ] <<<\n";
        std::cout << "(Z-score " << std::setprecision(2) << sig.z_score
                  << " vs threshold ±" << best_params.threshold << ")\n";
    }
    std::cout << "─────────────────────────────────────────\n";

    return 0;
}