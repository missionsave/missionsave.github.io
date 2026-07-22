#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <curl/curl.h>
#include "cJSON.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::string fetchUrl(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; CryptoLiquids/1.0)");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;
            response.clear();
        }
        curl_easy_cleanup(curl);
    }
    return response;
}

struct Asset {
    std::string symbol;
    double volume;
    double price;
};

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string url = 
        "https://api.coingecko.com/api/v3/coins/markets?"
        "vs_currency=usd&order=volume_desc&per_page=100&page=1&sparkline=false";

    std::string jsonData = fetchUrl(url);
    if (jsonData.empty()) {
        std::cerr << "Falha ao obter dados do CoinGecko." << std::endl;
        curl_global_cleanup();
        return 1;
    }

    cJSON* root = cJSON_Parse(jsonData.c_str());
    if (!root || !cJSON_IsArray(root)) {
        std::cerr << "Erro no parse JSON." << std::endl;
        if (root) cJSON_Delete(root);
        curl_global_cleanup();
        return 1;
    }

    std::vector<Asset> assets;
    int size = cJSON_GetArraySize(root);
    for (int i = 0; i < size; ++i) {
        cJSON* item = cJSON_GetArrayItem(root, i);
        if (!item) continue;
        cJSON* sym = cJSON_GetObjectItem(item, "symbol");
        cJSON* vol = cJSON_GetObjectItem(item, "total_volume");
        cJSON* prc = cJSON_GetObjectItem(item, "current_price");
        if (sym && vol && prc) {
            std::string s = sym->valuestring;
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            double v = vol->valuedouble;
            double p = prc->valuedouble;
            if (v > 0) assets.push_back({s, v, p});
        }
    }
    cJSON_Delete(root);

    // Os dados já vêm ordenados por volume, mas por segurança:
    std::sort(assets.begin(), assets.end(),
              [](const Asset& a, const Asset& b) { return a.volume > b.volume; });

    std::cout << "Top 20 ativos mais líquidos em USDT (CoinGecko - volume agregado global):\n";
    std::cout << "Pos | Símbolo | Volume (USDT)  | Preço (USDT)\n";
    int count = 0;
    for (const auto& a : assets) {
        if (++count > 30) break;
        std::cout << count << "  | " << a.symbol 
                  << " | " << a.volume 
                  << " | " << a.price << "\n";
    }

    curl_global_cleanup();
    return 0;
}