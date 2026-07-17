#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <curl/curl.h>
// #include <nlohmann/json.hpp>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <cJSON.h>
using namespace std;

// --- Helper: cURL Write Callback ---
size_t HttpCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// --- Futures Signature Engine ---
// MEXC Futures uses: ApiKey + RequestTime + RequestBody
// (For POST, RequestBody is the raw JSON string, no sorting needed.
//  For GET/DELETE with params, MEXC requires params sorted in dictionary
//  order and joined with '&' BEFORE signing — this helper assumes the
//  caller already built that string correctly; see note in sendFuturesRequest.)
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
    // NOTE: for GET requests with query params, MEXC requires the params to be
    // sorted alphabetically and joined with '&' before they're used here. This
    // function signs whatever raw string is in `payload`, so callers issuing
    // GET requests with parameters must pre-sort them before calling.
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
	// Limita a fase de ligação a um máximo de 10 segundos
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    // Limita a transferência total a um máximo de 30 segundos
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
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
bool getOpenPositionSymbolsfetched=0;
std::vector<std::string> getOpenPositionSymbols(){

	static std::vector<std::string> symbols;
	if(symbols.size()>0)return symbols;
	if(getOpenPositionSymbolsfetched==1)return symbols;
	getOpenPositionSymbolsfetched=1;
	
	const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

	// Endpoint MEXC Futures
	std::string endpoint = "/api/v1/private/position/open_positions";

	// GET request sem payload
	std::string response = sendFuturesRequest(envKey, envSecret, "GET", endpoint, "");
	// cout<<response<<"\n";


	size_t pos = 0;

	while (true) {
	// Encontrar "symbol":"XYZ_USDT"
	size_t s = response.find("\"symbol\":\"", pos);
	if (s == std::string::npos) break;

	s += 10; // avança depois de "symbol":" 
	size_t e = response.find("\"", s);
	if (e == std::string::npos) break;

	std::string sym = response.substr(s, e - s);

	// Encontrar holdVol
	size_t hv = response.find("\"holdVol\":", e);
	if (hv == std::string::npos) break;

	hv += 10; // depois de "holdVol":
	size_t hvEnd = response.find(",", hv);
	if (hvEnd == std::string::npos) break;

	double holdVol = std::stod(response.substr(hv, hvEnd - hv));

	if (holdVol > 0.0)
	symbols.push_back(sym);

	pos = e + 1;
	}

	return symbols;
}
bool symbol_opened(string symbol){
	vector<std::string> gops=getOpenPositionSymbols();
	for(int i=0;i<gops.size();i++){
		if(gops[i]==symbol)return 1;
	}
	return 0;
}
bool max_symbols_reached(){
	vector<std::string> gops=getOpenPositionSymbols();
	if(gops.size()>=4)return 1;
	return 0;
}

// --- Get All Opened Futures Positions ---
// GET /api/v1/private/position/open_positions
std::string getOpenedFuturesPositions() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        return "{\"error\":\"Missing API credentials in environment variables.\"}";
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);

    std::string endpoint = "/api/v1/private/position/open_positions";

    std::cout << "Fetching all opened futures positions..." << std::endl;
    return sendFuturesRequest(apiKey, apiSecret, "GET", endpoint);
}
int formatVol(double qty, int stepsize)
{
    // 1) Round qty to the desired number of decimals
    double factor = std::pow(10.0, stepsize);
    double rounded = std::floor(qty * factor + 1e-12) / factor;

    // 2) Convert to integer by removing the decimal point
    int result = static_cast<int>(rounded * factor);

    return result;
}
struct digits_contractsize {
    double priceScale = 0;
    double contractSize = 0;
};

double extractNumber(const std::string& src, const std::string& key) {
    std::string tag = "\"" + key + "\":";
    size_t start = src.find(tag);
    if (start == std::string::npos) return 0.0;

    start += tag.size();
    size_t end = src.find_first_of(",}", start);
    if (end == std::string::npos) return 0.0;

    std::string num = src.substr(start, end - start);
    return std::stod(num);
}

