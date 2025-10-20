// g++ -std=c++17 crbot.cpp -o crbot -ljsoncpp -lcpp-httplib -lcrypto -lssl

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <memory>

// Include for cpp-httplib (now installed via libcpp-httplib-dev)
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h> 

// Cryptography Library (OpenSSL)
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// JSON Library (JsonCpp)
#include <jsoncpp/json/json.h> // Correct path when using libjsoncpp-dev

#include "general.hpp"
using namespace std;


// --- Configuration & Constants ---
const std::string API_BASE_URL = "https://whitebit.com";
const std::string ENDPOINT = "/api/v4/collateral-account/summary";
const int SIGNATURE_SIZE = 64; // SHA-512 outputs 64 bytes

// --- Helper Functions (No Classes) ---

// --- Helper Functions (Corrected Base64 Encode) ---

/**
 * @brief Base64 encodes an input string.
 * Uses a corrected pattern to ensure proper encoding and no newlines/nulls.
 * @param input The string to encode.
 * @return The Base64 encoded string.
 */
std::string base64_encode(const std::string& input) {
    // 1. Create a Base64 filter BIO and a memory BIO to write the output
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio_out = BIO_new(BIO_s_mem());
    
    // Push the Base64 filter onto the output memory sink
    b64 = BIO_push(b64, bio_out);

    // Disable newline insertion (Crucial for HTTP standards)
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); 

    // 2. Write the input data through the Base64 filter
    BIO_write(b64, input.data(), input.length());
    
    // Crucial: Finalize the Base64 encoding to flush any buffered output and padding
    BIO_flush(b64);

    // 3. Get the final output data from the memory BIO
    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio_out, &bptr);
    
    // Create the output string from the buffer data and length
    std::string output(bptr->data, bptr->length);

    // 4. Cleanup
    // We only free the b64 bio, which also frees the bio_out attached to it.
    BIO_free_all(b64); 

    // The output string should already be clean, but we can return it directly.
    return output;
}
/**
 * @brief Computes HMAC SHA512 signature.
 * @param data The data to be signed (Base64 payload).
 * @param key The secret key.
 * @return The hexadecimal encoded signature.
 */
std::string hmac_sha512(const std::string& data, const std::string& key) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    HMAC(EVP_sha512(), 
         key.c_str(), key.length(),
         (unsigned char*)data.c_str(), data.length(), 
         digest, &digest_len);

    if (digest_len != SIGNATURE_SIZE) {
        throw std::runtime_error("HMAC-SHA512 digest length is incorrect.");
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SIGNATURE_SIZE; ++i) {
        ss << std::setw(2) << (int)digest[i];
    }

    return ss.str();
}

/**
 * @brief Parses a JSON string using JsonCpp.
 * @param json_str The raw JSON string.
 * @return The Json::Value object.
 * @throws std::runtime_error if parsing fails.
 */
Json::Value parse_json(const std::string& json_str) {
    Json::Value root_value;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(
        json_str.c_str(),
        json_str.c_str() + json_str.size(),
        &root_value,
        &errors
    );

    if (!parsingSuccessful) {
        throw std::runtime_error("JSON parsing failed: " + errors);
    }
    return root_value;
}

// --- Core API Function (No Classes) ---

/**
 * @brief Gets collateral account equity using HMAC-SHA512 POST request.
 * @return The total equity (margin + freeMargin) as a double.
 */
double get_equity() {
    // 1. Get API Keys from Environment Variables
    const char* api_key_env = std::getenv("WB_KEY");
    const char* secret_key_env = std::getenv("WB_SECRET");

    if (!api_key_env || !secret_key_env) {
        throw std::runtime_error("API keys not set. Please set WB_KEY and WB_SECRET environment variables.");
    }
    const std::string api_key = api_key_env;
    const std::string secret_key = secret_key_env;

    // 2. Build Request Body
    Json::Value body_obj;
    body_obj["request"] = ENDPOINT;
    
    // Calculate the nonce (current time in milliseconds)
    // IMPORTANT: WhiteBIT nonces must be strictly increasing.
    std::string current_time_ms = std::to_string(std::time(nullptr) * 1000); 

    // Use Json::Value(string) to ensure the output is a JSON string (with quotes)
    body_obj["nonce"] = Json::Value(current_time_ms);
    // body_obj["time"] = Json::Value(current_time_ms); // Manditory for private endpoints

    // Convert to string (bodyStr in JS)
    Json::StreamWriterBuilder wbuilder;
    // Ensure the output is a compact string (no pretty-printing/newlines)
    wbuilder["commentStyle"] = "None"; 
    wbuilder["indentation"] = "";
    std::string body_str = Json::writeString(wbuilder, body_obj);
    
    // 📢 DEBUG OUTPUT: Raw JSON Body
    std::cout << "\n[DEBUG] Raw JSON Body: " << body_str << "\n";

    // 3. Encode Payload (Base64)
    std::string payload = base64_encode(body_str);
    
    // 📢 DEBUG OUTPUT: Base64 Payload
    std::cout << "[DEBUG] Base64 Payload: " << payload << "\n";

    // 4. Generate Signature
    std::string signature = hmac_sha512(payload, secret_key);
    
    // 📢 DEBUG OUTPUT: Signature
    std::cout << "[DEBUG] Signature: " << signature << "\n";

    // 5. Build HTTP Client and Headers
    httplib::Client cli(API_BASE_URL);

    // WhiteBIT requires a strict timeout. 
    cli.set_connection_timeout(5); // 5 seconds
    
    httplib::Headers headers = {
        {"Content-Type", "application/json"},
        {"X-TXC-APIKEY", api_key},
        {"X-TXC-PAYLOAD", payload},
        {"X-TXC-SIGNATURE", signature}
    };
    
    // 6. Send POST Request
    auto res = cli.Post(ENDPOINT.c_str(), headers, body_str, "application/json");

    // 7. Handle Response
    if (!res) {
        throw std::runtime_error("HTTP request failed: " + httplib::to_string(res.error()));
    }

    if (res->status != 200) {
        throw std::runtime_error("[HTTP " + std::to_string(res->status) + "] " + res->body);
    }
    
    // 8. Parse JSON and Extract Values
    Json::Value res_json = parse_json(res->body);
    
    // Extract 'margin' and 'freeMargin' (they are strings in the API response)
    if (!res_json.isMember("margin") || !res_json.isMember("freeMargin")) {
        throw std::runtime_error("API response is missing 'margin' or 'freeMargin' fields.");
    }
    
    // Convert string fields to double and sum them (JS's Number(...) equivalent)
    double total_margin = std::stod(res_json["margin"].asString());
    double free_margin = std::stod(res_json["freeMargin"].asString());

    return total_margin + free_margin;
}


// ... [Keep all existing includes, helper functions, and main] ...

// --- New Core API Function ---
 
/**
 * @brief Fetches klines (candlestick data) from Binance Data API.
 * @param symbol The trading pair (e.g., "ETHUSDT").
 * @param interval The candlestick interval (e.g., "6h").
 * @param limit The number of data points to retrieve (max 1000).
 * @return The raw JSON response string from the API.
 * @throws std::runtime_error if the request or response fails.
 */
std::string get_binance_klines(const std::string& symbol, const std::string& interval, int limit) {
    const std::string API_BASE = "https://data-api.binance.vision";
    const std::string ENDPOINT = "/api/v3/klines";

    // Build the query string with parameters
    std::stringstream query;
    query << ENDPOINT
          << "?symbol=" << symbol
          << "&interval=" << interval
          << "&limit=" << limit;
    
    std::string path = query.str();

    // 1. Build HTTP Client
    // Note: The cpp-httplib client handles the full URL, but we'll use the base URL for the client.
    httplib::Client cli(API_BASE);
    
    // Set a reasonable timeout
    cli.set_connection_timeout(10); // 10 seconds

    // 2. Send GET Request
    // Since this is a public endpoint, no custom headers are required.
    auto res = cli.Get(path.c_str());

    // 3. Handle Response
    if (!res) {
        throw std::runtime_error("Binance API request failed: " + httplib::to_string(res.error()));
    }

    if (res->status != 200) {
        // Binance returns JSON errors, which can be helpful.
        throw std::runtime_error("[Binance HTTP " + std::to_string(res->status) + "] " + res->body);
    }
    
    // 4. Return the raw JSON body
    return res->body;
}



// ... [Existing Includes] ...

// --- Data Structure ---

/**
 * @brief Represents a single candlestick (Kline) from the Binance API.
 * Uses double for prices and long long for timestamps to ensure precision.
 */
