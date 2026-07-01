// sudo apt update && sudo apt install libcurl4-openssl-dev pkg-config
// mkdir -p include/nlohmann && wget -O include/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
// g++ -Os cwin.cpp -o cwin -I./include -Wl,-Bstatic $(pkg-config --static --libs libcurl) -Wl,-Bdynamic -lpthread -ldl
// g++ -std=c++20 cwin.cpp  -o cwin -I./include  $(pkg-config  --libs libcurl) -lcrypto -lssl  -lpthread -ldl -Os -s  -ffunction-sections -fdata-sections -Wl,--gc-sections  -fno-rtti -flto&& ./cwin



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
// #include <nlohmann/json.hpp>
// region mexc
#include "cmexc.cpp"
// void openAtomicBracketFuturesPosition(const std::string& symbol, const std::string& side, double qty, double entryPrice, double stopLoss, double takeProfit, int leverage = 20);
// int print_account();

// using json = nlohmann::json;
using namespace std;

struct Candle { long long timestamp; double high, low, close; };
struct StrategyParams { size_t period; double threshold, atr_mult; };
struct FeeConfig {
    // double maker_fee = 0.000, taker_fee = 0.000,  borrow_daily = 0.0003,
    double maker_fee = 0.0006, taker_fee = 0.0008, borrow_daily = 0.000,
     candle_hours = 1.0;      
    double borrow_per_candle() const { return borrow_daily * (candle_hours / 24.0); }
};

// Fixed Target Risk to 4%
static constexpr double RISK_PER_TRADE = 0.03;
enum class PositionType { NONE, LONG, SHORT };

struct BacktestResult {
    StrategyParams params; double total_return, dollar_gain, total_fees_paid, profit_factor;
    int total_trades, long_trades, short_trades, winning_longs, winning_shorts;
    double win_rate, fitness_score, max_drawdown;
    std::map<std::string, double> daily_profit; std::vector<double> daily_returns;
    std::vector<double> trade_pcts;
};

struct TrendReading { double ema_short, ema_medium, ema_long; int bullish_votes; bool is_bullish, is_trending; double trend_strength; };
struct LiveSignal {
    std::string direction, regime, warnings; double confidence, entry, stop_loss, take_profit, z_score, atr;
    double order_size_usd, order_size_btc, risk_amount, stop_distance, exact_position_btc, exact_position_usd;
    TrendReading trend; 
};

struct MonteCarloResult { double prob_of_ruin; double worst_case_drawdown; double median_final_equity; };

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

TrendReading calculate_trend(const std::vector<Candle>& candles, size_t up_to_idx, size_t short_p = 20, size_t medium_p = 50, size_t long_p = 100) {
    TrendReading t{};
    if (up_to_idx < long_p || candles.empty()) return t;
    auto calc_ema = [&](size_t period) {
        double ema = candles[0].close, k = 2.0 / (period + 1);
        for (size_t i = 1; i <= up_to_idx; ++i) ema = candles[i].close * k + ema * (1.0 - k);
        return ema;
    };
    double price = candles[up_to_idx].close;
    t.ema_short = calc_ema(short_p); t.ema_medium = calc_ema(medium_p); t.ema_long = calc_ema(long_p);
    t.bullish_votes = (price > t.ema_short) + (price > t.ema_medium) + (price > t.ema_long);
    t.is_bullish = (t.bullish_votes >= 2);
    t.is_trending = (t.ema_short > t.ema_medium && t.ema_medium > t.ema_long) || (t.ema_short < t.ema_medium && t.ema_medium < t.ema_long);
    double atr = calculate_atr(candles, up_to_idx, 14);
    if (atr > 0) t.trend_strength = std::min(1.0, std::abs(price - (t.ema_short + t.ema_medium + t.ema_long) / 3.0) / (atr * 3.0));
    return t;
}