digits_contractsize getContractSize(const std::string& apiKey,
                                    const std::string& apiSecret,
                                    const std::string& mexcSymbol)
{
    std::string endpoint = "/api/v1/contract/detail?symbol=" + mexcSymbol;

    std::string res = sendFuturesRequest(
        apiKey,
        apiSecret,
        "GET",
        endpoint,
        ""
    );

    // Verificar se existe "data"
    size_t dpos = res.find("\"data\":");
    if (dpos == std::string::npos)
        throw std::runtime_error("Invalid MEXC response: no data");

    // Extrair priceScale e contractSize
    double priceScale = extractNumber(res, "priceScale");
    double contractSize = extractNumber(res, "contractSize");

    if (contractSize == 0.0)
        throw std::runtime_error("Invalid MEXC response: no contractSize");

    return { priceScale, contractSize };
}
// --- Close the Running Position with the Highest Profit (Highest Unrealized PnL) ---
void closeMostProfitablePosition() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Execution Blocked: Missing API credentials." << std::endl;
        return;
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);

    // 1. Fetch all open positions
    std::string response = sendFuturesRequest(apiKey, apiSecret, "GET", "/api/v1/private/position/open_positions", "");

    cJSON* json = cJSON_Parse(response.c_str());
    if (!json) {
        std::cerr << "Error: Failed to parse open positions response." << std::endl;
        return;
    }

    cJSON* data = cJSON_GetObjectItem(json, "data");
    if (!data || !cJSON_IsArray(data)) {
        cJSON_Delete(json);
        std::cout << "No open positions array found." << std::endl;
        return;
    }

    int arraySize = cJSON_GetArraySize(data);
    double highestPnL = -99999999.0; // Seed with a very low value to catch even small or negative values if needed
    std::string bestSymbol = "";
    double bestVol = 0.0;
    double fallbackPrice = 0.0; 
    int bestSide = 0;        // positionType: 1 = Long, 2 = Short
    int bestOpenType = 1;    // openType: 1 = Isolated, 2 = Cross

    // 2. Loop through up to 4 positions to identify the one with the most gain
    for (int i = 0; i < arraySize; ++i) {
        cJSON* item = cJSON_GetArrayItem(data, i);
        if (!item) continue;

        cJSON* holdVolObj = cJSON_GetObjectItem(item, "holdVol");
        double holdVol = holdVolObj ? holdVolObj->valuedouble : 0.0;

        if (holdVol > 0.0) {
            cJSON* pnlObj = cJSON_GetObjectItem(item, "unrealisedPnL");
            double unrealisedPnL = pnlObj ? pnlObj->valuedouble : 0.0;

            // Check if this position has more profit than our recorded highest
            if (bestSymbol.empty() || unrealisedPnL > highestPnL) {
                highestPnL = unrealisedPnL;
                
                cJSON* symbolObj = cJSON_GetObjectItem(item, "symbol");
                bestSymbol = symbolObj ? symbolObj->valuestring : "";
                
                bestVol = holdVol;

                cJSON* avgPriceObj = cJSON_GetObjectItem(item, "holdAvgPrice");
                fallbackPrice = avgPriceObj ? avgPriceObj->valuedouble : 0.0;

                cJSON* posTypeObj = cJSON_GetObjectItem(item, "positionType");
                bestSide = posTypeObj ? posTypeObj->valueint : 1;

                cJSON* openTypeObj = cJSON_GetObjectItem(item, "openType");
                bestOpenType = openTypeObj ? openTypeObj->valueint : 1;
            }
        }
    }
    cJSON_Delete(json);

    if (bestSymbol.empty()) {
        std::cout << "No open positions found to close.\n";
        return;
    }

    // 3. Query the real-time ticker price for the target profitable symbol
    double targetPrice = 0.0;
    std::string tickerRes = sendFuturesRequest(apiKey, apiSecret, "GET", "/api/v1/contract/ticker", "symbol=" + bestSymbol);
    
    cJSON* tickerJson = cJSON_Parse(tickerRes.c_str());
    if (tickerJson) {
        cJSON* tData = cJSON_GetObjectItem(tickerJson, "data");
        if (tData) {
            cJSON* lastPriceObj = cJSON_GetObjectItem(tData, "lastPrice");
            if (lastPriceObj) {
                targetPrice = lastPriceObj->valuedouble;
            }
        }
        cJSON_Delete(tickerJson);
    }

    if (targetPrice <= 0.0) {
        targetPrice = fallbackPrice;
    }

    int closeSideInt = (bestSide == 1) ? 4 : 2;

    std::cout << "--- Closing Most Profitable Position via Limit Order ---" << std::endl;
    std::cout << "Symbol:        " << bestSymbol << std::endl;
    std::cout << "Current PnL:   " << highestPnL << " USDT" << std::endl;
    std::cout << "Target Price:  " << targetPrice << std::endl;
    std::cout << "Volume (Qty):  " << bestVol << std::endl;
    std::cout << "Exit Side:     " << (closeSideInt == 4 ? "Close Long (4)" : "Close Short (2)") << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    // 4. Submit Limit Close Order
    std::stringstream closeOrder;
    closeOrder << "{"
               << "\"symbol\":\"" << bestSymbol << "\","
               << "\"price\":" << targetPrice << ","
               << "\"vol\":" << static_cast<int>(bestVol) << ","
               << "\"side\":" << closeSideInt << ","
               << "\"type\":1,"  
               << "\"openType\":" << bestOpenType
               << "}";

    std::string closeRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/order/create", closeOrder.str());
    std::cout << "Close Position Response: " << closeRes << std::endl;

    // 5. Clean up associated TP/SL plan orders for this specific symbol
    std::stringstream cancelPayload;
    cancelPayload << "{\"symbol\":\"" << bestSymbol << "\"}";
    sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/planorder/cancel_all", cancelPayload.str());
}
// --- Optimized & Atomic Orchestrator ---
// Unified Execution (Entry + OCO TP/SL): POST /api/v1/private/order/create
void openAtomicBracketFuturesPosition(const std::string& symbol, const std::string& side,
	double qty, double entryPrice, double stopLoss,
	double takeProfit, int leverage = 30) {

	if(symbol_opened(symbol)) return;

	if(max_symbols_reached())closeMostProfitablePosition();

	const char* envKey = std::getenv("MEXC_API_KEY");
	const char* envSecret = std::getenv("MEXC_API_SECRET");

	if (!envKey || !envSecret) {
	std::cerr << "Execution Blocked: Missing API credentials." << std::endl;
	return;
	}

	std::string apiKey(envKey);
	std::string apiSecret(envSecret);

	digits_contractsize csize = getContractSize(apiKey, apiSecret, symbol);
	int contracts = static_cast<int>(std::floor(qty / csize.contractSize));

	if (contracts <= 0) {
	std::cerr << "Execution Blocked: Calculated volume rounds to 0 contracts." << std::endl;
	return;
	}

	int entrySideInt = (side == "BUY") ? 1 : 3;

	// --- AUTO‑LEVERAGE (Corrected Direct Calculation) ---

	// --- AUTO‑LEVERAGE (Liquidation == Stop-Loss) ---

	int bestLev = 1; 
	double maxLevExact = 1.0;
	const double MMR = 0.004; // 0.4% Maintenance Margin Rate
	
	if (side == "BUY") {
		if (stopLoss < entryPrice) {
			maxLevExact = entryPrice / (entryPrice - stopLoss + (entryPrice * MMR));
		}
	} else { // "SELL" (Short)
		if (stopLoss > entryPrice) {
			maxLevExact = entryPrice / (stopLoss - entryPrice + (entryPrice * MMR));
		}
	}
	
	bestLev = (int)maxLevExact; 
	
	// Clamp to exchange rules (e.g., max 50x)
	if (bestLev > 50) bestLev = 50; 
	if (bestLev < 1)  bestLev = 1;
	
	// --- CALCULATE ACTUAL LIQUIDATION PRICE FROM CHOSEN LEVERAGE ---
	double liquidationPrice = 0.0;
	if (side == "BUY") {
		liquidationPrice = entryPrice * (1.0 - (1.0 / bestLev) + MMR);
	} else { // "SELL"
		liquidationPrice = entryPrice * (1.0 + (1.0 / bestLev) - MMR);
	}
	
	// --- PRINT OUTCOME ---
	std::cout << std::fixed << std::setprecision(6);
	std::cout << "--- Order Analysis ---" << std::endl;
	std::cout << "Side:              " << side << std::endl;
	std::cout << "Entry Price:       " << entryPrice << std::endl;
	std::cout << "Target Stop-Loss:  " << stopLoss << std::endl;
	std::cout << "Selected Leverage: " << bestLev << "x" << (bestLev == 50 ? " (Capped at Max)" : "") << std::endl;
	std::cout << "Liquidation Price: " << liquidationPrice << std::endl;
	std::cout << "----------------------" << std::endl;
	
	

	// --- ORDER PAYLOAD ---
	std::stringstream pOrder;
	pOrder << "{"
	<< "\"symbol\":\"" << symbol << "\","
	<< "\"price\":" << entryPrice << ","
	<< "\"vol\":" << contracts << ","
	<< "\"side\":" << entrySideInt << ","
	<< "\"type\":1,"
	<< "\"openType\":1,"
	<< "\"leverage\":" << bestLev << ","
	<< "\"stopLossPrice\":" << stopLoss << ","
	<< "\"takeProfitPrice\":" << takeProfit
	<< "}";

	std::cout << pOrder.str() << "\n";
	// return;
	std::cout << "Sending Unified Entry and TP/SL Order Payload...\n";

	std::string orderRes = sendFuturesRequest(apiKey, apiSecret, "POST",
				"/api/v1/private/order/create",
				pOrder.str());
	std::cout << "Exchange Response: " << orderRes << "\n";
}

