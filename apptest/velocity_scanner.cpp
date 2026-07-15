#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <curl/curl.h>

// Drop nlohmann/json, just include the lightweight cJSON C-header
// You just need to add cJSON.c and cJSON.h to your compilation
#include "cJSON.h" 

struct symbolstruct {
    std::string symbol;
    double volume24h = 0.0;
    double openInterest = 0.0;
    
    double getTurnoverRatio() const {
        if (openInterest <= 0.0) return 0.0;
        return volume24h / openInterest;
    }
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::vector<symbolstruct> fetchHyperliquidVelocity() {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    std::vector<symbolstruct> outputSymbols;

    if (!curl) {
        std::cerr << "CURL initialization failure." << std::endl;
        return outputSymbols;
    }

    std::string url = "https://api.hyperliquid.xyz/info";
    std::string jsonPayload = "{\"type\": \"metaAndAssetCtxs\"}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "API request failed: " << curl_easy_strerror(res) << std::endl;
        return outputSymbols;
    }

    // Parse using cJSON (Pure C, ultra lightweight)
    cJSON* parsedJson = cJSON_Parse(readBuffer.c_str());
    if (!parsedJson) {
        std::cerr << "JSON parsing error." << std::endl;
        return outputSymbols;
    }

    if (cJSON_IsArray(parsedJson) && cJSON_GetArraySize(parsedJson) >= 2) {
        cJSON* metaObj = cJSON_GetArrayItem(parsedJson, 0);
        cJSON* assetCtxs = cJSON_GetArrayItem(parsedJson, 1);

        cJSON* universeMeta = cJSON_GetObjectItemCaseSensitive(metaObj, "universe");

        if (cJSON_IsArray(universeMeta) && cJSON_IsArray(assetCtxs)) {
            int universeSize = cJSON_GetArraySize(universeMeta);
            int ctxsSize = cJSON_GetArraySize(assetCtxs);
            int itemsCount = std::min(universeSize, ctxsSize);

            for (int i = 0; i < itemsCount; ++i) {
                cJSON* metaItem = cJSON_GetArrayItem(universeMeta, i);
                cJSON* ctxItem = cJSON_GetArrayItem(assetCtxs, i);

                cJSON* nameObj = cJSON_GetObjectItemCaseSensitive(metaItem, "name");
                if (!cJSON_IsString(nameObj)) continue;

                symbolstruct asset;
                asset.symbol = std::string(nameObj->valuestring) + "_USDT";

                cJSON* volObj = cJSON_GetObjectItemCaseSensitive(ctxItem, "dayNtlVlm");
                cJSON* oiObj = cJSON_GetObjectItemCaseSensitive(ctxItem, "openInterest");

                // Hyperliquid API stores these metrics as string fields inside the JSON
                asset.volume24h = volObj && cJSON_IsString(volObj) ? std::stod(volObj->valuestring) : 0.0;
                asset.openInterest = oiObj && cJSON_IsString(oiObj) ? std::stod(oiObj->valuestring) : 0.0;

                if (asset.openInterest > 0.0) {
                    outputSymbols.push_back(asset);
                }
            }
        }
    }

    // Always free memory allocated by cJSON
    cJSON_Delete(parsedJson);

    // Sort descending by ratio
    std::sort(outputSymbols.begin(), outputSymbols.end(), [](const symbolstruct& a, const symbolstruct& b) {
        return a.getTurnoverRatio() > b.getTurnoverRatio();
    });
    std::cout<<"oss "<<outputSymbols.size()<<"\n";
    // if (outputSymbols.size() > 20) {
    //     outputSymbols.resize(20);
    // }

    return outputSymbols;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::vector<symbolstruct> top20Symbols = fetchHyperliquidVelocity();

    std::cout << "std::vector<symbolstruct> symbols = {\n";
    for (size_t i = 0; i < top20Symbols.size(); ++i) {
        std::cout << "    {\"" << top20Symbols[i].symbol << "\"}";
        if (i < top20Symbols.size() - 1) {
            std::cout << ",\n";
        } else {
            std::cout << "\n";
        }
    }
    std::cout << "};\n";

    curl_global_cleanup();
    return 0;
}