std::string classify_regime(const std::vector<Candle>& candles, size_t idx, size_t lookback = 20) {
    if (idx < lookback + 14) return "UNKNOWN";
    if ((calculate_atr(candles, idx, 14) / candles[idx].close) * 100.0 > 3.5) return "VOLATILE";
    return calculate_trend(candles, idx).is_trending && (calculate_atr(candles, idx, 14) / candles[idx].close) * 100.0 < 2.5 ? "TRENDING" : "RANGING";
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

BacktestResult run_backtest(const std::vector<Candle>& candles, const StrategyParams& params, const FeeConfig& fees = FeeConfig{}) {
    double capital = 1000.0, initial_capital = capital, position = 0.0, total_fees = 0.0, gross_wins = 0.0, gross_losses = 0.0;
    double entry_price = 0.0, trailing_sl = 0.0, dynamic_tp = 0.0, peak_capital = capital, current_trade_fee = 0.0;
    double current_trade_start_cap = capital; // FIXED: Tracks capital at start of specific trade
    int long_trades = 0, short_trades = 0, winning_longs = 0, winning_shorts = 0;
    PositionType pos_type = PositionType::NONE;
    std::map<std::string, double> daily_profit; std::vector<double> daily_returns; std::vector<double> trade_pcts;

    auto log_daily = [&](long long ts, double pnl) {
        time_t t = ts / 1000; char buf[11]; std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::gmtime(&t));
        double net = pnl - current_trade_fee; daily_profit[buf] += net;
        daily_returns.push_back(net / initial_capital * 100.0); current_trade_fee = 0.0;
    };
    auto apply_fee = [&](double rate, double notional) { double fee = notional * rate; capital -= fee; total_fees += fee; current_trade_fee += fee; };

    for (size_t i = std::max((size_t)101, params.period + 1); i < candles.size(); ++i) {
        double price = candles[i].close, atr = calculate_atr(candles, i, 14);
        TrendReading trend = calculate_trend(candles, i);
        if (pos_type != PositionType::NONE) apply_fee(fees.borrow_per_candle(), std::abs(position) * price);

        if (pos_type == PositionType::NONE) {
            double z = calculate_z_score(candles, i, params.period), stop_dist = atr * params.atr_mult;
            if (z < -params.threshold && trend.is_bullish && trend.is_trending && trend.bullish_votes >= 2) {
                current_trade_start_cap = capital; // FIXED: Snapshot capital here
                position = (capital * RISK_PER_TRADE) / stop_dist; entry_price = price;
                pos_type = PositionType::LONG; trailing_sl = price - stop_dist; dynamic_tp = price + stop_dist * 2.0;
                apply_fee(fees.maker_fee, position * price);
            } else if (z > params.threshold && !trend.is_bullish && trend.is_trending && trend.bullish_votes <= 1) {
                current_trade_start_cap = capital; // FIXED: Snapshot capital here
                position = (capital * RISK_PER_TRADE) / stop_dist; entry_price = price;
                pos_type = PositionType::SHORT; trailing_sl = price + stop_dist; dynamic_tp = price - stop_dist * 2.0;
                apply_fee(fees.maker_fee, position * price);
            }
        } else {
            bool is_l = (pos_type == PositionType::LONG), exit = false; double pnl = 0.0, exit_price = price;
            if ((is_l && candles[i].low <= trailing_sl) || (!is_l && candles[i].high >= trailing_sl)) {
                exit_price = trailing_sl; pnl = position * (is_l ? trailing_sl - entry_price : entry_price - trailing_sl); exit = true;
            } else if ((is_l && candles[i].high >= dynamic_tp) || (!is_l && candles[i].low <= dynamic_tp)) {
                exit_price = dynamic_tp; pnl = position * (is_l ? dynamic_tp - entry_price : entry_price - dynamic_tp); exit = true;
            } else if ((is_l && !trend.is_bullish) || (!is_l && trend.is_bullish)) {
                pnl = position * (is_l ? price - entry_price : entry_price - price); exit = true;
            } else {
                double n_sl = price + (is_l ? -1 : 1) * (atr * params.atr_mult);
                if ((is_l && n_sl > trailing_sl) || (!is_l && n_sl < trailing_sl)) trailing_sl = n_sl;
                dynamic_tp = price + (is_l ? 1 : -1) * (atr * params.atr_mult * 2.0);
            }
            if (exit) {
                double net_pnl = pnl - current_trade_fee;
                capital += pnl; *(pnl >= 0 ? &gross_wins : &gross_losses) += std::abs(pnl);
                apply_fee(fees.taker_fee, std::abs(position) * exit_price); log_daily(candles[i].timestamp, pnl);
                
                // FIXED: Store the percentage return of the trade based on capital at entry
                trade_pcts.push_back(net_pnl / current_trade_start_cap); 
                
                if (capital <= 0) capital = 0.01; pos_type = PositionType::NONE;
                (is_l ? long_trades : short_trades)++; if (pnl > 0) (is_l ? winning_longs : winning_shorts)++;
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
    
    // FIXED: Made random generator static to prevent OS entropy pool exhaustion
    static std::random_device rd; 
    static std::mt19937 g(rd());
    
    for (int sim = 0; sim < iterations; ++sim) {
        std::vector<double> shuffled_pcts = actual_trade_pcts;
        std::shuffle(shuffled_pcts.begin(), shuffled_pcts.end(), g);
        double sim_capital = start_capital, sim_peak = start_capital, sim_max_dd = 0.0;
        bool ruined = false;
        
        for (double pct : shuffled_pcts) {
            // FIXED: Apply compounding percentage instead of absolute dollar values
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

LiveSignal generate_signal(const std::vector<Candle>& candles, size_t eval_idx, const StrategyParams& params, double free_margin) {
    LiveSignal sig{}; if (eval_idx < 101 || eval_idx >= candles.size()) return sig;
    
    sig.trend = calculate_trend(candles, eval_idx); sig.regime = classify_regime(candles, eval_idx);
    sig.atr = calculate_atr(candles, eval_idx, 14); sig.z_score = calculate_z_score(candles, eval_idx, params.period);
    
    double price = candles[eval_idx].close, stop_dist = sig.atr * params.atr_mult;
    sig.stop_distance = stop_dist; 
    sig.risk_amount = free_margin * RISK_PER_TRADE; // e.g. 1000 * 0.03 = 30.0
    
    // Math Correction: Removed LEVERAGE multiplier. Size is purely $Risk / StopDistance.
    sig.exact_position_btc = sig.order_size_btc = (stop_dist > 0.0) ? (sig.risk_amount / stop_dist) : 0.0;
    sig.exact_position_usd = sig.order_size_usd = sig.exact_position_btc * price;

    if (sig.trend.bullish_votes == 1 || sig.trend.bullish_votes == 2) sig.warnings += "[WARN] EMA split. ";
    if (sig.regime == "VOLATILE") sig.warnings += "[WARN] Volatile. ";
    if (!sig.trend.is_trending) sig.warnings += "[INFO] Ranging. ";

    bool can_l = sig.z_score < -params.threshold && sig.trend.is_bullish && sig.trend.is_trending && sig.trend.bullish_votes >= 2;
    bool can_s = sig.z_score >  params.threshold && !sig.trend.is_bullish && sig.trend.is_trending && sig.trend.bullish_votes <= 1;
    sig.direction = can_l ? "LONG" : (can_s ? "SHORT" : "WAIT");
    sig.entry = price;
    sig.stop_loss = price + (can_l ? -1 : 1) * (can_l || can_s ? stop_dist : 0);
    sig.take_profit = price + (can_l ? 1 : -1) * (can_l || can_s ? stop_dist * 2.0 : 0);
    sig.confidence = (can_l || can_s) ? calculate_confidence(sig.z_score, params.threshold, sig.trend, sig.regime, can_l) : 0.0;
    return sig;
}
struct symbolstruct {
    string name;
    float stepsize;
    int digits;
    string mexc;
};

vector<symbolstruct> symbols = {
    {"BTCUSDT", 0.001f,   2, "BTC_USDT"},
    {"ETHUSDT", 0.01f,   2, "ETH_USDT"},
    {"TRXUSDT", 0.00001f, 5, "TRX_USDT"},
    {"XRPUSDT", 0.0001f,  4, "XRP_USDT"},
    {"ADAUSDT", 0.0001f,  4, "ADA_USDT"},
    {"SOLUSDT", 0.001f,   3, "SOL_USDT"},
    {"AVAXUSDT",0.001f,   3, "AVAX_USDT"},
    {"SUIUSDT", 0.0001f,  4, "SUI_USDT"},
    {"TIAUSDT", 0.0001f,  4, "TIA_USDT"},
    {"LINKUSDT",0.001f,   3, "LINK_USDT"},
    {"DOGEUSDT",0.00001f, 5, "DOGE_USDT"},
    {"LTCUSDT", 0.001f,   3, "LTC_USDT"},
    {"NEARUSDT",0.001f,   3, "NEAR_USDT"},
    {"BNBUSDT", 0.001f,   3, "BNB_USDT"}
};
inline double roundPrice(double price, int digits)
{
    const double factor = std::pow(10.0, digits);
    return std::floor(price * factor) / factor;
}



std::vector<double> extractArray(const std::string& src, const std::string& key) {
    std::vector<double> out;

    std::string tag = "\"" + key + "\":[";
    size_t start = src.find(tag);
    if (start == std::string::npos) return out;

    start += tag.size();
    size_t end = src.find("]", start);
    if (end == std::string::npos) return out;

    std::string arr = src.substr(start, end - start);

    // split by comma
    size_t pos = 0;
    while (true) {
        size_t comma = arr.find(",", pos);
        std::string num = (comma == std::string::npos)
            ? arr.substr(pos)
            : arr.substr(pos, comma - pos);

        if (!num.empty()) out.push_back(std::stod(num));
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }

    return out;
}

int seek(int idsmb) {
	bool dbg=0;
	// int idsmb=1;
	float stepsize=symbols[idsmb].stepsize;

CURL* curl = curl_easy_init();
std::string readBuffer;

std::vector<Candle> candles;

if (curl) {
    std::string url =
        "https://contract.mexc.com/api/v1/contract/kline/" +
        symbols[idsmb].mexc +
        "?interval=Min60&limit=1000";
	// url="https://contract.mexc.com/api/v1/contract/kline/BTC_USDT?interval=Min60&limit=1000";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    if (curl_easy_perform(curl) == CURLE_OK) {

        // --- parse manual ---
        std::vector<double> t = extractArray(readBuffer, "time");
        std::vector<double> o = extractArray(readBuffer, "open");
        std::vector<double> h = extractArray(readBuffer, "high");
        std::vector<double> l = extractArray(readBuffer, "low");
        std::vector<double> c = extractArray(readBuffer, "close");

        size_t n = t.size();
        candles.reserve(n);

        for (size_t i = 0; i < n; ++i) {
            candles.push_back({
                (long long)t[i] * 1000LL,   // mexc time is seconds
                h[i],
                l[i],
                c[i]
            });
        }

        std::cout << "Loaded " << candles.size() << " MEXC futures candles.\n";
    }
    else {
        std::cout << "Fetch failed.\n";
    }

    curl_easy_cleanup(curl);
}

	// string symbol=
	// int idsmb=symbols.size()-1;


    // CURL* curl = curl_easy_init(); std::string readBuffer;
    // if(dbg)std::cout << "=== Professional 1H Bot: Real Data + Optimization + Walk-Forward ===\n";
    // std::vector<Candle> candles;
    
    // if (curl) {
    //     if(dbg)std::cout << "Fetching 1000 "+symbols[idsmb].name+" 1h candles from Binance...\n";
    //     std::string url = "https://data-api.binance.vision/api/v3/klines?symbol=" 
    //                   + symbols[idsmb].name 
    //                   + "&interval=1h&limit=1000";
    // 	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //     curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    //     if (curl_easy_perform(curl) == CURLE_OK) {
    //         try {
    //             auto data = json::parse(readBuffer);
    //             for (const auto& item : data) candles.push_back({item[0].get<long long>(), std::stod(item[2].get<std::string>()), std::stod(item[3].get<std::string>()), std::stod(item[4].get<std::string>())});
    //             std::cout << "Loaded " << candles.size() << " real candles.\n";
    //         } catch (...) { std::cout << "JSON parse failed.\n"; }
    //     } else { std::cout << "Fetch failed.\n"; }
    //     curl_easy_cleanup(curl);
    // }
    if (candles.size() < 101) { std::cout << "ERROR: Insufficient data. Exiting.\n"; return 1; }
    candles.resize(candles.size() - 1); //////////////////////////
    
    size_t last_closed_idx = candles.size() - 2;
    if(dbg)std::cout << "Last Closed Candle: $" << std::fixed << std::setprecision(6) << candles[last_closed_idx].close << "\n\nRunning Robust Optimization...\n";

	StrategyParams best_params = {10, 1.0, 1.5}; 
    double best_robust_fitness = -9999.0; 
    BacktestResult best_metrics{};
    MonteCarloResult best_mc{};

    for (size_t p : {5, 8, 10, 13, 20}) {
        for (auto t : {0.8, 1.0, 1.2, 1.5}) {
            for (auto a : {1.2, 1.5, 1.8, 2.2}) {
                
                BacktestResult res = run_backtest(candles, {p, t, a});
                
                if (res.total_trades >= 20 && res.max_drawdown < 35.0) {
                    MonteCarloResult mc_test = run_monte_carlo(res.trade_pcts, 1000.0, 500);
                    
                    if (mc_test.prob_of_ruin <= 0.05 && mc_test.worst_case_drawdown < 40.0) {
                        double robust_fitness = res.total_return * (1.0 - (mc_test.worst_case_drawdown / 100.0));
                        if (robust_fitness > best_robust_fitness) {
                            best_robust_fitness = robust_fitness;
                            best_params = {p, t, a};
                            best_metrics = res;
                            best_mc = mc_test; 
                        }
                    }
                }
            }
        }
    }
    
    if(dbg)std::cout << "Best Params: Period=" << best_params.period << " Th=" << best_params.threshold << " ATR=" << best_params.atr_mult << "\n"
              << "Standard Return: " << best_metrics.total_return << "% | Historical DD: " << best_metrics.max_drawdown << "% | Trades: " << best_metrics.total_trades << "\n";

    MonteCarloResult mc = run_monte_carlo(best_metrics.trade_pcts, 1000.0, 2000);
    if(dbg)std::cout << "\n================= MONTE CARLO STRESS TEST =================\n"
              << "Simulations Run        : 2,000 Shuffled Iterations\n"
              << "Risk of Ruin (30% DD)  : " << std::setprecision(6) << mc.prob_of_ruin << "%\n"
              << "95% Conf. Max Drawdown : " << mc.worst_case_drawdown << "%\n"
              << "Median Final Equity    : $" << mc.median_final_equity << "\n";

			  if(dbg)std::cout << "\n================ 10-CANDLE WALK-FORWARD ANALYSIS ================\n";
			  if(dbg)std::cout << std::left << std::setw(18) << "Time" << std::setw(8) << "Action" 
              << std::setw(12) << "Entry" << std::setw(12) << "Stop Loss" << "Take Profit\n";
			  if(dbg)std::cout << "-----------------------------------------------------------------\n";

	int count=0;
    // for (size_t i = 101; i <= last_closed_idx; ++i) {
	if(dbg)for (size_t i = last_closed_idx - 19; i <= last_closed_idx; ++i) {
        time_t t = candles[i].timestamp / 1000; char buf[20];
        std::strftime(buf, sizeof(buf), "%m-%d %H:%M", std::gmtime(&t));
        
        LiveSignal sig = generate_signal(candles, i, best_params, 1000.0);
        
        std::cout << std::left << std::setw(18) << buf << std::setw(8) << sig.direction;
        if (sig.direction != "WAIT") {
			count++;
            std::cout << std::setw(12) << sig.entry << std::setw(12) << sig.stop_loss << sig.take_profit;
        } else {
            std::cout << std::setw(12) << "-" << std::setw(12) << "-" << "-";
        }
        std::cout << "\n";
    }
	if(dbg)cout<<"count "<<count<<"\n";
    if(dbg)printf("LAST_10_CANDLES HLC:\n");
    if(dbg)for (int i = (int)candles.size() - 20; i < (int)candles.size(); i++) {
        time_t t = candles[i].timestamp / 1000; char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::gmtime(&t));
        printf("%s %.6f %.6f %.6f\n", buf, candles[i].high, candles[i].low, candles[i].close);
    }
    if(dbg)std::cout << "\n==================== LIVE SIGNAL EDGE ====================\n";
    // Fixed: Passing 1000.0 as Capital for Signal Generation to match backtest logic
    LiveSignal edge_sig = generate_signal(candles, last_closed_idx, best_params, 56.0);
    double notional = edge_sig.exact_position_btc * edge_sig.entry;
    double leverage_needed = notional / 56;   // FREE_MARGIN = 100 USD in your 
    if(dbg)std::cout << "Leverage Needed   : " << leverage_needed << "x\n";
	if(dbg)std::cout << "Current Candle close: $" << std::fixed << std::setprecision(6) << candles[candles.size()-1].close<<"\n";
    
	// if(dbg)print_account();
    if (edge_sig.direction != "WAIT") 
	// if(1)
	{
        double projected_loss = edge_sig.exact_position_btc * std::abs(edge_sig.entry - edge_sig.stop_loss);
        double projected_win = edge_sig.exact_position_btc * std::abs(edge_sig.entry - edge_sig.take_profit);
		double qty = std::floor(edge_sig.exact_position_btc / stepsize) * stepsize;
		int d = symbols[idsmb].digits;
        
        printf("EXECUTE: %s %s %f %f %f %f \n", symbols[idsmb].name.c_str(), (edge_sig.direction == "LONG" ? "BUY" : "SELL"), 
               edge_sig.exact_position_btc, edge_sig.entry, edge_sig.stop_loss, edge_sig.take_profit);
        std::cout << "Target Risk %  : " << (RISK_PER_TRADE * 100.0) << "%\n";
        std::cout << "Projected Loss : -$"  << projected_loss << " (Hit SL)\n";
        std::cout << "Projected Win  : +$"   << projected_win << " (Hit TP)\n";
		cout<<"entry: "<<edge_sig.entry<<" "<<roundPrice(edge_sig.entry, d)<<"\n";
		cout<<"qty: "<<qty<<"\n";
        // return 0;
        // print_account();
		openAtomicBracketFuturesPosition(
			symbols[idsmb].mexc,
			edge_sig.direction == "LONG" ? "BUY" : "SELL",
			edge_sig.exact_position_btc,
			edge_sig.entry,
			edge_sig.stop_loss,
			edge_sig.take_profit 
			// roundPrice(edge_sig.entry, d),
			// roundPrice(edge_sig.stop_loss, d),
			// roundPrice(edge_sig.take_profit, d),
			// d,
			// stepsize
		);
			   
    } else {
        if(dbg)std::cout << "No active trade edge condition met on the last closed candle. Holding.\n";
    }

    if(dbg)if (!edge_sig.warnings.empty()) std::cout << "\n--- Edge Warnings -------------\n" << edge_sig.warnings << "\n";
    return 0;
}
int main(){
	// print_account();
	// return 0;
	// seek(0);
	std::string openPositions = getOpenedFuturesPositions();
    std::cout << "Open Positions Data:\n" << openPositions << "\n" << std::endl;
	vector<std::string> gops=getOpenPositionSymbols();
	cout<<"gopsize: "<<gops.size()<<"\n";
	for(int i=0;i<gops.size();i++){
		cout<<"gops: "<<gops[i]<<"\n";
	}
	// return 0;
	// seek(1);return 0;
	for(int i=0;i<symbols.size();i++){
		seek(i);
	}
	return 0;
}