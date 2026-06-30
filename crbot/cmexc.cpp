
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

// --- Helper: cURL Write Callback ---
size_t HttpCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// --- Futures Signature Engine ---
// MEXC Futures uses: ApiKey + RequestTime + RequestBody
std::string getFuturesSignature(const std::string& apiKey, const std::string& secret, 
                                const std::string& timestamp, const std::string& requestBody) {
    std::string message = apiKey + timestamp + requestBody;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    HMAC(EVP_sha256(), secret.c_str(), secret.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(),
         hash, &length);

    std::stringstream ss;
    for (unsigned int i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// --- Futures HTTP Engine ---
std::string sendFuturesRequest(const std::string& apiKey, const std::string& apiSecret, 
                               const std::string& method, const std::string& endpoint, 
                               const std::string& payload = "") {
    CURL* curl = curl_easy_init();
    if (!curl) return "{\"error\":\"CURL_INIT_FAILED\"}";

    // Generate millisecond timestamp
    long long msTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string timestamp = std::to_string(msTimestamp);

    // Generate HMAC signature
    std::string signature = getFuturesSignature(apiKey, apiSecret, timestamp, payload);
    std::string url = "https://contract.mexc.com" + endpoint;
    
    // For GET requests, append payload to URL if it exists (query string)
    if (method == "GET" && !payload.empty()) {
        url += "?" + payload;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("ApiKey: " + apiKey).c_str());
    headers = curl_slist_append(headers, ("Request-Time: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("Signature: " + signature).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    
    if (method == "POST" && !payload.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

// --- NEW FUNCTION: Get All Opened Futures Positions ---
std::string getOpenedFuturesPositions() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        return "{\"error\":\"Missing API credentials in environment variables.\"}";
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);

    // Endpoint for checking open futures positions
    std::string endpoint = "/api/v1/private/position/open_positions";
    
    std::cout << "Fetching all opened futures positions..." << std::endl;
    return sendFuturesRequest(apiKey, apiSecret, "GET", endpoint);
}

// --- Orchestrator: Bracket Futures Position (JSON Adapted) ---
void openBracketFuturesPosition(const std::string& symbol, const std::string& side, 
                                double qty, double entryPrice, double stopLoss, double takeProfit) {
    
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Execution Blocked: Missing API credentials." << std::endl;
        return;
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);
    
    // MEXC Futures Side Ints: 1 = Open Long, 2 = Close Short, 3 = Open Short, 4 = Close Long
    int entrySideInt = (side == "BUY") ? 1 : 3;
    int exitSideInt  = (side == "BUY") ? 4 : 2;

    // --- STEP 1: Post Core Margin Position Entry ---
    // Note: MEXC Futures uses JSON payloads, not query parameters
    std::stringstream pOrder;
    pOrder << "{"
           << "\"symbol\":\"" << symbol << "\","
           << "\"price\":" << entryPrice << ","
        //    << "\"price\":" << std::fixed << std::setprecision(2) << entryPrice << ","
           << "\"vol\":" << qty << ","
        //    << "\"vol\":" << std::fixed << std::setprecision(4) << qty << ","
           << "\"side\":" << entrySideInt << ","
           << "\"type\":1,"       // 1 = Limit Order
           << "\"openType\":1,"   // 1 = Isolated Margin
           << "\"leverage\":5"    // Specify Default Leverage
           << "}";

    std::cout << "Sending Futures Entry Order..." << std::endl;
    std::string entryRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/order/submit", pOrder.str());
    std::cout << "Entry Response: " << entryRes << "\n\n";

    // --- STEP 2: Deploy Stop Loss Guard ---
    std::stringstream slOrder;
    slOrder << "{"
            << "\"symbol\":\"" << symbol << "\","
            << "\"triggerPrice\":" << stopLoss << ","
            << "\"price\":" <<  (side == "BUY" ? stopLoss * 0.995 : stopLoss * 1.005) << ","
            << "\"vol\":"  << qty << ","
            << "\"side\":" << exitSideInt << ","
            << "\"type\":1,"       // 1 = Limit trigger
            << "\"executeCycle\":1"// GTC
            << "}";
    // slOrder << "{"
    //         << "\"symbol\":\"" << symbol << "\","
    //         << "\"triggerPrice\":" << std::fixed << std::setprecision(2) << stopLoss << ","
    //         << "\"price\":" << std::fixed << std::setprecision(2) << (side == "BUY" ? stopLoss * 0.995 : stopLoss * 1.005) << ","
    //         << "\"vol\":" << std::fixed << std::setprecision(4) << qty << ","
    //         << "\"side\":" << exitSideInt << ","
    //         << "\"type\":1,"       // 1 = Limit trigger
    //         << "\"executeCycle\":1"// GTC
    //         << "}";

    std::cout << "Deploying Stop Loss Guard..." << std::endl;
    std::string slRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/planorder/place", slOrder.str());
    std::cout << "Stop Loss Response: " << slRes << "\n\n";

    // --- STEP 3: Deploy Take Profit Order ---
    std::stringstream tpOrder;
    tpOrder << "{"
            << "\"symbol\":\"" << symbol << "\","
            << "\"triggerPrice\":" << takeProfit << ","
            << "\"price\":"  << takeProfit << ","
            << "\"vol\":" << qty << ","
            << "\"side\":" << exitSideInt << ","
            << "\"type\":1,"
            << "\"executeCycle\":1"
            << "}";
    // tpOrder << "{"
    //         << "\"symbol\":\"" << symbol << "\","
    //         << "\"triggerPrice\":" << std::fixed << std::setprecision(2) << takeProfit << ","
    //         << "\"price\":" << std::fixed << std::setprecision(2) << takeProfit << ","
    //         << "\"vol\":" << std::fixed << std::setprecision(4) << qty << ","
    //         << "\"side\":" << exitSideInt << ","
    //         << "\"type\":1,"
    //         << "\"executeCycle\":1"
    //         << "}";

    std::cout << "Deploying Take Profit Order..." << std::endl;
    std::string tpRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/planorder/place", tpOrder.str());
    std::cout << "Take Profit Response: " << tpRes << "\n";
}
// --- Light Extraction Helper ---
// Searches for the USDT block and extracts the designated balance field
double extractUsdtBalance(const std::string& jsonResponse, const std::string& balanceField = "availableBalance") {
    // 1. Locate the USDT asset block
    size_t usdtPos = jsonResponse.find("\"currency\":\"USDT\"");
    if (usdtPos == std::string::npos) {
        return 0.0; // USDT block not found
    }

    // 2. Look for the target balance field within or near this block
    size_t fieldPos = jsonResponse.find("\"" + balanceField + "\":", usdtPos);
    if (fieldPos == std::string::npos) {
        return 0.0;
    }

    // 3. Jump past the field name and the colon (e.g., "availableBalance":)
    size_t valueStart = fieldPos + balanceField.length() + 3;
    
    // 4. Find where the number ends (usually a comma or closing bracket)
    size_t valueEnd = jsonResponse.find_first_of(",}", valueStart);
    if (valueEnd == std::string::npos) {
        return 0.0;
    }

    std::string valueStr = jsonResponse.substr(valueStart, valueEnd - valueStart);
    try {
        return std::stod(valueStr);
    } catch (...) {
        return 0.0; 
    }
}

// --- Fetch Only USDT Available Balance ---
double getUsdtFuturesBalance() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Missing API credentials." << std::endl;
        return 0.0;
    }

    std::string rawJson = sendFuturesRequest(envKey, envSecret, "GET", "/api/v1/private/account/assets");
    
    // Extract "availableBalance" specifically for the USDT object
    return extractUsdtBalance(rawJson, "availableBalance");
}

// --- Wrapper to Print Metrics ---
int print_account() {
    std::cout << "Fetching Futures Account Balances..." << std::endl;
    double accountInfo = getUsdtFuturesBalance();
    
    std::cout << "Account Metrics Details:\n" << accountInfo << std::endl;
	
    // 1. Fetching all currently open futures positions
    std::string openPositions = getOpenedFuturesPositions();
    std::cout << "Open Positions Data:\n" << openPositions << "\n" << std::endl;

    // 2. Example: Opening a 5x Futures Long on BTC_USDT (Notice MEXC futures uses the underscore)
    // openBracketFuturesPosition("BTC_USDT", "BUY", 0.025, 64200.00, 63100.00, 67500.00);
    return 0;
}

