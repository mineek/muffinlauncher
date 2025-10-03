#pragma once

#include <string>
#include "version.h"

struct ForgeVersionInfo {
    std::string minecraftVersion;
    std::string forgeVersion;
    std::string metadataUrl;
};

bool getRecommendedForgeVersion(const std::string& mcVersion, ForgeVersionInfo& outInfo);
bool injectForgeIntoVersionInfo(const ForgeVersionInfo& fvi, VersionInfo& vi);

