#include "minecraft_launcher.h"
#include "settings.h"
#include "java_utils.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>

bool launchMinecraft(const VersionInfo& vi, const std::string& playerName) {
    std::string versionDir = Settings::versionsDir + vi.id + "/";
    std::string clientJarPath = versionDir + vi.id + ".jar";
    if (!std::filesystem::exists(clientJarPath)) {
        std::cerr << "Client jar does not exist? searched at: " << clientJarPath << std::endl;
        return false;
    }

    std::string classpath = "";
    for (const auto& lib : vi.libraries) {
        std::string libPath = versionDir + "libraries/";
        if (!lib.artifact.path.empty()) {
            libPath += lib.artifact.path;
        } else if (!lib.name.empty()) {
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
            libPath += groupId + "/" + artifactId + "/" + version + "/" + artifactId + "-" + version + ".jar";
        } else {
            continue;
        }

        if (std::filesystem::exists(libPath)) {
            if (!classpath.empty())
                classpath += ":" + libPath;
            else
                classpath = libPath;
        }
    }
    classpath += ":" + clientJarPath;

    if (!verifyJavaVersion(vi.javaVersion.majorVersion)) {
        return false;
    }

    // start building the final command to be excuted
    std::string command = "java";

    for (const auto& arg : vi.arguments.jvm) {
        command += " " + arg;
    }

    if (vi.arguments.jvm.empty()) {
        command += " -Djava.library.path=" + versionDir + "natives";
        command += " -cp " + classpath;
    }

    int majorJavaVersion = vi.javaVersion.majorVersion;
    if (majorJavaVersion >= 9) {
        command += " -XstartOnFirstThread";
    }

    command += " -Xmx2G";
    command += " -Xms1G";

    command += " " + vi.mainClass;

    for (const auto& arg : vi.arguments.game) {
        if (!arg.value.empty()) {
            for (const auto& v : arg.value) {
                if (v == "--demo" || v == "--server" || v == "--port" || v == "--mppass") {
                    continue;
                } else if (v == "${server_ip}" || v == "${server_port}") {
                    continue;
                } else if (v == "-") {
                    continue;
                } else if (v.find("quickPlay") != std::string::npos) {
                    continue;
                }
                command += " " + v;
            }
        }
    }

    if (vi.arguments.game.empty()) {
        command += " --username " + playerName;
        command += " --version " + vi.id;
        command += " --gameDir " + versionDir;
        command += " --assetsDir " + versionDir + "assets/";
        command += " --assetIndex " + vi.assetIndex.id;
        command += " --uuid 0";
        command += " --accessToken 0";
        command += " --userType offline";
        command += " --userProperties {}";
        command += " --width 854";
        command += " --height 480";
        command += " --versionType MuffinLauncher";

        // if we have forge, add "--tweakClass net.minecraftforge.fml.common.launcher.FMLTweaker"
        for (const auto& lib : vi.libraries) {
            if (lib.name.find("net.minecraftforge:forge") != std::string::npos) {
                std::cout << " - Injecting Forge" << std::endl;
                command += " --tweakClass net.minecraftforge.fml.common.launcher.FMLTweaker";
                break;
            }
        }
    }

    size_t pos;
    while ((pos = command.find("${natives_directory}")) != std::string::npos) {
        command.replace(pos, 20, versionDir + "natives");
    }
    while ((pos = command.find("${library_directory}")) != std::string::npos) {
        command.replace(pos, 20, versionDir + "libraries");
    }
    while ((pos = command.find("${instance_icon}")) != std::string::npos) {
        command.replace(pos, 16, "icon.png");
    }
    while ((pos = command.find("${profile_name}")) != std::string::npos) {
        command.replace(pos, 15, vi.id);
    }
    while ((pos = command.find("${classpath}")) != std::string::npos) {
        command.replace(pos, 12, classpath);
    }
    while ((pos = command.find("${auth_player_name}")) != std::string::npos) {
        command.replace(pos, 19, playerName);
    }
    while ((pos = command.find("${auth_access_token}")) != std::string::npos) {
        command.replace(pos, 20, "0");
    }
    while ((pos = command.find("${game_directory}")) != std::string::npos) {
        command.replace(pos, 17, versionDir);
    }
    while ((pos = command.find("${assets_root}")) != std::string::npos) {
        command.replace(pos, 14, versionDir + "assets/");
    }
    while ((pos = command.find("${assets_index_name}")) != std::string::npos) {
        command.replace(pos, 20, vi.assetIndex.id);
    }
    while ((pos = command.find("${resolution_width}")) != std::string::npos) {
        command.replace(pos, 19, "854");
    }
    while ((pos = command.find("${resolution_height}")) != std::string::npos) {
        command.replace(pos, 20, "480");
    }
    while ((pos = command.find("${auth_uuid}")) != std::string::npos) {
        command.replace(pos, 12, "0");
    }
    while ((pos = command.find("${version_name}")) != std::string::npos) {
        command.replace(pos, 15, vi.id);
    }
    while ((pos = command.find("${user_properties}")) != std::string::npos) {
        command.replace(pos, 18, "{}");
    }
    while ((pos = command.find("${user_type}")) != std::string::npos) {
        command.replace(pos, 12, "offline");
    }
    while ((pos = command.find("${clientid}")) != std::string::npos) {
        command.replace(pos, 11, "0");
    }
    while ((pos = command.find("${auth_xuid}")) != std::string::npos) {
        command.replace(pos, 12, "0");
    }
    while ((pos = command.find("${version_type}")) != std::string::npos) {
        command.replace(pos, 15, "MuffinLauncher");
    }

    std::cout << " - Launching Minecraft " << vi.id << std::endl;
    // std::cout << command << std::endl;
    int ret = system(command.c_str());
    return ret == 0;
}
