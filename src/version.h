#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

class VersionsListBetaCraft {
public:
    struct VersionEntry {
        std::string id;
        std::string url;
        std::string releaseTime;
        std::string time;
        std::string type;
    };
    std::string trim_at;
    std::vector<VersionEntry> versions;
};

class VersionListMojang {
public:
    struct VersionEntry {
        std::string id;
        std::string type;
        std::string url;
        std::string time;
        std::string releaseTime;
    };
    struct Latest {
        std::string release;
        std::string snapshot;
    };
    Latest latest;
    std::vector<VersionEntry> versions;
};

class VersionInfo {
public:
    struct ArgumentRule {
        struct OS {
            std::string name;
        };
        std::string action;
        OS os;
        struct Features {
            bool is_demo_user = false;
            bool has_server = false;
        };
        Features features;
    };

    struct Argument {
        std::vector<std::string> value;
        std::vector<ArgumentRule> rules;
    };

    struct Arguments {
        std::vector<std::string> jvm;
        std::vector<Argument> game;
    };

    struct AssetIndex {
        std::string id;
        std::string sha1;
        int size;
        int totalSize;
        std::string url;
    };

    struct JavaVersion {
        std::string component;
        int majorVersion;
        int minVersion;
        int advisedMaxVersion;
    };

    struct Download {
        std::string sha1;
        int size;
        std::string url;
    };

    struct Downloads {
        Download client;
        Download server;
        Download windows_server;
    };

    struct Library {
        struct Artifact {
            std::string path;
            std::string sha1;
            int size;
            std::string url;
        };
        struct Classifier {
            std::string path;
            std::string sha1;
            int size;
            std::string url;
        };
        struct Natives {
            std::string linux;
            std::string osx;
            std::string windows;
            std::string arch;
        };
        struct Extract {
            std::vector<std::string> exclude;
        };
        struct Rule {
            std::string action;
            struct OS {
                std::string name;
            };
            OS os;
        };
        Artifact artifact;
        std::map<std::string, Classifier> classifiers;
        Extract extract;
        std::string name;
        std::string url;
        Natives natives;
        std::vector<Rule> rules;
    };

    struct Logging {
        struct File {
            std::string id;
            std::string sha1;
            int size;
            std::string url;
        };
        std::string argument;
        File file;
        std::string type;
    };

    Arguments arguments;
    AssetIndex assetIndex;
    std::string assets;
    int complianceLevel;
    Downloads downloads;
    std::string id;
    std::string uid;
    JavaVersion javaVersion;
    std::string protocol_version;
    std::vector<Library> libraries;
    Logging logging;
    std::string mainClass;
    int minimumLauncherVersion;
    std::string releaseTime;
    std::string time;
    std::string type;

    static VersionInfo fromJson(const nlohmann::json& j);
};

class AssetIndex {
public:
    std::map<std::string, std::pair<std::string, int>> objects;
};