struct Kline {
    long long open_time;
    double open_price;
    double high_price;
    double low_price;
    double close_price;
    double volume;
    long long close_time;
    // We omit the rest of the fields for simplicity, but they can be added if needed.

    // A simple method for debugging/printing
    void print() const {
        std::cout << "  [Kline] Time: " << open_time 
                  << ", Open: " << std::fixed << std::setprecision(8) << open_price 
                  << ", Close: " << close_price 
                  << ", Volume: " << volume << "\n";
    }
};

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <limits>
#include <map>
#include <tuple>
#include <iostream>

// --- Assuming Kline struct is defined elsewhere ---
// struct Kline { double high_price; double low_price; double close_price; ... };

// --- Result Struct ---
struct ATRResult {
    std::vector<double> atr_values;
    double min_atr = std::numeric_limits<double>::max();
    double max_atr = std::numeric_limits<double>::lowest();
};

// --- Caching Definitions ---
namespace AtrCache {
    using KlineDataPtr = const std::vector<Kline>*;
    // CacheKey: (Ptr to Klines, period)
    using CacheKey = std::tuple<KlineDataPtr, int>;
    using CacheValue = ATRResult; // Cache stores the entire ATRResult struct
    
    static std::map<CacheKey, CacheValue> atr_cache;
} // End namespace AtrCache
/**
 * @brief Calculates the True Range (TR) for a given kline index.
 */
double calculate_true_range(const std::vector<Kline>& vklines, size_t i) {
    if (i == 0) {
        // For the very first bar, TR is simply High - Low
        return vklines[i].high_price - vklines[i].low_price;
    }
    
    double h_l = vklines[i].high_price - vklines[i].low_price;
    double h_c_prev = std::abs(vklines[i].high_price - vklines[i-1].close_price);
    double l_c_prev = std::abs(vklines[i].low_price - vklines[i-1].close_price);

    // Return the greatest of the three
    return std::max({h_l, h_c_prev, l_c_prev});
}
/**
 * @brief Calculates the Average True Range (ATR), caches the result, and returns the ATR vector
 * along with its minimum and maximum values.
 * @param vklines The vector of historical klines.
 * @param period The ATR period (default 14).
 * @return An ATRResult struct containing the ATR values and their min/max.
 */
ATRResult calculate_atr_cache(const std::vector<Kline>& vklines, int period = 14) {
    
    // --- Caching Check ---
    using AtrCache::KlineDataPtr;
    using AtrCache::CacheKey;
    
    KlineDataPtr klines_ptr = &vklines;
    CacheKey key = std::make_tuple(klines_ptr, period);

    auto it = AtrCache::atr_cache.find(key);
    
    if (it != AtrCache::atr_cache.end()) {
        // Cache hit
        return it->second;
    }

    // --- Calculation (Cache Miss) ---
    ATRResult result;
    size_t data_size = vklines.size();

    if (data_size <= (size_t)period) {
        std::cerr << "Error: Not enough data for ATR calculation (Need > " << period << " bars).\n";
        return result;
    }

    // A. Calculate True Range (TR)
    std::vector<double> tr_values(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        tr_values[i] = calculate_true_range(vklines, i);
    }
    
    // Resize the result vector
    result.atr_values.resize(data_size, 0.0);

    // B. Calculate Initial ATR (Simple Moving Average of TR)
    double initial_sum_tr = 0.0;
    for (int i = 0; i < period; ++i) {
        initial_sum_tr += tr_values[i];
    }
    // The first valid ATR value is at index 'period' (using TA standard)
    result.atr_values[period] = initial_sum_tr / period;

    // C. Calculate Subsequent ATRs (Wilder's Smoothing/EMA)
    for (size_t i = period + 1; i < data_size; ++i) {
        // ATR_i = [(ATR_{i-1} * (n-1)) + TR_i] / n
        result.atr_values[i] = ((result.atr_values[i-1] * (period - 1)) + tr_values[i]) / period;
    }
    
    // D. Determine Min/Max Range
    // Start min/max tracking from the first valid ATR value
    for (size_t i = period; i < data_size; ++i) {
        double current_atr = result.atr_values[i];
        result.min_atr = std::min(result.min_atr, current_atr);
        result.max_atr = std::max(result.max_atr, current_atr);
    }

    // --- Store and Return ---
    // Store the calculated result in the cache
    AtrCache::atr_cache[key] = result;

    return result;
}






// #include <iostream>
// #include <sstream>
// #include <stdexcept>
// #include <string>
// #include <vector>
// #include <json/json.h>
// #include "httplib.h"

// struct Kline {
//     long long open_time;
//     double open_price;
//     double high_price;
//     double low_price;
//     double close_price;
//     double volume;
//     long long close_time;
// };

/**
 * @brief Fetch klines from Binance with start/end time using JsonCpp.
 */
Json::Value get_binance_klines_range(const std::string& symbol,
                                     const std::string& interval,
                                     long long startTime,
                                     long long endTime) {
    const std::string API_BASE = "https://data-api.binance.vision";
    const std::string ENDPOINT = "/api/v3/klines";

    std::stringstream query;
    query << ENDPOINT
          << "?symbol=" << symbol
          << "&interval=" << interval
          << "&startTime=" << startTime
          << "&endTime=" << endTime;

    httplib::Client cli(API_BASE);
    cli.set_connection_timeout(10);

    auto res = cli.Get(query.str().c_str());
    if (!res) {
        throw std::runtime_error("Binance API request failed: " + httplib::to_string(res.error()));
    }
    if (res->status != 200) {
        throw std::runtime_error("[Binance HTTP " + std::to_string(res->status) + "] " + res->body);
    }

    Json::CharReaderBuilder rbuilder;
    Json::Value root;
    std::string errs;
    std::istringstream s(res->body);
    if (!Json::parseFromStream(rbuilder, s, &root, &errs)) {
        throw std::runtime_error("JSON parse error: " + errs);
    }

    return root;
}

/**
 * @brief Determines if high was reached before low within a 4h candle.
 * @param symbol Trading pair (e.g., "ETHUSDT").
 * @param candle The 4h candle struct.
 * @return true if high was reached before low, false otherwise.
 */
bool highFirst(const std::string& symbol, const Kline& candle) {
    // Fetch 1m klines inside the 4h window
    Json::Value data = get_binance_klines_range(symbol, "1m", candle.open_time, candle.close_time);

    long long tHigh = -1, tLow = -1;

    for (const auto& k : data) {
        long long t = k[0].asInt64(); // open time
        double high = std::stod(k[2].asString());
        double low  = std::stod(k[3].asString());

        if (tHigh == -1 && high >= candle.high_price) {
            tHigh = t;
        }
        if (tLow == -1 && low <= candle.low_price) {
            tLow = t;
        }
		// cotm(high,low)
        if (tHigh != -1 && tLow != -1) break;
    }

    if (tHigh == -1 || tLow == -1) {
        throw std::runtime_error("Não foi possível determinar ordem high/low");
    }

    return (tHigh < tLow);
}
/**
 * @brief Determines whether the high was reached before the low within a range of candles.
 * @param symbol The trading pair symbol (e.g., "BTCUSDT").
 * @param candles The full vector of candles (e.g., 4h candles).
 * @param fromindex Starting index (inclusive).
 * @param toindex Ending index (inclusive).
 * @return true if the high was reached before the low, false otherwise.
 * @throws std::runtime_error if the order cannot be determined.
 */
bool highFirst(const std::string& symbol, const std::vector<Kline>& candles, int fromindex, int toindex) {
    if (candles.empty() || fromindex < 0 || toindex >= (int)candles.size() || fromindex > toindex) {
        throw std::invalid_argument("Invalid candle range for highFirst()");
    }

    // Determine the overall time window
    const long long start_time = candles[fromindex].open_time;
    const long long end_time   = candles[toindex].close_time;

    // Compute target high/low across the selected candles
    double targetHigh = candles[fromindex].high_price;
    double targetLow  = candles[fromindex].low_price;
    for (int i = fromindex + 1; i <= toindex; ++i) {
        targetHigh = std::max(targetHigh, candles[i].high_price);
        targetLow  = std::min(targetLow, candles[i].low_price);
    }

    // Fetch 1m klines covering that full range
    Json::Value data = get_binance_klines_range(symbol, "1m", start_time, end_time);

    long long tHigh = -1;
    long long tLow  = -1;

    // Iterate through 1m candles to find when highs/lows were reached
    for (const auto& k : data) {
        long long t = k[0].asInt64();  // open time
        double high = std::stod(k[2].asString());
        double low  = std::stod(k[3].asString());

        if (tHigh == -1 && high >= targetHigh)
            tHigh = t;

        if (tLow == -1 && low <= targetLow)
            tLow = t;

        if (tHigh != -1 && tLow != -1)
            break;  // stop early when both found
    }

    if (tHigh == -1 || tLow == -1) {
        throw std::runtime_error("Não foi possível determinar ordem high/low para " + symbol);
    }

    // true = high hit first, false = low hit first
    return (tHigh < tLow);
}


