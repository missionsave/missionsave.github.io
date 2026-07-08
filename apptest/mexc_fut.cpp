#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <curl/curl.h>

struct FutureInfo {
    std::string symbol;
    double volume24;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string httpGet(const std::string& url)
{
    CURL* curl = curl_easy_init();
    std::string buffer;

    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return buffer;
}

int main()
{
    std::string data = httpGet("https://contract.mexc.com/api/v1/contract/ticker");

    std::vector<FutureInfo> futures;

    size_t pos = 0;
    while ((pos = data.find("\"symbol\":\"", pos)) != std::string::npos)
    {
        pos += 10; // skip "symbol":" 
        size_t end = data.find("\"", pos);
        std::string symbol = data.substr(pos, end - pos);

        // Filtrar apenas futuros USDT
        if (symbol.size() > 5 && symbol.substr(symbol.size() - 5) == "_USDT")
        {
            // Procurar volume24
            size_t vpos = data.find("\"volume24\":", end);
            if (vpos != std::string::npos)
            {
                vpos += 11; // skip "volume24":
                size_t vend = data.find(",", vpos);
                double volume24 = std::stod(data.substr(vpos, vend - vpos));

                futures.push_back({symbol, volume24});
            }
        }
        pos = end;
    }

    // Ordenar por liquidez (volume24)
    std::sort(futures.begin(), futures.end(),
              [](const FutureInfo& a, const FutureInfo& b) {
                  return a.volume24 > b.volume24;
              });

    std::cout << "=== Futuros USDT ordenados por liquidez (volume24) ===\n";
    for (auto& f : futures)
        std::cout << f.symbol << "  volume24=" << f.volume24 << "\n";

    return 0;
}
