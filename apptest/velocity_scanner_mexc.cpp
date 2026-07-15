#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <curl/curl.h>
#include "cJSON.h" 

struct symbolstruct {
    std::string symbol;
    double volume24h_usdt = 0.0; // Stored in Bybit's "turnover24h"
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::vector<symbolstruct> fetchTopBybitVolume() {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    std::vector<symbolstruct> outputSymbols;

    if (!curl) {
        std::cerr << "CURL initialization failure." << std::endl;
        return outputSymbols;
    }

    // Category 'linear' queries all USDT/USDC settled futures/perpetuals
    std::string url = "https://api.bybit.com/v5/market/tickers?category=linear";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "API request failed: " << curl_easy_strerror(res) << std::endl;
        return outputSymbols;
    }

    cJSON* parsedJson = cJSON_Parse(readBuffer.c_str());
    if (!parsedJson) {
        std::cerr << "JSON parsing error." << std::endl;
        return outputSymbols;
    }

    cJSON* retCode = cJSON_GetObjectItemCaseSensitive(parsedJson, "retCode");
    if (retCode && retCode->valueint == 0) {
        cJSON* result = cJSON_GetObjectItemCaseSensitive(parsedJson, "result");
        if (result) {
            cJSON* dataList = cJSON_GetObjectItemCaseSensitive(result, "list");
            if (cJSON_IsArray(dataList)) {
                int size = cJSON_GetArraySize(dataList);

                for (int i = 0; i < size; ++i) {
                    cJSON* item = cJSON_GetArrayItem(dataList, i);

                    cJSON* symObj = cJSON_GetObjectItemCaseSensitive(item, "symbol");
                    // "turnover24h" in Bybit linear tickers represents the exact 24-hour volume in USDT
                    cJSON* turnoverObj = cJSON_GetObjectItemCaseSensitive(item, "turnover24h"); 

                    if (!symObj || !cJSON_IsString(symObj)) continue;

                    std::string symbolStr = symObj->valuestring;
                    
                    // Filter: Only keep USDT pairs (ignoring USDC settled ones to keep things unified)
                    if (symbolStr.length() < 4 || symbolStr.compare(symbolStr.length() - 4, 4, "USDT") != 0) {
                        continue;
                    }

                    double turnover_usdt = turnoverObj && cJSON_IsString(turnoverObj) ? std::stod(turnoverObj->valuestring) : 0.0;

                    symbolstruct asset;
                    asset.symbol = symbolStr;
                    asset.volume24h_usdt = turnover_usdt;

                    outputSymbols.push_back(asset);
                }
            }
        }
    }

    cJSON_Delete(parsedJson);

    // Sort descending by 24h USDT Volume
    std::sort(outputSymbols.begin(), outputSymbols.end(), [](const symbolstruct& a, const symbolstruct& b) {
        return a.volume24h_usdt > b.volume24h_usdt;
    });

    // Keep only the top 20 most-traded assets
    if (outputSymbols.size() > 20) {
        outputSymbols.resize(20);
    }

    std::cout << "Bybit Top Volume Output size: " << outputSymbols.size() << "\n";
    return outputSymbols;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::vector<symbolstruct> topSymbols = fetchTopBybitVolume();

    std::cout << "std::vector<symbolstruct> symbols = {\n";
    for (size_t i = 0; i < topSymbols.size(); ++i) {
        // Formatted to output as "SYMBOL_USDT" to match your standard struct format
        std::string formattedSymbol = topSymbols[i].symbol;
        size_t pos = formattedSymbol.find("USDT");
        if (pos != std::string::npos && pos > 0) {
            formattedSymbol.insert(pos, "_"); // Turn BTCUSDT into BTC_USDT
        }
        
        std::cout << "    {\"" << formattedSymbol << "\"}";
        if (i < topSymbols.size() - 1) {
            std::cout << ",\n";
        } else {
            std::cout << "\n";
        }
    }
    std::cout << "};\n";

    curl_global_cleanup();
    return 0;
}