// ... [Existing Helper Functions] ...
// ... [Existing get_equity function] ...
// ... [Existing get_binance_klines function] ...


// --- New Parsing Function ---

/**
 * @brief Parses the raw Binance Klines JSON array into a vector of Kline structs.
 * @param klines_json The raw JSON string from get_binance_klines.
 * @return A vector of Kline structs.
 * @throws std::runtime_error if JSON parsing fails or the format is unexpected.
 */
std::vector<Kline> parse_klines(const std::string& klines_json) {
    Json::Value root_value;
    try {
        root_value = parse_json(klines_json); // Reuse the existing parse_json helper
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse Binance Klines JSON: " + std::string(e.what()));
    }

    if (!root_value.isArray()) {
        throw std::runtime_error("Binance Klines response is not a JSON array.");
    }

    std::vector<Kline> klines_vec;
    
    // Iterate over the outer array (each element is a kline array)
    for (const auto& kline_arr : root_value) {
        if (!kline_arr.isArray() || kline_arr.size() < 6) {
            std::cerr << "Warning: Skipping malformed kline entry.\n";
            continue;
        }

        Kline kline;
        
        // 0: Open time (long long)
        kline.open_time = std::stoll(kline_arr[0].asString());
        
        // 1: Open price (double)
        kline.open_price = std::stod(kline_arr[1].asString());
        
        // 2: High price (double)
        kline.high_price = std::stod(kline_arr[2].asString());
        
        // 3: Low price (double)
        kline.low_price = std::stod(kline_arr[3].asString());
        
        // 4: Close price (double)
        kline.close_price = std::stod(kline_arr[4].asString());
        
        // 5: Volume (double)
        kline.volume = std::stod(kline_arr[5].asString());
        
        // 6: Close time (long long) - Only required for a full struct, but good practice
        if (kline_arr.size() > 6) {
            kline.close_time = std::stoll(kline_arr[6].asString());
        } else {
            kline.close_time = 0; 
        }

        klines_vec.push_back(kline);
    }

    return klines_vec;
}
#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>
#include <algorithm>
// ... [Existing Includes] ...

// --- RSI Calculation Function ---

/**
 * @brief Calculates the Relative Strength Index (RSI) for a vector of Klines.
 * * RSI Formula: RSI = 100 - (100 / (1 + RS))
 * Uses the Close price for calculation.
 * * @param klines The vector of Kline structs containing price data.
 * @param period The RSI period (Standard is 14).
 * @return A vector of int, representing the RSI values (0-100).
 * @throws std::runtime_error if there is insufficient data.
 */
std::vector<int> fill_rsi(const std::vector<Kline>& klines, int period = 14,float rscale=30) {
    // Need at least period + 1 bars to calculate the first RSI value.
    if (klines.size() < (size_t)period + 1) {
        throw std::runtime_error("Insufficient data for RSI calculation (Need at least " + 
                                 std::to_string(period + 1) + " bars).");
    }

    std::vector<int> rsi_values(klines.size(), 0);
    
    // --- Step 1: Calculate Gains and Losses ---
    // The size of daily_gains/losses is klines.size() - 1
    std::vector<double> daily_gains(klines.size() - 1);
    std::vector<double> daily_losses(klines.size() - 1);

    for (size_t i = 1; i < klines.size(); ++i) {
        double change = klines[i].close_price - klines[i - 1].close_price;
        
        if (change > 0) {
            daily_gains[i - 1] = change;
            daily_losses[i - 1] = 0.0;
        } else {
            daily_gains[i - 1] = 0.0;
            // Use absolute value for losses
            daily_losses[i - 1] = std::abs(change);
        }
    }

    // --- Step 2: Calculate Initial Averages (SMA for the first N periods) ---
    // The first RSI value is calculated at index 'period' (i.e., the 14th kline)
    
    // Sum of the first 'period' gains and losses (indices 0 to period-1)
    double initial_avg_gain = std::accumulate(daily_gains.begin(), 
                                              daily_gains.begin() + period, 
                                              0.0) / period;
                                              
    double initial_avg_loss = std::accumulate(daily_losses.begin(), 
                                              daily_losses.begin() + period, 
                                              0.0) / period;
    
    // Current averages for EMA smoothing
    double avg_gain = initial_avg_gain;
    double avg_loss = initial_avg_loss;
    
    
    // --- Step 3: Calculate the First RSI Value (at index 'period') ---
	
	



    double rs = (avg_loss != 0.0) ? (avg_gain / avg_loss) : 200.0; // Use a high value if loss is zero
    rsi_values[period] = (int)std::round(rscale - (rscale / (1.0 + rs)));
    
    // --- Step 4: Calculate Subsequent RSI Values (Wilder's Smoothing/EMA) ---
    
    for (size_t i = period + 1; i < klines.size(); ++i) {
        // The current gain/loss corresponds to the (i-1) index in daily_gains/losses vector
        double current_gain = daily_gains[i - 1];
        double current_loss = daily_losses[i - 1];
        
        // Wilder's Smoothing Formula (equivalent to EMA with alpha = 1/period)
        avg_gain = ((avg_gain * (period - 1)) + current_gain) / period;
        avg_loss = ((avg_loss * (period - 1)) + current_loss) / period;

        // Calculate RS and RSI
        rs = (avg_loss != 0.0) ? (avg_gain / avg_loss) : 200.0;
        rsi_values[i] = (int)std::round(rscale - (rscale / (1.0 + rs)));
    }
    
    // For the first 'period' entries, the RSI is typically 0 (or simply not defined).
    // The vector is initialized with 0s, which is acceptable for the leading NaNs.
    
    return rsi_values;
}
#include <vector>
#include <map>
#include <tuple>
#include <iostream>

// Assuming your Kline struct is defined
// Assuming your original fill_rsi function is defined and available:
// std::vector<int> fill_rsi(const std::vector<Kline>& klines, int period, float rscale);

// --- Custom Key for Caching ---
// The key is: (Memory Address of klines, period, rscale)
using KlineDataPtr = const std::vector<Kline>*;
using CacheKey = std::tuple<KlineDataPtr, int, float>;
using CacheValue = std::vector<int>;

// --- Cache Structure ---
// CacheMap: Map< (Ptr to Klines, period, rscale), RSI_Results >
static std::map<CacheKey, CacheValue> rsi_cache;

/**
 * @brief Caches and retrieves the result of the fill_rsi calculation.
 * * If the result for the given klines data, period, and rscale is in the cache, 
 * it returns the cached result. Otherwise, it calculates, stores, and returns it.
 * * @param klines The input klines data (used to determine unique data source).
 * @param period The RSI calculation period.
 * @param rscale The custom RSI scale (e.g., 20.0).
 * @return The calculated or cached vector of RSI values.
 */
std::vector<int> fill_rsi_cache(const std::vector<Kline>& klines, int period, float rscale) {
    
    // 1. Create the unique cache key
    // We use the address of the klines vector (klines) as the unique data identifier.
    // This is safe ONLY if the contents of 'klines' don't change between calls
    // OR if 'klines' is always passed by reference (which it is here).
    KlineDataPtr klines_ptr = &klines;
    CacheKey key = std::make_tuple(klines_ptr, period, rscale);

    // 2. Check the cache
    auto it = rsi_cache.find(key);
    
    if (it != rsi_cache.end()) {
        // Cache hit! Return the stored result.
        // std::cout << "DEBUG: Cache Hit for (Period=" << period << ", RScale=" << rscale << ")\n";
        return it->second;
    }

    // 3. Cache miss: Calculate the result
    // std::cout << "DEBUG: Cache Miss. Calculating RSI for (Period=" << period << ", RScale=" << rscale << ")\n";
    
    // IMPORTANT: Call your original RSI calculation function here.
    std::vector<int> result = fill_rsi(klines, period, rscale);

    // 4. Store the result in the cache
    // Note: We use the move constructor for efficiency when storing large vectors.
    rsi_cache[key] = std::move(result);

    // 5. Return the result (from the newly stored cache entry)
    return rsi_cache.at(key);
}
vector<int> vrsi;

