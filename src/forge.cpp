#include "forge.h"
#include <iostream>
#include <cstdio>
#include <string>
#include "network_utils.h"

bool getRecommendedForgeVersion(const std::string& mcVersion, ForgeVersionInfo& outInfo) {
    std::string apiUrl = "https://files.minecraftforge.net/maven/net/minecraftforge/forge/promotions_slim.json";
    std::string jsonResponse;
    if (!downloadFileEasy(apiUrl, jsonResponse, false, true)) {
        std::cerr << "Failed to download Forge versions index" << std::endl;
        return false;
    }

    size_t mcPos = jsonResponse.find("\"" + mcVersion + "-recommended\":");
    bool usedLatest = false;
    if (mcPos == std::string::npos) {
        std::cerr << "No recommended Forge version found for Minecraft " << mcVersion << std::endl;
        std::cerr << "using latest instead" << std::endl;
        mcPos = jsonResponse.find("\"" + mcVersion + "-latest\":");
        if (mcPos == std::string::npos) {
            std::cerr << "No latest Forge version found for Minecraft " << mcVersion << std::endl;
            return false;
        }
        usedLatest = true;
        // return false;
    }
    // size_t versionStart = jsonResponse.find("\"", mcPos + mcVersion.length() + 14);
    size_t versionStart = jsonResponse.find("\"", mcPos + mcVersion.length() + (usedLatest ? 11 : 14));
    if (versionStart == std::string::npos) {
        std::cerr << "could not find start of version string" << std::endl;
        return false;
    }
    size_t versionEnd = jsonResponse.find("\"", versionStart + 1);
    if (versionEnd == std::string::npos) {
        std::cerr << "could not find end of version string" << std::endl;
        return false;
    }
    std::string forgeVersion = jsonResponse.substr(versionStart + 1, versionEnd - versionStart - 1);

    outInfo.minecraftVersion = mcVersion;
    outInfo.forgeVersion = forgeVersion;
    outInfo.metadataUrl = "https://meta.prismlauncher.org/v1/net.minecraftforge/" + forgeVersion + ".json";

    return true;
}

bool injectForgeIntoVersionInfo(const ForgeVersionInfo& fvi, VersionInfo& vi) {
    std::string forgeJsonContent;
    if (!downloadFileEasy(fvi.metadataUrl, forgeJsonContent, false, true)) {
        std::cerr << "Failed to download Forge metadata JSON from " << fvi.metadataUrl << std::endl;
        return false;
    }

    auto j = nlohmann::json::parse(forgeJsonContent, nullptr, false);
    if (j.is_discarded()) {
        std::cerr << "Failed to parse Forge metadata JSON" << std::endl;
        return false;
    }
    
    VersionInfo forgeInfo = VersionInfo::fromJson(j);
    if (forgeInfo.uid.empty()) {
        std::cerr << "Forge metadata JSON parsing error?" << std::endl;
        return false;
    }

    // vi.libraries.insert(vi.libraries.end(), forgeInfo.libraries.begin(), forgeInfo.libraries.end());

    // older forge versions don't have the full url in the metadata, only the maven repo base url.
    for (auto& lib : forgeInfo.libraries) {
        // if (lib.url.empty()) {
        //     lib.url = "https://libraries.minecraft.net/";
        // }
        if (lib.artifact.url.empty()) {
            std::string baseUrl = lib.url.empty() ? "https://libraries.minecraft.net/" : lib.url;
            size_t colon1 = lib.name.find(':');
            size_t colon2 = lib.name.find(':', colon1 + 1);
            size_t colon3 = lib.name.find(':', colon2 + 1);
            if (colon3 != std::string::npos) {
                lib.name[colon3] = '-';
            }

            // :sob:
            if (colon1 != std::string::npos && colon2 != std::string::npos) {
                std::string group = lib.name.substr(0, colon1);
                std::string artifact = lib.name.substr(colon1 + 1, colon2 - colon1 - 1);
                std::string version = lib.name.substr(colon2 + 1);
                //https://maven.minecraftforge.net/net/minecraftforge/forge/1.11.2-13.20.1.2588/forge-1.11.2-13.20.1.2588-universal.jar
                std::string versionDir = version;
                size_t dash = versionDir.rfind('-');
                if (dash != std::string::npos) {
                    versionDir = versionDir.substr(0, dash);
                }
                std::string path = group;
                for (auto& c : path) {
                    if (c == '.') c = '/';
                }
                path += "/" + artifact + "/" + versionDir + "/" + artifact + "-" + version + ".jar";
                lib.artifact.url = baseUrl + path;
            }
        }
        vi.libraries.push_back(lib);
    }
    vi.mainClass = forgeInfo.mainClass;

    return true;
}