// --- Orchestrator: Bracket Futures Position (JSON Adapted) ---
// Entry order:      POST /api/v1/private/order/create
// SL / TP orders:   POST /api/v1/private/planorder/place/v2   (CORRECTED — was /planorder/place)
void openBracketFuturesPosition1(const std::string& symbol, const std::string& side, double qty, double entryPrice, double stopLoss, double takeProfit, int pricePrecision, float volPrecision,int leverage = 20 ) {
    // NOTE on precision: MEXC enforces a per-contract tick size / lot size
    // (priceUnit / volUnit, available from the contract detail endpoint).
    // The fixed precision below is a placeholder — for symbols where the
    // real tick size differs from what's hardcoded here, the order will be
    // rejected ("price/vol precision error"). Fetch contract details per
    // symbol and pass the correct precision in for production use.

    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Execution Blocked: Missing API credentials." << std::endl;
        return;
    }

    std::string apiKey(envKey);
    std::string apiSecret(envSecret);

	digits_contractsize csize=getContractSize(apiKey,apiSecret,symbol);
	int contracts = static_cast<int>(std::floor(qty / csize.contractSize));

	cout<<"csize: "<<csize.contractSize<<" "<<contracts<<"\n";

    // MEXC Futures Side Ints: 1 = Open Long, 2 = Close Short, 3 = Open Short, 4 = Close Long
    int entrySideInt = (side == "BUY") ? 1 : 3;
    int exitSideInt  = (side == "BUY") ? 4 : 2;

    // --- STEP 1: Post Core Margin Position Entry ---
    // Place Order fields (per docs): symbol, price, vol, leverage, side, type, openType
    std::stringstream pOrder;
    pOrder << "{"
           << "\"symbol\":\"" << symbol << "\","
           << "\"price\":" <<  entryPrice << ","
        //    << "\"price\":" << std::fixed << std::setprecision(pricePrecision) << entryPrice << ","
           << "\"vol\":" << contracts << ","
           << "\"side\":" << entrySideInt << ","
           << "\"type\":1,"        // 1 = Limit Order
           << "\"openType\":1,"    // 1 = Isolated Margin
           << "\"leverage\":" << leverage
           << "}";
    std::cout << pOrder.str() << "\n";
	// return;
    std::cout << "Sending Futures Entry Order..." << std::endl;
    std::string entryRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/order/create", pOrder.str());
    std::cout << "Entry Response: " << entryRes << "\n\n";

    // --- STEP 2: Deploy Stop Loss Guard ---
    // Place Plan Order required fields (per docs): symbol, vol, leverage, side,
    // openType, triggerPrice, triggerType, executeCycle, orderType, trend
    std::stringstream slOrder;
    slOrder << "{"
            << "\"symbol\":\"" << symbol << "\","
            << "\"leverage\":" << leverage << ","
            << "\"openType\":1,"                          // 1 = isolated
            << "\"triggerPrice\":" << std::fixed << std::setprecision(pricePrecision) << stopLoss << ","
            // triggerType: for a long stop-loss, trigger when price <= stopLoss (2);
            // for a short stop-loss, trigger when price >= stopLoss (1).
            << "\"triggerType\":" << (side == "BUY" ? 2 : 1) << ","
            << "\"price\":" << (side == "BUY" ? stopLoss * 0.995 : stopLoss * 1.005) << ","
            << "\"vol\":" << contracts << ","
            << "\"side\":" << exitSideInt << ","
            << "\"orderType\":1,"     // 1 = Limit
            << "\"executeCycle\":1,"  // 1 = 24 hours (2 = 7 days) — there is no GTC option
            << "\"trend\":1"          // 1 = trigger off latest price
            << "}";

    std::cout << "Deploying Stop Loss Guard..." << std::endl;
    std::string slRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/planorder/place/v2", slOrder.str());
    std::cout << "Stop Loss Response: " << slRes << "\n\n";

    // --- STEP 3: Deploy Take Profit Order ---
    std::stringstream tpOrder;
    tpOrder << "{"
            << "\"symbol\":\"" << symbol << "\","
            << "\"leverage\":" << leverage << ","
            << "\"openType\":1,"
            << "\"triggerPrice\":" << std::fixed << std::setprecision(pricePrecision) << takeProfit << ","
            // triggerType: for a long take-profit, trigger when price >= takeProfit (1);
            // for a short take-profit, trigger when price <= takeProfit (2).
            << "\"triggerType\":" << (side == "BUY" ? 1 : 2) << ","
            << "\"price\":" << takeProfit << ","
            << "\"vol\":" << contracts << ","
            << "\"side\":" << exitSideInt << ","
            << "\"orderType\":1,"
            << "\"executeCycle\":1,"
            << "\"trend\":1"
            << "}";

    std::cout << "Deploying Take Profit Order..." << std::endl;
    std::string tpRes = sendFuturesRequest(apiKey, apiSecret, "POST", "/api/v1/private/planorder/place/v2", tpOrder.str());
    std::cout << "Take Profit Response: " << tpRes << "\n";
}