float pctDiff(float b,float a) {
  return ((a - b) / b) * 100;
}
struct ProjectionResult {
    bool pattern_found = false;
    double high_extreme = 0.0;
    double low_extreme = 0.0;
    std::string first_hit_label = "NONE"; // "HIGH", "LOW", or "NONE"
    size_t pattern_start_index = 0;
    int outcome_score = -1;             // 1 if High is hit first, 0 if Low is hit first, -1 if neither/undetermined
	double open_price_match=0;
	float percentage_from_open_to_high=0;
	float percentage_from_open_to_low=0;
	float percentage_from_open_to_close=0;
	int first_hit_offset=-1;

};
/**
 * @brief Searches for the most recent RSI pattern in historical data and projects the outcome 
 *        (Max High/Min Low and which was hit first) over the next fsize candles.
 *
 * Deterministic first-hit rules:
 * - Reference is the open of the first projection bar (match_index).
 * - A small epsilon is used to avoid micro crosses.
 * - If both sides are crossed in the same bar:
 *   - Bullish (close >= open): assume open -> low -> high (low touched first).
 *   - Bearish (close <  open): assume open -> high -> low (high touched first).
 *   - Equal close==open: fall back to magnitude.
 */
ProjectionResult analyze_rsi_pattern_and_project_v1(
    const std::vector<int>& vrsi, 
    const std::vector<Kline>& klines, 
    int wsize = 10, 
    int fsize = 6) 
{
    ProjectionResult result;
    const size_t data_size = vrsi.size();

    // Basic safety checks
    if (klines.size() != data_size) {
        std::cerr << "Error: vrsi and klines size mismatch.\n";
        return result;
    }
    if (data_size < static_cast<size_t>(wsize + fsize)) {
        std::cerr << "Error: Not enough data (need at least wsize + fsize bars).\n";
        return result;
    }

    // Target pattern = last wsize RSI values (excluding the very latest to avoid look-ahead)
    const std::vector<int> target_pattern(vrsi.end() - wsize - 1, vrsi.end() - 1);

    // Search backward for historical matches.
    for (int i = static_cast<int>(data_size) - wsize - fsize-1; i >= 14; --i) {
        bool match = true;
        for (int j = 0; j < wsize; ++j) {
            if (vrsi[i + j] != target_pattern[j]) { match = false; break; }
        }
        if (!match) continue;

        // Found a match
        const size_t match_index = static_cast<size_t>(i + wsize); // first bar after the matched pattern
        if (match_index >= klines.size()) continue; // safety

        result.pattern_found = true;
        result.pattern_start_index = i;

        const double open_price  = klines[match_index].open_price;
        const double close_price  = klines[match_index].close_price;
        double max_high = open_price;
        double min_low  = open_price;

        // First-hit tracking
        result.first_hit_label = "NONE";
        result.outcome_score   = -1;
        int first_hit_bar_offset = -1;

        // Reference baseline and tolerance
        const double reference_price = open_price;
        const double eps = 1e-6; // adjust if your tick size is larger

        // Scan the full fsize window
        for (size_t k = match_index; k < match_index + static_cast<size_t>(fsize) && k < klines.size(); ++k) {
            const double o = klines[k].open_price;
            const double h = klines[k].high_price;
            const double l = klines[k].low_price;
            const double c = klines[k].close_price;

            // update extremes
            if (h > max_high) max_high = h;
            if (l  < min_low)  min_low  = l;

            // detect first hit (once)
            if (result.first_hit_label == "NONE") {
                const bool crossed_high = (h > reference_price + eps);
                const bool crossed_low  = (l < reference_price - eps);

                if (crossed_high && !crossed_low) {
                    result.first_hit_label = "HIGH";
                    result.outcome_score = 1;
                    first_hit_bar_offset = static_cast<int>(k - match_index);
                } 
                else if (crossed_low && !crossed_high) {
                    result.first_hit_label = "LOW";
                    result.outcome_score = 0;
                    first_hit_bar_offset = static_cast<int>(k - match_index);
                } 
                else if (crossed_high && crossed_low) {
                    // Both sides crossed in same bar: resolve deterministically.
                    if (c > o + eps) {
                        // Bullish bar: open -> low -> high (low first)
                        result.first_hit_label = "LOW";
                        result.outcome_score = 0;
                    } else if (c < o - eps) {
                        // Bearish bar: open -> high -> low (high first)
                        result.first_hit_label = "HIGH";
                        result.outcome_score = 1;
                    } else {
                        // Doji/flat: fall back to magnitude
                        const double high_diff = h - reference_price;
                        const double low_diff  = reference_price - l;
                        if (high_diff > low_diff) {
                            result.first_hit_label = "HIGH (Indet.)";
                            result.outcome_score = 1;
                        } else {
                            result.first_hit_label = "LOW (Indet.)";
                            result.outcome_score = 0;
                        }
                    }
                    first_hit_bar_offset = static_cast<int>(k - match_index);
                }
            }
        }

        // Fill result with extremes and percentages
        result.high_extreme = max_high;
        result.low_extreme  = min_low;
        result.open_price_match = open_price;
        result.percentage_from_open_to_high = pctDiff(static_cast<float>(open_price), static_cast<float>(max_high));
        result.percentage_from_open_to_low  = pctDiff(static_cast<float>(open_price), static_cast<float>(min_low));
        result.percentage_from_open_to_close  = pctDiff(static_cast<float>(open_price), static_cast<float>(close_price));
        result.first_hit_offset = first_hit_bar_offset;

        return result; // return the first matching historical pattern
    }

    // no pattern found
    return result;
}


ProjectionResult analyze_rsi_pattern_and_project(
    const std::vector<int>& vrsi,
    const std::vector<Kline>& klines,
    int wsize = 10,
    int fsize = 6,
    int tolerance = 0 // max allowed difference per RSI point
)
{
    ProjectionResult result;
    const size_t data_size = vrsi.size();

    if (klines.size() != data_size || data_size < static_cast<size_t>(wsize + fsize + 1)) {
        std::cerr << "Error: invalid data sizes.\n";
        return result;
    }

    // --- Target pattern (the latest known RSI window)
    const std::vector<int> target_pattern(vrsi.end() - wsize - 1, vrsi.end() - 1);

    double best_similarity = 1e9;
    int best_match_index = -1;

    // --- Search backwards for similar RSI sequences
    for (int i = static_cast<int>(data_size) - wsize - fsize - 2; i >= 14; --i) {
        double total_diff = 0.0;
        for (int j = 0; j < wsize; ++j)
            total_diff += std::abs(vrsi[i + j] - target_pattern[j]);

        const double avg_diff = total_diff / wsize;
        if (avg_diff <= tolerance && avg_diff < best_similarity) {
            best_similarity = avg_diff;
            best_match_index = i;
        }
    }

    if (best_match_index == -1)
        return result; // no match found

    // --- Define projection window: fsize bars *after* the pattern
    const size_t future_start = static_cast<size_t>(best_match_index + wsize);
    const size_t future_end   = std::min(future_start + static_cast<size_t>(fsize), klines.size());

    if (future_start >= klines.size() - 1)
        return result; // nothing ahead to project

    result.pattern_found = true;
    result.pattern_start_index = best_match_index;

    // --- Baseline = open of the first *future* bar
    const double open_price = klines[future_start].open_price;
    const double close_price = klines[future_end - 1].close_price;

    double max_high = open_price;
    double min_low  = open_price;

    result.first_hit_label = "NONE";
    result.outcome_score = -1;
    int first_hit_bar_offset = -1;

    const double reference_price = open_price;
    const double eps = 1e-6;

    // --- Scan *future* fsize bars only
    for (size_t k = future_start; k < future_end; ++k) {
        const double o = klines[k].open_price;
        const double h = klines[k].high_price;
        const double l = klines[k].low_price;
        const double c = klines[k].close_price;

        if (h > max_high) max_high = h;
        if (l < min_low) min_low = l;

        if (result.first_hit_label == "NONE") {
            const bool crossed_high = (h > reference_price + eps);
            const bool crossed_low  = (l < reference_price - eps);

            if (crossed_high && !crossed_low) {
                result.first_hit_label = "HIGH";
                result.outcome_score = 1;
                first_hit_bar_offset = static_cast<int>(k - future_start);
            } 
            else if (crossed_low && !crossed_high) {
                result.first_hit_label = "LOW";
                result.outcome_score = 0;
                first_hit_bar_offset = static_cast<int>(k - future_start);
            } 
            else if (crossed_high && crossed_low) {
                // both crossed in same bar
                if (c > o + eps) {
                    result.first_hit_label = "LOW"; // low came first
                    result.outcome_score = 0;
                } else if (c < o - eps) {
                    result.first_hit_label = "HIGH"; // high came first
                    result.outcome_score = 1;
                } else {
                    const double high_diff = h - reference_price;
                    const double low_diff  = reference_price - l;
                    if (high_diff > low_diff) {
                        result.first_hit_label = "HIGH (Indet.)";
                        result.outcome_score = 1;
                    } else {
                        result.first_hit_label = "LOW (Indet.)";
                        result.outcome_score = 0;
                    }
                }
                first_hit_bar_offset = static_cast<int>(k - future_start);
            }
        }
    }

    // --- Fill final results
    result.high_extreme = max_high;
    result.low_extreme  = min_low;
    result.open_price_match = open_price;
    result.percentage_from_open_to_high = pctDiff(static_cast<float>(open_price), static_cast<float>(max_high));
    result.percentage_from_open_to_low  = pctDiff(static_cast<float>(open_price), static_cast<float>(min_low));
    result.percentage_from_open_to_close  = pctDiff(static_cast<float>(open_price), static_cast<float>(close_price));
    result.first_hit_offset = first_hit_bar_offset;

    return result;
}




