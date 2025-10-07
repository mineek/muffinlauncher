#include "network_utils.h"
#include <iostream>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include "settings.h"

bool downloadFileEasy(const std::string& url, std::string& outContent, bool quiet, bool cache) {
    if (!quiet) {
        std::cout << " - Downloading " << url << std::endl;
    }
    if (cache) {
        std::string cacheDir = Settings::tempDir + "cache/";
        std::filesystem::create_directories(cacheDir);
        std::string filename;
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash != std::string::npos && lastSlash + 1 < url.size()) {
            filename = url.substr(lastSlash + 1);
        } else {
            cache = false;
        }
        if (cache) {
            // sanitize
            for (char& c : filename) {
                if (c == '/' || c == '\\' || c == '?' || c == '%' || c == '*' || c == ':' || c == '|' || c == '"' || c == '<' || c == '>') {
                    c = '_';
                }
            }

            std::string cachePath = cacheDir + std::to_string(std::hash<std::string>{}(url)) + "_" + filename;
            if (std::filesystem::exists(cachePath)) {
                if (!quiet) {
                    std::cout << "   - Used cache " << cachePath << std::endl;
                }
                std::ifstream f(cachePath);
                outContent.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                f.close();
                return true;
            }
            if (!quiet) {
                std::cout << "   - Caching to " << cachePath << std::endl;
            }
            CURL* curl;
            CURLcode res;
            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, std::string* data) {
                    data->append((char*)ptr, size * nmemb);
                    return size * nmemb;
                });
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outContent);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                if (res == CURLE_OK) {
                    std::ofstream f(cachePath);
                    f.write(outContent.data(), outContent.size());
                    f.close();
                    return true;
                } else {
                    return false;
                }
            }
            return false;
        }
    }
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, std::string* data) {
            data->append((char*)ptr, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outContent);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return res == CURLE_OK;
    }
    return false;
}