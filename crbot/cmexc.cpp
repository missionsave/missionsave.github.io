#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

// HMAC-SHA256 signature generator
std::string generateSignature(const std::string& secret, const std::string& message) {
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

size_t HttpCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// Low-level HTTP Client wrapper
std::string postMexcRequest(const std::string& apiKey, const std::string& apiSecret, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) return "{\"error\":\"CURL_INIT_FAILED\"}";

    // Combine payload and the signature directly into the target URL
    std::string url = "https://api.mexc.com/api/v3/order?" + payload + "&signature=" + generateSignature(apiSecret, payload);
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-MEXC-APIKEY: " + apiKey).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Crucial: Use CUSTOMREQUEST "POST" instead of CURLOPT_POST.
    // This tells curl to send a POST action header without appending hidden form body headers.
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

// Orchestrator to construct the entire Bracket position order loop
void openBracketMarginPosition(const std::string& symbol, const std::string& side, 
                               double qty, double entryPrice, double stopLoss, double takeProfit) {
    
    // Extracting credentials from Environment Variables safely
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Execution Blocked: Missing API credentials in environment variables." << std::endl;
        return;
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);

    long long msTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Determine exit sides
    std::string exitSide = (side == "BUY") ? "SELL" : "BUY";
    std::string marginFlag = (side == "BUY") ? "isMarginBuy=true" : "isMarginSell=true";
    std::string exitMarginFlag = (side == "BUY") ? "isMarginSell=true" : "isMarginBuy=true";

    // --- STEP 1: Post Core Margin Position Entry (LIMIT = 0% Fee Target) ---
    std::stringstream pOrder;
    pOrder << "symbol=" << symbol << "&side=" << side << "&type=LIMIT"
           << "&quantity=" << std::fixed << std::setprecision(4) << qty
           << "&price=" << std::fixed << std::setprecision(2) << entryPrice
           << "&timeInForce=GTC&" << marginFlag << "&timestamp=" << msTimestamp;

    std::cout << "Sending Entry Order..." << std::endl;
    std::string entryRes = postMexcRequest(apiKey, apiSecret, pOrder.str());
    std::cout << "Entry Response: " << entryRes << "\n\n";

    // --- STEP 2: Deploy Stop Loss Order ---
    msTimestamp++; // Incrementing slightly to maintain signature sequence windows
    std::stringstream slOrder;
    slOrder << "symbol=" << symbol << "&side=" << exitSide << "&type=STOP_LOSS_LIMIT"
            << "&quantity=" << std::fixed << std::setprecision(4) << qty
            << "&stopPrice=" << std::fixed << std::setprecision(2) << stopLoss
            << "&price=" << std::fixed << std::setprecision(2) << (stopLoss * 0.995) // Adding subtle price slip room to guarantee fill
            << "&timeInForce=GTC&" << exitMarginFlag << "&timestamp=" << msTimestamp;

    std::cout << "Deploying Stop Loss Guard..." << std::endl;
    std::string slRes = postMexcRequest(apiKey, apiSecret, slOrder.str());
    std::cout << "Stop Loss Response: " << slRes << "\n\n";

    // --- STEP 3: Deploy Take Profit Target Order ---
    msTimestamp++;
    std::stringstream tpOrder;
    tpOrder << "symbol=" << symbol << "&side=" << exitSide << "&type=TAKE_PROFIT_LIMIT"
            << "&quantity=" << std::fixed << std::setprecision(4) << qty
            << "&stopPrice=" << std::fixed << std::setprecision(2) << takeProfit
            << "&price=" << std::fixed << std::setprecision(2) << takeProfit
            << "&timeInForce=GTC&" << exitMarginFlag << "&timestamp=" << msTimestamp;

    std::cout << "Deploying Take Profit Order..." << std::endl;
    std::string tpRes = postMexcRequest(apiKey, apiSecret, tpOrder.str());
    std::cout << "Take Profit Response: " << tpRes << "\n";
}

int test() {
    // Example: Opening a 5x Spot Margin Long on BTC
    // Parameters: Symbol, Side, Quantity, Entry Price, Stop Loss, Take Profit
    openBracketMarginPosition("BTCUSDT", "BUY", 0.025, 64200.00, 63100.00, 67500.00);
    return 0;
}



#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

// Signature Engine Helper
std::string getMexcSignature(const std::string& secret, const std::string& message) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    HMAC(EVP_sha256(), secret.c_str(), secret.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), hash, &length);
    std::stringstream ss;
    for (unsigned int i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

size_t ReadBalanceCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// Function to fetch free wallet collateral metrics
std::string getFreeMarginBalance() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) return "{\"error\":\"Missing API Keys in Environment\"}";

    CURL* curl = curl_easy_init();
    if (!curl) return "{\"error\":\"CURL_INIT_FAILED\"}";

    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Construct the payload query segment
    std::stringstream payload;
    payload << "recvWindow=5000&timestamp=" << timestamp;

    std::string signature = getMexcSignature(envSecret, payload.str());
    std::string fullUrl = "https://api.mexc.com/api/v3/account?" + payload.str() + "&signature=" + signature;

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-MEXC-APIKEY: " + std::string(envKey)).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET"); // Strictly a GET request structure
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReadBalanceCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

int print_account() {
    std::string accountInfo = getFreeMarginBalance();
    std::cout << "Account Metrics Details:\n" << accountInfo << std::endl;
    return 0;
}