/**
 * @brief Calculates the percentage difference from Open price to High and Low prices 
 * for a given kline index and returns them in a vector.
 * @param vklines The vector of historical klines.
 * @param index The zero-based index of the kline to analyze.
 * @return std::vector<float>{Open->High %, Open->Low %}
 */
std::vector<float> calculate_open_extremes_vector(
    const std::vector<Kline>& vklines, 
    size_t index) 
{
    // Initialize the result vector with 2 elements set to 0.0f
    std::vector<float> result(3, 0.0f);

    // Safety check
    if (index >= vklines.size()) {
        std::cerr << "Error: Index " << index << " out of bounds.\n";
        // Returns {0.0f, 0.0f} on error
        return result; 
    }
    
    const Kline& kline = vklines[index];
    
    // Cast to float for consistency with pctDiff
    float open_f = (float)kline.open_price;
    float high_f = (float)kline.high_price;
    float low_f = (float)kline.low_price;
    float close_f = (float)kline.close_price;

    // Index 0: Open -> High Percentage (Potential Gain)
    result[0] = pctDiff(open_f, high_f);
    
    // Index 1: Open -> Low Percentage (Potential Loss)
    result[1] = pctDiff(open_f, low_f);

    result[2] = pctDiff(open_f, close_f);

    return result;
}
// #include <vector>
// #include <numeric>
// #include <algorithm>
// #include <iostream>

// #include <vector>
// #include <numeric>
// #include <iostream>
// #include <cmath>

/**
 * @brief Calculates the Exponential Moving Average (EMA) of a data series.
 * @param data The input vector of prices/values (e.g., Close Prices).
 * @param period The EMA period (e.g., 12, 26, 9, or 14 for RSI).
 * @return A vector of floats containing the EMA values.
 */
std::vector<float> calculate_ema(const std::vector<float>& data, int period) {
    size_t data_size = data.size();
    std::vector<float> ema_values(data_size, 0.0);

    if (data_size == 0 || period <= 0 || data_size < (size_t)period) {
        // Not enough data for calculation
        return ema_values;
    }

    // 1. Calculate the Smoothing Factor (Multiplier)
    // The standard formula for the multiplier (alpha) is 2 / (period + 1)
    const float alpha = 2.0 / (period + 1.0);

    // 2. Calculate the Initial EMA Value (SMA of the first 'period' data points)
    // This value is stored at the index corresponding to the end of the first period.
    float sum = 0.0;
    for (int i = 0; i < period; ++i) {
        sum += data[i];
    }
    ema_values[period - 1] = sum / period;
    
    // 3. Calculate Subsequent EMA Values
    // EMA_current = (Price_current * alpha) + (EMA_previous * (1 - alpha))
    for (size_t i = period; i < data_size; ++i) {
        ema_values[i] = (data[i] * alpha) + (ema_values[i - 1] * (1.0 - alpha));
    }

    return ema_values;
}
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

// Assuming 'Kline' struct and 'calculate_ema' are defined elsewhere:
/*
struct Kline {
    double open_price;
    double high_price;
    double low_price;
    double close_price;
};
// Function to calculate EMA, assumed to work with float vectors:
std::vector<float> calculate_ema(const std::vector<float>& data, int period);
*/

/**
 * @brief Calculates the MACD Histogram and scales it to [0, xmax].
 * @param vklines The vector of historical klines.
 * @param xmax The desired maximum value for the scaled histogram (e.g., 100).
 * @param FAST_PERIOD The fast EMA period (default 12).
 * @param SLOW_PERIOD The slow EMA period (default 26).
 * @param SIGNAL_PERIOD The signal EMA period (default 9).
 * @return A vector of ints containing scaled MACD Histogram values.
 */
std::vector<int> calculate_macd_histogram(
    const std::vector<Kline>& vklines,
    int FAST_PERIOD = 12,
    int SLOW_PERIOD = 26,
    int SIGNAL_PERIOD = 9,
    float xmax=80)
{
    const float xmin = 0.0f;  // fixed as requested
    size_t data_size = vklines.size();
    if (data_size < static_cast<size_t>(SLOW_PERIOD + SIGNAL_PERIOD)) {
        std::cerr << "Error: Not enough data for MACD calculation.\n";
        return {};
    }

    // 1. Extract close prices
    std::vector<float> close_prices(data_size);
    for (size_t i = 0; i < data_size; ++i)
        close_prices[i] = static_cast<float>(vklines[i].close_price);

    // 2. Compute EMAs
    std::vector<float> fast_ema = calculate_ema(close_prices, FAST_PERIOD);
    std::vector<float> slow_ema = calculate_ema(close_prices, SLOW_PERIOD);

    // 3. Compute MACD line (difference)
    std::vector<float> macd_line(data_size, 0.0f);
    for (size_t i = SLOW_PERIOD - 1; i < data_size; ++i)
        macd_line[i] = fast_ema[i] - slow_ema[i];

    // 4. Compute signal line
    std::vector<float> signal_line = calculate_ema(macd_line, SIGNAL_PERIOD);

    // 5. Compute MACD histogram (valid range only)
    size_t start_index = SLOW_PERIOD + SIGNAL_PERIOD - 2;
    std::vector<float> valid_hist;
    valid_hist.reserve(data_size - start_index);

    for (size_t i = start_index; i < data_size; ++i)
        valid_hist.push_back(macd_line[i] - signal_line[i]);

    if (valid_hist.empty()) return {};

    // 6. Compute min/max of valid data
    float raw_min = *std::min_element(valid_hist.begin(), valid_hist.end());
    float raw_max = *std::max_element(valid_hist.begin(), valid_hist.end());
    float raw_range = raw_max - raw_min;

    if (raw_range == 0.0f) {
        int mid = static_cast<int>(std::round(xmax / 2.0f));
        return std::vector<int>(data_size, mid);
    }

    // 7. Scale to [0, xmax]
    std::vector<int> scaled_histogram(data_size, 0);
    for (size_t i = 0; i < valid_hist.size(); ++i) {
        float normalized = (valid_hist[i] - raw_min) / raw_range;
        float scaled = xmin + normalized * (xmax - xmin);
        scaled_histogram[start_index + i] = static_cast<int>(std::round(scaled));
    }

    return scaled_histogram;
}
#include <vector>
#include <map>
#include <tuple>
#include <iostream>

// --- Custom Key for Caching ---
// The key is: (Memory Address of klines, FAST_PERIOD, SLOW_PERIOD, SIGNAL_PERIOD)
using KlineDataPtr = const std::vector<Kline>*;
using CacheKeym = std::tuple<KlineDataPtr, int, int, int,float>;
using CacheValuem = std::vector<int>; // MACD Histogram values are doubles

// --- Cache Structure ---
// CacheMap: Map< (Ptr to Klines, F_PERIOD, S_PERIOD, SIG_PERIOD), MACD_Hist_Results >
static std::map<CacheKeym, CacheValuem> macd_hist_cache;

// Assuming your original calculate_macd_histogram function is defined and available:
// std::vector<double> calculate_macd_histogram(const std::vector<Kline>& vklines, int FAST_PERIOD, int SLOW_PERIOD, int SIGNAL_PERIOD);

