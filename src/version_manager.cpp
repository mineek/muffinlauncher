#include "version.h"
#include "settings.h"
#include "network_utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <algorithm>

bool versionValid(const std::string& version) {
    std::string jsonContentBC;
    std::string jsonContentMC;
    if (!downloadFileEasy(Settings::versionsJsonUrlBC, jsonContentBC, false, true)) {
        std::cerr << "Failed to download versions list (Betacraft)" << std::endl;
        return false;
    }
    if (!downloadFileEasy(Settings::versionsJsonUrlMC, jsonContentMC, false, true)) {
        std::cerr << "Failed to download versions list (Mojang)" << std::endl;
        return false;
    }

    auto j = nlohmann::json::parse(jsonContentBC, nullptr, false);
    if (j.is_discarded()) {
        std::cerr << "Failed to parse versions list JSON" << std::endl;
        return false;
    }

    VersionsListBetaCraft versionsListBC;
    if (j.contains("trim_at")) {
        versionsListBC.trim_at = j["trim_at"].get<std::string>();
    }
    if (j.contains("versions") && j["versions"].is_array()) {
        for (const auto& v : j["versions"]) {
            VersionsListBetaCraft::VersionEntry ve;
            if (v.contains("id")) ve.id = v["id"].get<std::string>();
            if (v.contains("url")) ve.url = v["url"].get<std::string>();
            if (v.contains("releaseTime")) ve.releaseTime = v["releaseTime"].get<std::string>();
            if (v.contains("time")) ve.time = v["time"].get<std::string>();
            if (v.contains("type")) ve.type = v["type"].get<std::string>();
            versionsListBC.versions.push_back(ve);
        }
    }

    for (const auto& ve : versionsListBC.versions) {
        if (ve.id == version) {
            std::cout << " - Found version " << version << " (" << ve.type << ", released " << ve.releaseTime << ")" << std::endl;
            return true;
        }
    }

    auto j2 = nlohmann::json::parse(jsonContentMC, nullptr, false);
    if (j2.is_discarded()) {
        std::cerr << "Failed to parse versions list JSON" << std::endl;
        return false;
    }
    VersionListMojang versionsListMC;
    if (j2.contains("latest")) {
        const auto& latest = j2["latest"];
        if (latest.contains("release")) versionsListMC.latest.release = latest["release"].get<std::string>();
        if (latest.contains("snapshot")) versionsListMC.latest.snapshot = latest["snapshot"].get<std::string>();
    }
    if (j2.contains("versions") && j2["versions"].is_array()) {
        for (const auto& v : j2["versions"]) {
            VersionListMojang::VersionEntry ve;
            if (v.contains("id")) ve.id = v["id"].get<std::string>();
            if (v.contains("type")) ve.type = v["type"].get<std::string>();
            if (v.contains("url")) ve.url = v["url"].get<std::string>();
            if (v.contains("time")) ve.time = v["time"].get<std::string>();
            if (v.contains("releaseTime")) ve.releaseTime = v["releaseTime"].get<std::string>();
            versionsListMC.versions.push_back(ve);
        }
    }
    for (const auto& ve : versionsListMC.versions) {
        if (ve.id == version) {
            std::cout << " - Found version " << version << " (" << ve.type << ", released " << ve.releaseTime << ")" << std::endl;
            return true;
        }
    }

    return false;
}

bool downloadVersionInfo(const VersionsListBetaCraft::VersionEntry& ve, VersionInfo& outVersionInfo) {
    std::string versionJsonContent;
    if (!downloadFileEasy(ve.url, versionJsonContent, false, true)) {
        std::cerr << "Failed to download version info from " << ve.url << std::endl;
        return false;
    }

    auto j = nlohmann::json::parse(versionJsonContent, nullptr, false);
    if (j.is_discarded()) {
        std::cerr << "Failed to parse version info JSON" << std::endl;
        return false;
    }

    outVersionInfo = VersionInfo::fromJson(j);
    return true;
}