// --- Light Extraction Helper ---
// Searches for the USDT block and extracts the designated balance field
float extractUsdtBalance(const std::string& jsonResponse, const std::string& balanceField = "availableBalance") {
    size_t usdtPos = jsonResponse.find("\"currency\":\"USDT\"");
    if (usdtPos == std::string::npos) {
        return 0.0; // USDT block not found
    }

    size_t fieldPos = jsonResponse.find("\"" + balanceField + "\":", usdtPos);
    if (fieldPos == std::string::npos) {
        return 0.0;
    }

    size_t valueStart = fieldPos + balanceField.length() + 3;

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
struct money{
	float availableBalance;
	float equity;
};
money getUsdtFuturesBalance() {
    const char* envKey = std::getenv("MEXC_API_KEY");
    const char* envSecret = std::getenv("MEXC_API_SECRET");

    if (!envKey || !envSecret) {
        std::cerr << "Missing API credentials." << std::endl;
        return {0.0,0};
    }

    std::string rawJson = sendFuturesRequest(envKey, envSecret, "GET", "/api/v1/private/account/asset/USDT");
	cout<<"rawJson: "<<rawJson<<"\n";
    return {extractUsdtBalance(rawJson, "availableBalance"),extractUsdtBalance(rawJson, "equity")};
}

// int main(){
// 	closeOldestPosition();
// 	return 0;
// }

// --- Wrapper to Print Metrics ---
int print_account() {
    std::cout << "Fetching Futures Account Balances..." << std::endl;
    money accountInfo = getUsdtFuturesBalance();

    std::cout << "Account Metrics Details:\n" << accountInfo.equity << std::endl;
return 0;
    // 1. Fetching all currently open futures positions
    std::string openPositions = getOpenedFuturesPositions();
    std::cout << "Open Positions Data:\n" << openPositions << "\n" << std::endl;

    // 2. Example: Opening a 5x Futures Long on BTC_USDT (Notice MEXC futures uses the underscore)
    // openBracketFuturesPosition("BTC_USDT", "BUY", 0.025, 64200.00, 63100.00, 67500.00, 5);
    return 0;
}