/**
 * @brief Caches and retrieves the result of the MACD Histogram calculation.
 * If the result for the given klines data and periods is in the cache, it returns the cached result.
 * Otherwise, it calculates, stores, and returns it.
 * * @param vklines The input klines data (used to determine unique data source).
 * @param FAST_PERIOD The fast EMA period (default 12).
 * @param SLOW_PERIOD The slow EMA period (default 26).
 * @param SIGNAL_PERIOD The signal line EMA period (default 9).
 * @return The calculated or cached vector of MACD Histogram values.
 */
std::vector<int> calculate_macd_histogram_cache(
    const std::vector<Kline>& vklines,
    int FAST_PERIOD = 12,
    int SLOW_PERIOD = 26,
    int SIGNAL_PERIOD = 9,float xmax=100) 
{
    
    // 1. Create the unique cache key
    KlineDataPtr klines_ptr = &vklines;
    CacheKeym key = std::make_tuple(klines_ptr, FAST_PERIOD, SLOW_PERIOD, SIGNAL_PERIOD,xmax);

    // 2. Check the cache
    auto it = macd_hist_cache.find(key);
    
    if (it != macd_hist_cache.end()) {
        // Cache hit! Return the stored result.
        // std::cout << "DEBUG: MACD Cache Hit.\n";
        return it->second;
    }

    // 3. Cache miss: Calculate the result
    // std::cout << "DEBUG: MACD Cache Miss. Calculating.\n";
    
    // IMPORTANT: Call your original MACD calculation function here.
    std::vector<int> result = calculate_macd_histogram(vklines, FAST_PERIOD, SLOW_PERIOD, SIGNAL_PERIOD,xmax);

    // 4. Store the result in the cache
    // Use std::move for efficient storage of large vectors.
    macd_hist_cache[key] = std::move(result);

    // 5. Return the result (from the newly stored cache entry)
    return macd_hist_cache.at(key);
}

 
// ----------------------------------------------------------------------
// V1: TrustedClassic (Sorted by Minimal Deviation from 12, 26, 9)
// The most commonly accepted and validated parameters.
// ----------------------------------------------------------------------
std::vector<std::vector<int>> macdTrusted = {
    {12, 26, 9}, {13, 26, 9}, {12, 27, 9}, {12, 26, 10}, {11, 26, 9}, {12, 25, 9}, {12, 26, 8}, {14, 26, 9}, 
    {13, 27, 9}, {12, 28, 9}, {12, 26, 11}, {11, 27, 9}, {10, 26, 9}, {12, 25, 10}, {12, 24, 9}, {12, 26, 7}, 
    {15, 26, 9}, {13, 28, 9}, {12, 29, 9}, {12, 26, 12}, {11, 28, 9}, {10, 27, 9}, {12, 25, 11}, {12, 23, 9}, 
    {9, 26, 9}, {12, 26, 6}, {12, 26, 13}, {13, 29, 9}, {12, 30, 9}, {14, 28, 9}, {15, 27, 9}, {16, 26, 9}, 
    {12, 22, 9}, {10, 28, 9}, {9, 27, 9}, {8, 26, 9}, {12, 26, 14}, {17, 26, 9}, {13, 30, 9}, {12, 31, 9}, 
    {14, 29, 9}, {15, 28, 9}, {16, 27, 9}, {17, 26, 9}, {12, 21, 9}, {10, 29, 9}, {9, 28, 9}, {8, 27, 9}, 
    {7, 26, 9}, {12, 26, 15}, {18, 26, 9}, {14, 30, 9}, {13, 31, 9}, {12, 32, 9}, {15, 29, 9}, {16, 28, 9}, 
    {17, 27, 9}, {18, 26, 9}, {12, 20, 9}, {10, 30, 9}, {9, 29, 9}, {8, 28, 9}, {7, 27, 9}, {6, 26, 9}, 
    {12, 26, 16}, {19, 26, 9}, {15, 31, 9}, {14, 32, 9}, {13, 33, 9}, {12, 33, 9}, {16, 30, 9}, {17, 29, 9}, 
    {18, 28, 9}, {19, 27, 9}, {12, 19, 9}, {10, 31, 9}, {9, 30, 9}, {8, 29, 9}, {7, 28, 9}, {6, 27, 9}, 
    {5, 26, 9}, {12, 26, 17}, {20, 26, 9}, {16, 32, 9}, {15, 33, 9}, {14, 34, 9}, {13, 35, 9}, {12, 34, 9}, 
    {17, 31, 9}, {18, 30, 9}, {19, 29, 9}, {20, 28, 9}, {12, 18, 9}, {10, 32, 9}, {9, 31, 9}, {8, 30, 9},
    {7, 29, 9}, {6, 28, 9}, {5, 27, 9}, {4, 26, 9}, // Total 106 entries
 

// ----------------------------------------------------------------------
// V2: FastAgressive (Shorter Timeframes for sensitivity)
// mfastperiod is lowest, mslowperiod is next, msignal is lowest.
// ----------------------------------------------------------------------
 
    // F: 2 (min), S: 7-20, M: 6-10
    {2, 7, 6}, {2, 8, 6}, {2, 9, 6}, {2, 10, 6}, {2, 11, 6}, {2, 12, 6}, {2, 13, 6}, {2, 14, 6}, {2, 15, 6}, {2, 16, 6}, {2, 17, 6}, {2, 18, 6}, {2, 19, 6}, {2, 20, 6},
    // F: 3, S: 8-20, M: 6-10
    {3, 8, 7}, {3, 9, 7}, {3, 10, 7}, {3, 11, 7}, {3, 12, 7}, {3, 13, 7}, {3, 14, 7}, {3, 15, 7}, {3, 16, 7}, {3, 17, 7}, {3, 18, 7}, {3, 19, 7}, {3, 20, 7},
    // F: 4, S: 9-20, M: 6-10
    {4, 9, 8}, {4, 10, 8}, {4, 11, 8}, {4, 12, 8}, {4, 13, 8}, {4, 14, 8}, {4, 15, 8}, {4, 16, 8}, {4, 17, 8}, {4, 18, 8}, {4, 19, 8}, {4, 20, 8},
    // F: 5, S: 10-20, M: 6-10
    {5, 10, 9}, {5, 11, 9}, {5, 12, 9}, {5, 13, 9}, {5, 14, 9}, {5, 15, 9}, {5, 16, 9}, {5, 17, 9}, {5, 18, 9}, {5, 19, 9}, {5, 20, 9},
    // F: 6, S: 11-20, M: 6-10
    {6, 11, 10}, {6, 12, 10}, {6, 13, 10}, {6, 14, 10}, {6, 15, 10}, {6, 16, 10}, {6, 17, 10}, {6, 18, 10}, {6, 19, 10}, {6, 20, 10},
    // F: 7, S: 12-20, M: 6-10
    {7, 12, 6}, {7, 13, 7}, {7, 14, 8}, {7, 15, 9}, {7, 16, 10}, {7, 17, 6}, {7, 18, 7}, {7, 19, 8}, {7, 20, 9},
    // F: 8, S: 13-20, M: 6-10
    {8, 13, 10}, {8, 14, 6}, {8, 15, 7}, {8, 16, 8}, {8, 17, 9}, {8, 18, 10}, {8, 19, 6}, {8, 20, 7},
    // F: 9, S: 14-20, M: 6-10
    {9, 14, 8}, {9, 15, 9}, {9, 16, 10}, {9, 17, 6}, {9, 18, 7}, {9, 19, 8}, {9, 20, 9},
    // F: 10, S: 15-20, M: 6-10
    {10, 15, 10}, {10, 16, 6}, {10, 17, 7}, {10, 18, 8}, {10, 19, 9}, {10, 20, 10},
    // F: 11, S: 16-20, M: 6-10
    {11, 16, 6}, {11, 17, 7}, {11, 18, 8}, {11, 19, 9}, {11, 20, 10},
    // F: 12, S: 17-20, M: 6-10
    {12, 17, 6}, {12, 18, 7}, {12, 19, 8}, {12, 20, 9}, // Total 105 entries
 

// ----------------------------------------------------------------------
// V3: SlowReliable (Longer Timeframes for smoother trends)
// mfastperiod and mslowperiod are high, msignal is around 9.
// ----------------------------------------------------------------------
 
    // F: 20-25, S: 30-40, M: 8-10
    {20, 40, 9}, {20, 39, 9}, {20, 38, 9}, {20, 37, 9}, {20, 36, 9}, {20, 35, 9}, {20, 34, 9}, {20, 33, 9}, {20, 32, 9}, {20, 31, 9}, {20, 30, 9},
    {21, 40, 9}, {21, 39, 9}, {21, 38, 9}, {21, 37, 9}, {21, 36, 9}, {21, 35, 9}, {21, 34, 9}, {21, 33, 9}, {21, 32, 9}, {21, 31, 9},
    {22, 40, 9}, {22, 39, 9}, {22, 38, 9}, {22, 37, 9}, {22, 36, 9}, {22, 35, 9}, {22, 34, 9}, {22, 33, 9}, {22, 32, 9},
    {23, 40, 9}, {23, 39, 9}, {23, 38, 9}, {23, 37, 9}, {23, 36, 9}, {23, 35, 9}, {23, 34, 9}, {23, 33, 9}, {23, 32, 9}, {23, 31, 9},
    {24, 40, 9}, {24, 39, 9}, {24, 38, 9}, {24, 37, 9}, {24, 36, 9}, {24, 35, 9}, {24, 34, 9}, {24, 33, 9}, {24, 32, 9}, {24, 31, 9},
    {25, 40, 9}, {25, 39, 9}, {25, 38, 9}, {25, 37, 9}, {25, 36, 9}, {25, 35, 9}, {25, 34, 9}, {25, 33, 9}, {25, 32, 9}, {25, 31, 9},
    // Add variations for Signal period (M=8, M=10)
    {18, 30, 8}, {18, 30, 10}, {18, 31, 8}, {18, 31, 10}, {18, 32, 8}, {18, 32, 10}, {18, 33, 8}, {18, 33, 10}, {18, 34, 8}, {18, 34, 10},
    {19, 30, 8}, {19, 30, 10}, {19, 31, 8}, {19, 31, 10}, {19, 32, 8}, {19, 32, 10}, {19, 33, 8}, {19, 33, 10}, {19, 34, 8}, {19, 34, 10},
    {20, 30, 8}, {20, 30, 10}, {20, 31, 8}, {20, 31, 10}, {20, 32, 8}, {20, 32, 10}, {20, 33, 8}, {20, 33, 10}, {20, 34, 8}, {20, 34, 10},
    {21, 35, 11}, {21, 35, 12}, {22, 36, 11}, {22, 36, 12}, {23, 37, 11}, {23, 37, 12}, {24, 38, 11}, {24, 38, 12}, // Total 102 entries
 

// ----------------------------------------------------------------------
// V4: SignalDiversity (Focus on 12/26 but vary msignal widely)
// mfastperiod=12, mslowperiod=26, msignal spans 6 to 30.
// ----------------------------------------------------------------------

    // F=12, S=26 (Classic)
    {12, 26, 6}, {12, 26, 7}, {12, 26, 8}, {12, 26, 9}, {12, 26, 10}, {12, 26, 11}, {12, 26, 12}, {12, 26, 13}, {12, 26, 14}, {12, 26, 15},
    {12, 26, 16}, {12, 26, 17}, {12, 26, 18}, {12, 26, 19}, {12, 26, 20}, {12, 26, 21}, {12, 26, 22}, {12, 26, 23}, {12, 26, 24}, {12, 26, 25},
    {12, 26, 26}, {12, 26, 27}, {12, 26, 28}, {12, 26, 29}, {12, 26, 30},
    
    // F=12, S=27 (Slightly slower)
    {12, 27, 6}, {12, 27, 7}, {12, 27, 8}, {12, 27, 9}, {12, 27, 10}, {12, 27, 11}, {12, 27, 12}, {12, 27, 13}, {12, 27, 14}, {12, 27, 15},
    {12, 27, 16}, {12, 27, 17}, {12, 27, 18}, {12, 27, 19}, {12, 27, 20}, {12, 27, 21}, {12, 27, 22}, {12, 27, 23}, {12, 27, 24}, {12, 27, 25},
    {12, 27, 26}, {12, 27, 27}, {12, 27, 28}, {12, 27, 29}, {12, 27, 30},
    
    // F=11, S=26 (Slightly faster)
    {11, 26, 6}, {11, 26, 7}, {11, 26, 8}, {11, 26, 9}, {11, 26, 10}, {11, 26, 11}, {11, 26, 12}, {11, 26, 13}, {11, 26, 14}, {11, 26, 15},
    {11, 26, 16}, {11, 26, 17}, {11, 26, 18}, {11, 26, 19}, {11, 26, 20}, {11, 26, 21}, {11, 26, 22}, {11, 26, 23}, {11, 26, 24}, {11, 26, 25},
    {11, 26, 26}, {11, 26, 27}, {11, 26, 28}, {11, 26, 29}, {11, 26, 30},
    
    // F=13, S=26 (Alternative Fast)
    {13, 26, 6}, {13, 26, 7}, {13, 26, 8}, {13, 26, 9}, {13, 26, 10}, {13, 26, 11}, {13, 26, 12}, {13, 26, 13}, // Total 108 entries
 

// ----------------------------------------------------------------------
// V5: BroadSweep (Iterative sweep of all valid parameters)
// Exploratory—tests the full search space defined by your loops.
// ----------------------------------------------------------------------
 
    // F=2, S=7...40, M=6
    {2, 7, 6}, {2, 8, 6}, {2, 9, 6}, {2, 10, 6}, {2, 11, 6}, {2, 12, 6}, {2, 13, 6}, {2, 14, 6}, {2, 15, 6}, {2, 16, 6}, {2, 17, 6}, {2, 18, 6}, {2, 19, 6}, {2, 20, 6}, 
    {2, 21, 6}, {2, 22, 6}, {2, 23, 6}, {2, 24, 6}, {2, 25, 6}, {2, 26, 6}, {2, 27, 6}, {2, 28, 6}, {2, 29, 6}, {2, 30, 6}, {2, 31, 6}, {2, 32, 6}, {2, 33, 6}, 
    {2, 34, 6}, {2, 35, 6}, {2, 36, 6}, {2, 37, 6}, {2, 38, 6}, {2, 39, 6}, {2, 40, 6}, 
    // F=3, S=8...40, M=6
    {3, 8, 7}, {3, 9, 7}, {3, 10, 7}, {3, 11, 7}, {3, 12, 7}, {3, 13, 7}, {3, 14, 7}, {3, 15, 7}, {3, 16, 7}, {3, 17, 7}, {3, 18, 7}, {3, 19, 7}, {3, 20, 7},
    {3, 21, 7}, {3, 22, 7}, {3, 23, 7}, {3, 24, 7}, {3, 25, 7}, {3, 26, 7}, {3, 27, 7}, {3, 28, 7}, {3, 29, 7}, {3, 30, 7}, {3, 31, 7}, {3, 32, 7}, {3, 33, 7},
    {3, 34, 7}, {3, 35, 7}, {3, 36, 7}, {3, 37, 7}, {3, 38, 7}, {3, 39, 7}, {3, 40, 7},
    // F=4, S=9...40, M=8
    {4, 9, 8}, {4, 10, 8}, {4, 11, 8}, {4, 12, 8}, {4, 13, 8}, {4, 14, 8}, {4, 15, 8}, {4, 16, 8}, {4, 17, 8}, {4, 18, 8}, {4, 19, 8}, {4, 20, 8},
    {4, 21, 8}, {4, 22, 8}, {4, 23, 8}, {4, 24, 8}, {4, 25, 8}, {4, 26, 8}, {4, 27, 8}, {4, 28, 8}, {4, 29, 8}, {4, 30, 8}, {4, 31, 8}, {4, 32, 8}, 
    {4, 33, 8}, {4, 34, 8}, {4, 35, 8}, {4, 36, 8}, {4, 37, 8}, {4, 38, 8}, {4, 39, 8}, {4, 40, 8},
    // F=5, S=10...13, M=9
    {5, 10, 9}, {5, 11, 9}, {5, 12, 9}, {5, 13, 9}, {5, 14, 9}, {5, 15, 9}, {5, 16, 9}, {5, 17, 9}, {5, 18, 9}, {5, 19, 9}, {5, 20, 9}, {5, 21, 9}, // Total 110 entries
};
std::vector<int> rsi_periods = {
    14,  // clássico, equilíbrio
    21,  // mais suave, robusto
    10,  // curto mas estável
    7,   // mais reativo
    5,   // muito sensível
    2,   // extremamente volátil
    9,
    12,
    18,
    20,
    25,
    28,
    30,
    35,
    40,
    45,
    50,
    55,
    60,
    70};