bool downloadGameFilesForInfo(const VersionInfo& vi) {
    // main client jar
    std::string versionDir = Settings::versionsDir + vi.id + "/";
    std::filesystem::create_directories(versionDir);
    std::string clientJarPath = versionDir + vi.id + ".jar";
    std::string clientJarContent;

    if (!std::filesystem::exists(clientJarPath)) {
        if (!downloadFileEasy(vi.downloads.client.url, clientJarContent)) {
            std::cerr << "Failed to download client jar from " << vi.downloads.client.url << std::endl;
            return false;
        }

        std::ofstream ofs(clientJarPath, std::ios::binary);
        if (!ofs) {
            std::cerr << "Failed to open file " << clientJarPath << std::endl;
            return false;
        }
        ofs.write(clientJarContent.data(), clientJarContent.size());
        ofs.close();
    }

    //libs
    std::string libsDir = versionDir + "libraries/";
    std::filesystem::create_directories(libsDir);
    for (const auto& lib : vi.libraries) {
        if (lib.artifact.url.empty()) continue;
        std::string libPath = libsDir + lib.artifact.path;

        std::string url = lib.artifact.url;

        if (lib.artifact.path.empty() && !lib.name.empty()) {
            std::string namePath = lib.name;
            size_t firstColon = namePath.find(':');
            size_t secondColon = namePath.find(':', firstColon + 1);
            if (firstColon == std::string::npos || secondColon == std::string::npos) {
                std::cerr << "Invalid library name format " << lib.name << std::endl;
                continue;
            }
            std::string groupId = namePath.substr(0, firstColon);
            std::string artifactId = namePath.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string version = namePath.substr(secondColon + 1);
            std::replace(groupId.begin(), groupId.end(), '.', '/');
            libPath = libsDir + groupId + "/" + artifactId + "/" + version + "/" + artifactId + "-" + version + ".jar";
        }
        std::filesystem::create_directories(std::filesystem::path(libPath).parent_path());

        if (std::filesystem::exists(libPath)) {
            continue;
        }
        std::string libContent;
        if (!downloadFileEasy(url, libContent)) {
            std::cerr << "Failed to download library from " << url << std::endl;
            continue;
        }
        std::ofstream libOfs(libPath, std::ios::binary);
        if (!libOfs) {
            std::cerr << "Failed to open file " << libPath << std::endl;
            continue;
        }
        libOfs.write(libContent.data(), libContent.size());
        libOfs.close();
    }

    std::string assetIndexContent;
    if (!downloadFileEasy(vi.assetIndex.url, assetIndexContent, false, true)) {
        std::cerr << "Failed to download asset index from " << vi.assetIndex.url << std::endl;
        return false;
    }
    std::string assetsDir = versionDir + "assets/";
    std::filesystem::create_directories(assetsDir);
    std::string assetIndexPath = assetsDir + vi.assetIndex.id + ".json";
    std::ofstream aiOfs(assetIndexPath, std::ios::binary);
    if (!aiOfs) {
        std::cerr << "Failed to open file " << assetIndexPath << std::endl;
        return false;
    }
    aiOfs.write(assetIndexContent.data(), assetIndexContent.size());
    aiOfs.close();

    //assets
    AssetIndex assetIndex;
    auto aj = nlohmann::json::parse(assetIndexContent, nullptr, false);
    if (aj.is_discarded() || !aj.contains("objects")) {
        std::cerr << "Failed to parse asset index JSON" << std::endl;
        return false;
    }
    for (auto it = aj["objects"].begin(); it != aj["objects"].end(); ++it) {
        std::string key = it.key();
        std::string hash = it.value().value("hash", "");
        int size = it.value().value("size", 0);
        assetIndex.objects[key] = {hash, size};
    }

    std::filesystem::create_directories(assetsDir + "objects/");
    std::filesystem::create_directories(assetsDir + "indexes/");
    std::filesystem::copy_file(assetIndexPath, assetsDir + "indexes/" + vi.assetIndex.id + ".json", std::filesystem::copy_options::overwrite_existing);

    std::cout << " - Downloading " << assetIndex.objects.size() << " assets" << std::endl;
    std::atomic<int> assetsDownloaded = 0;
    const int numThreads = 8;
    std::vector<std::thread> threads;
    auto it = assetIndex.objects.begin();
    std::mutex itMutex;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            while (true) {
                std::pair<std::string, std::pair<std::string, int>> item;
                {
                    std::lock_guard<std::mutex> lock(itMutex);
                    if (it == assetIndex.objects.end()) {
                        break;
                    }
                    item = *it;
                    ++it;
                }
                std::string key = item.first;
                std::string hash = item.second.first;
                std::string assetUrl = "https://resources.download.minecraft.net/" + hash.substr(0, 2) + "/" + hash;
                std::string assetPath = assetsDir + "objects/" + hash.substr(0, 2) + "/" + hash;
                std::filesystem::create_directories(std::filesystem::path(assetPath).parent_path());
                if (std::filesystem::exists(assetPath)) {
                    assetsDownloaded++;
                    continue;
                }
                std::string assetContent;
                if (!downloadFileEasy(assetUrl, assetContent, true)) {
                    std::cerr << "Failed to download asset: " << key << std::endl;
                    continue;
                }
                std::ofstream assetOfs(assetPath, std::ios::binary);
                if (!assetOfs) {
                    std::cerr << "Failed to open file " << assetPath << std::endl;
                    continue;
                }
                assetOfs.write(assetContent.data(), assetContent.size());
                assetOfs.close();
                assetsDownloaded++;
                std::cout << " - Downloaded asset " << assetsDownloaded.load() << "/" << assetIndex.objects.size() << "\r" << std::flush;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << std::endl;

    // natives
    std::string nativesDir = versionDir + "natives/";
    std::filesystem::create_directories(nativesDir);
    for (const auto& lib : vi.libraries) {
        if (!lib.natives.osx.empty()) {
            auto it = lib.classifiers.find(lib.natives.osx);
            if (it == lib.classifiers.end()) continue;
            const auto& classifier = it->second;
            std::string nativePath = nativesDir + classifier.path;
            if (classifier.path.empty() && !lib.name.empty()) {
                std::string namePath = lib.name;
                size_t firstColon = namePath.find(':');
                size_t secondColon = namePath.find(':', firstColon + 1);
                if (firstColon == std::string::npos || secondColon == std::string::npos) {
                    std::cerr << "Invalid library name format " << lib.name << std::endl;
                    continue;
                }
                std::string groupId = namePath.substr(0, firstColon);
                std::string artifactId = namePath.substr(firstColon + 1, secondColon - firstColon - 1);
                std::string version = namePath.substr(secondColon + 1);
                std::replace(groupId.begin(), groupId.end(), '.', '/');
                nativePath = nativesDir + groupId + "/" + artifactId + "/" + version + "/" + artifactId + "-" + version + "-" + lib.natives.osx + ".jar";
            }
            std::filesystem::create_directories(std::filesystem::path(nativePath).parent_path());
            if (std::filesystem::exists(nativePath)) {
                continue;
            }
            std::string nativeContent;
            std::string url = classifier.url;
            if (!downloadFileEasy(url, nativeContent)) {
                std::cerr << "Failed to download native " << url << std::endl;
                continue;
            }
            std::ofstream nativeOfs(nativePath, std::ios::binary);
            if (!nativeOfs) {
                std::cerr << "Failed to open " << nativePath << std::endl;
                continue;
            }
            nativeOfs.write(nativeContent.data(), nativeContent.size());
            nativeOfs.close();

            // eh, I was too lazy to include a zip library, will fix later :D
            std::string unzipCmd = "unzip -o " + nativePath + " -d " + nativesDir;
            int ret = system(unzipCmd.c_str());
            if (ret != 0) {
                std::cerr << "Failed to unzip native " << nativePath << std::endl;
                continue;
            }
        }
    }

    std::filesystem::remove_all(nativesDir + "META-INF");

    return true;
}
