#include <iostream>
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

#include "settings.h"
#include "version.h"
#include "network_utils.h"
#include "version_manager.h"
#include "minecraft_launcher.h"
#include "forge.h"

int main(int argc, char* argv[]) {
    std::cout << "Welcome to MuffinLauncher" << std::endl;

    std::filesystem::create_directories(Settings::tempDir);
    std::filesystem::create_directories(Settings::versionsDir);

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <version>" << std::endl;
        return 1;
    }

    bool shouldInjectForge = false;
    std::string playerName = "Player";
    if (argc >= 3) {
        std::string arg1 = argv[2];
        if (arg1 == "--forge") {
            shouldInjectForge = true;
            if (argc >= 4) {
                playerName = argv[3];
            }
        } else {
            playerName = arg1;
            if (argc >= 4) {
                std::string arg2 = argv[3];
                if (arg2 == "--forge") {
                    shouldInjectForge = true;
                }
            }
        }
    }

    std::string version = argv[1];
    std::cout << "Trying to launch Minecraft " << version << std::endl;
    std::cout << " - Player name: " << playerName << std::endl;

    if (!versionValid(version)) {
        std::cerr << "Version " << version << " is not found, maybe invalid?" << std::endl;
        return 1;
    }

    VersionsListBetaCraft::VersionEntry ve;
    {
        std::string manifestContentBC;
        if (!downloadFileEasy(Settings::versionsJsonUrlBC, manifestContentBC, false, true)) {
            std::cerr << "Failed to download versions list" << std::endl;
            return 1;
        }
        auto j = nlohmann::json::parse(manifestContentBC, nullptr, false);
        if (j.is_discarded()) {
            std::cerr << "Failed to parse versions list JSON" << std::endl;
            return 1;
        }
        if (j.contains("versions") && j["versions"].is_array()) {
            for (const auto& v : j["versions"]) {
                if (v.contains("id") && v["id"].get<std::string>() == version) {
                    ve.id = v["id"].get<std::string>();
                    ve.url = v["url"].get<std::string>();
                    ve.releaseTime = v["releaseTime"].get<std::string>();
                    ve.time = v["time"].get<std::string>();
                    ve.type = v["type"].get<std::string>();
                    break;
                }
            }
        }

        std::string manifestContentMC;
        if (!downloadFileEasy(Settings::versionsJsonUrlMC, manifestContentMC, false, true)) {
            std::cerr << "Failed to download versions list" << std::endl;
            return 1;
        }
        auto j2 = nlohmann::json::parse(manifestContentMC, nullptr, false);
        if (j2.is_discarded()) {
            std::cerr << "Failed to parse versions list JSON" << std::endl;
            return 1;
        }
        if (j2.contains("versions") && j2["versions"].is_array()) {
            for (const auto& v : j2["versions"]) {
                if (v.contains("id") && v["id"].get<std::string>() == version) {
                    // do not override betacraft entry
                    if (!ve.id.empty()) {
                        break;
                    }
                    ve.id = v["id"].get<std::string>();
                    ve.url = v["url"].get<std::string>();
                    ve.releaseTime = v["releaseTime"].get<std::string>();
                    ve.time = v["time"].get<std::string>();
                    ve.type = v["type"].get<std::string>();
                    break;
                }
            }
        }

        if (ve.id.empty()) {
            std::cerr << "Version entry for " << version << " not found" << std::endl;
            return 1;
        }
    }

    VersionInfo vi;
    if (!downloadVersionInfo(ve, vi)) {
        std::cerr << "Failed to download version info for " << version << std::endl;
        return 1;
    }

    // perform fixup for arm64 macOS and old mc
    if (vi.javaVersion.majorVersion < 9) {
    VersionInfo::Library libJavaObjC;
    libJavaObjC.name = "ca.weblite:java-objc-bridge:1.1.0-mmachina.1";
    libJavaObjC.artifact.url = "https://github.com/MinecraftMachina/Java-Objective-C-Bridge/releases/download/1.1.0-mmachina.1/java-objc-bridge-1.1.jar";
    libJavaObjC.artifact.sha1 = "369a83621e3c65496348491e533cb97fe5f2f37d";
    libJavaObjC.artifact.size = 91947;
    vi.libraries.push_back(libJavaObjC);

    int addedLWJGL = 0;
    int addedLWJGLUtil = 0;
    int addedJInput = 0;
    int addedLWJGLPlatform = 0;
    int addedJInputPlatform = 0;
    for (auto& lib : vi.libraries) {
        if (lib.name.find("lwjgl") != std::string::npos && lib.name.find("lwjgl-platform") == std::string::npos && lib.name.find("lwjgl_util") == std::string::npos && lib.name.find("librarylwjglopenal") == std::string::npos) {
            if (addedLWJGL) {
                lib.artifact.url = "";
                continue;
            }
            // std::cout << " - Replacing lwjgl" << std::endl;
            lib.artifact.url = "https://libraries.minecraft.net/org/lwjgl/lwjgl/lwjgl/2.9.4-nightly-20150209/lwjgl-2.9.4-nightly-20150209.jar";
            lib.artifact.sha1 = "697517568c68e78ae0b4544145af031c81082dfe";
            lib.artifact.size = 1047168;
            addedLWJGL = 1;
        }

        if (lib.name.find("lwjgl_util") != std::string::npos) {
            if (addedLWJGLUtil) {
                lib.artifact.url = "";
                continue;
            }
            // std::cout << " - Replacing lwjgl_util" << std::endl;
            lib.artifact.url = "https://libraries.minecraft.net/org/lwjgl/lwjgl/lwjgl_util/2.9.4-nightly-20150209/lwjgl_util-2.9.4-nightly-20150209.jar";
            lib.artifact.sha1 = "d51a7c040a721d13efdfbd34f8b257b2df882ad0";
            lib.artifact.size = 173887;
            addedLWJGLUtil = 1;
        }

        if (lib.name.find("jinput") != std::string::npos && lib.name.find("jinput-platform") == std::string::npos) {
            if (addedJInput) {
                lib.artifact.url = "";
                continue;
            }
            lib.artifact.url = "https://libraries.minecraft.net/net/java/jinput/jinput/2.0.5/jinput-2.0.5.jar";
            lib.artifact.sha1 = "39c7796b469a600f72380316f6b1f11db6c2c7c4";
            lib.artifact.size = 208338;
            addedJInput = 1;
        }

        if (lib.name.find("lwjgl-platform") != std::string::npos) {
            if (addedLWJGLPlatform) {
                lib.natives.osx = "";
                continue;
            }
            // std::cout << " - Replacing lwjgl-platform natives" << std::endl;
            lib.natives.osx = "natives-macos-aarch64";
            lib.classifiers["natives-macos-aarch64"].url = "https://github.com/MinecraftMachina/lwjgl/releases/download/2.9.4-20150209-mmachina.2/lwjgl-platform-2.9.4-nightly-20150209-natives-osx.jar";
            lib.classifiers["natives-macos-aarch64"].sha1 = "eff546c0b319d6ffc7a835652124c18089c67f36";
            lib.classifiers["natives-macos-aarch64"].size = 488316;
            addedLWJGLPlatform = 1;
        }

        if (lib.name.find("jinput-platform") != std::string::npos) {
            if (addedJInputPlatform) {
                lib.natives.osx = "";
                continue;
            }
            // std::cout << " - Replacing jinput-platform natives" << std::endl;
            lib.natives.osx = "natives-macos-aarch64";
            lib.classifiers["natives-macos-aarch64"].url = "https://github.com/r58Playz/jinput-m1/raw/main/plugins/OSX/bin/jinput-platform-2.0.5.jar";
            lib.classifiers["natives-macos-aarch64"].sha1 = "bbccddeeff00112233445566778899aabbccddeeff";
            lib.classifiers["natives-macos-aarch64"].size = 123456;
            addedJInputPlatform = 1;
        }

        if (lib.name.find("com.mojang:text2speech") != std::string::npos) {
            // std::cout << " - Replacing com.mojang:text2speech" << std::endl;
            lib.artifact.url = "https://libraries.minecraft.net/com/mojang/text2speech/1.11.3/text2speech-1.11.3.jar";
        }
    }
    }

    if (shouldInjectForge) {
        ForgeVersionInfo fvi;
        if (!getRecommendedForgeVersion(version, fvi)) {
            std::cerr << "Failed to get recommended Forge version for " << version << std::endl;
            return 1;
        }
        std::cout << " - Injecting Forge " << fvi.forgeVersion << std::endl;
        if (!injectForgeIntoVersionInfo(fvi, vi)) {
            std::cerr << "Failed to inject Forge into the game" << std::endl;
            return 1;
        }
    }

    if (!downloadGameFilesForInfo(vi)) {
        std::cerr << "Failed to download game files for " << version << std::endl;
        return 1;
    }

    if (!launchMinecraft(vi, playerName)) {
        std::cerr << "Failed to launch Minecraft " << version << std::endl;
        return 1;
    }

    return 0;
}