int main() {
    std::cout << "Starting Crypto Data Retrieval (Functional C++)\n";

    try {
        // --- 1. WhiteBIT Equity Check (Skipped for brevity, assume still working) ---
        /*
        double equity = get_equity();
        std::cout << "\n----------------------------------------\n";
        std::cout << "✅ WhiteBIT Equity: " << std::fixed << std::setprecision(8) << equity << "\n";
        std::cout << "----------------------------------------\n";
        */
        
        // --- 2. Binance Klines Data Fetch ---
        std::string symbol = "BTCUSDT";
        symbol = "ETHUSDT";
        // symbol = "XRPUSDT";
        // symbol = "SOLUSDT";
        // symbol = "SUIUSDT";
        // symbol = "DOGEUSDT";
        // symbol = "TIAUSDT";
        // symbol = "AVAXUSDT";
        std::string interval = "1d";
        int limit = 999; 

        std::cout << "\nFetching Binance Klines (" << symbol << ", " << interval << ", limit: " << limit << ")...";
        std::string klines_json = get_binance_klines(symbol, interval, limit);
        std::cout << " Done.\n";
 
        // --- 3. Parse and Store Data ---
        std::vector<Kline> vklines = parse_klines(klines_json);
		// vklines.pop_back();
		// vklines.pop_back();
		// vklines.pop_back();
		// vklines.pop_back();
		// vklines.pop_back();
		// vklines.pop_back();
		// vklines.pop_back();

        std::cout << "\n✅ Successfully parsed " << vklines.size() << " Klines into a vector struct.\n";
        std::cout << "----------------------------------------\n";



// std::cout << "--- DEBUG: Close Prices for the RSI Window ---\n";
// size_t debug_start = vklines.size() - wsize - 1; 
// size_t debug_end = vklines.size();

// for (size_t i = debug_start; i < debug_end; ++i) {
//     std::cout << "Index " << i << ": " << std::fixed << std::setprecision(5) << vklines[i].close_price << "\n";
// }
// std::cout << "----------------------------------------------\n";
//         cout<<vrsi.back()<<" " <<vrsi.size()<<endl;
//         cout<<vrsi.back()<<" " <<vrsi.size()<<endl;


        // --- 4. Print the first few entries ---
        // std::cout << "First " << std::min(5, (int)vklines.size()) << " Klines:\n";
        // for (size_t i = 0; i < std::min((size_t)5, vklines.size()); ++i) {
        //     vklines[i].print();
        // }
        // std::cout << "----------------------------------------\n";

vint vmacd;

		// ... inside main function ...

// --- 5. Analyze RSI Pattern and Project Price ---
// int wsize = 10;
// int fsize = 6;
bool flagbreak = 0;
for (int wsize = 8; wsize > 3; wsize--) {
	for (float rscale = 80; rscale >= 20; rscale -= 5) {
		// for(int i=0;i<6;i++){ //more trust
		for(int i=0;i<rsi_periods.size();i++){
		// for(int i=26;i>2;i--){
		// lop(i, 3, 17) {
			vrsi = fill_rsi_cache(vklines, rsi_periods[i], rscale);
			// Add this loop right after you call fill_rsi

			// int wsize = 4;
			int fsize = 1 * 1;

			// ... (call to analyze_rsi_pattern_and_project) ...
			// std::cout << "\nAnalyzing RSI pattern (w=" << wsize << ", f=" << fsize << "):\n";

			ProjectionResult projection = analyze_rsi_pattern_and_project(vrsi, vklines, wsize, fsize);

			if (projection.pattern_found) {
				if(abs(projection.percentage_from_open_to_high)<0.5 && abs(projection.percentage_from_open_to_low)<0.5)continue;
				bool isg1=abs(projection.percentage_from_open_to_high)>0.8 && abs(projection.percentage_from_open_to_low)<0.8;
				bool isg2=abs(projection.percentage_from_open_to_high)<0.8 && abs(projection.percentage_from_open_to_low)>0.8;
				// if(isg1 || isg2){}else continue;
				
				// if(projection.pattern_start_index==545)continue;
				// if(projection.pattern_start_index==552)continue;
				// if(projection.pattern_start_index==556)continue;
				// lop(macdi, 0, 50) {
				lop(macdi, 0, macdTrusted.size()) {
					for(int mcdinterval=100;mcdinterval>=20;mcdinterval-=5){
					// lop(mslowperiod, 6, 40) {
						// if (mslowperiod - mfastperiod < 5) continue;
						// lop(msignal, 6, 30) {
							// vmacd = calculate_macd_histogram(vklines, macdTrusted[macdi][0], macdTrusted[macdi][1], macdTrusted[macdi][2],mcdinterval);
							vmacd = calculate_macd_histogram_cache(vklines, macdTrusted[macdi][0], macdTrusted[macdi][1], macdTrusted[macdi][2],mcdinterval);
							// vmacd = calculate_macd_histogram_cache(vklines, mfastperiod, mslowperiod, msignal);

							vint vmacdactual = vint(vmacd.end() - wsize - 1, vmacd.end() - 1);
							// cotm(vmacdactual);
							// flagbreak = 1;
							vint vmac = vint(vmacd.begin() + projection.pattern_start_index,
											 vmacd.begin() + projection.pattern_start_index + wsize);
							if(accumulate(vmac.begin(),vmac.end(),0)==0)continue;
							// cotm(vmac)
							if (vmacdactual == vmac) {
								flagbreak = 1;
								// cotm(vmacd, vmacd.size()) cotm(vmac) cotm(vmacdactual)
									cotm(mcdinterval,macdi, macdTrusted[macdi]) 
									break;
									// cotm(mfastperiod, mslowperiod, msignal) break;
							}
				// 		}
						if (flagbreak) break;
					}
				// 	if (flagbreak) break;
				}
				// flagbreak=0;
				if (!flagbreak) continue;
				vint vrsiactual = vint(vrsi.end() - wsize - 1, vrsi.end() - 1);
				cotm(vrsiactual, rscale, wsize, i);

				cotm(projection.first_hit_offset, projection.pattern_start_index)

						std::cout
					<< "----------------------------------------\n";
				std::cout << "✅ Pattern Found!\n";
				std::cout << "Pattern Start Index: " << projection.pattern_start_index << "\n";
				std::cout << "OUTCOME SCORE:       " << projection.outcome_score << "\n";
				std::cout << "First Extreme Hit:   " << projection.first_hit_label << "\n";

				std::cout << "\n--- Projection Details (Entry at Open Price) ---\n";
				std::cout << "Entry Open Price:    " << std::fixed << std::setprecision(5)
						  << projection.open_price_match << "\n";

				// Print the new percentage fields
				std::cout << "Max Gain (%):        +" << std::fixed << std::setprecision(4)
						  << projection.percentage_from_open_to_high << "%\n";
				std::cout << "Max Loss (%):        " << std::fixed << std::setprecision(4)
						  << projection.percentage_from_open_to_low << "%\n";
				std::cout << "Close    (%):        " << std::fixed << std::setprecision(4)
						  << projection.percentage_from_open_to_close << "%\n";

				std::cout << "Max High Price:      " << std::fixed << std::setprecision(5) << projection.high_extreme
						  << "\n";
				std::cout << "Min Low Price:       " << std::fixed << std::setprecision(5) << projection.low_extreme
						  << "\n";

				    try {


						Kline &toseek=vklines[projection.pattern_start_index+wsize];
						

        // bool result = highFirst(symbol, vklines,projection.pattern_start_index+wsize,projection.pattern_start_index+wsize+fsize);
        bool result = highFirst(symbol, toseek);
        std::cout << (result ? "High primeiro" : "Low primeiro") << std::endl;
		cotm(toseek.high_price,toseek.low_price);
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
    }

				break;
			} else {
				// ...
			}
			if (flagbreak) break;
		}
		if (flagbreak) break;
	}
	if (flagbreak) break;
}

	vfloat highlowper = calculate_open_extremes_vector(vklines, vklines.size() - 1);
	cotm(highlowper) 
	
	try {
		Kline &klineactual=vklines.back();
		bool result = highFirst(symbol, klineactual);
		std::cout << (result ? "High primeiro" : "Low primeiro") << std::endl;
		cotm(klineactual.high_price,klineactual.low_price,klineactual.close_price)
	} catch (const std::exception& e) {
		std::cerr << "Erro: " << e.what() << std::endl;
	}

	} catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR:\n";
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}