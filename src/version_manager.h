#pragma once

#include "version.h"
#include <string>

bool versionValid(const std::string& version);
bool downloadVersionInfo(const VersionsListBetaCraft::VersionEntry& ve, VersionInfo& outVersionInfo);
bool downloadGameFilesForInfo(const VersionInfo& vi);
