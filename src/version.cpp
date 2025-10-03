#include "version.h"

VersionInfo VersionInfo::fromJson(const nlohmann::json& j) {
    VersionInfo vi;

    if (j.contains("arguments")) {
        const auto& args = j["arguments"];
        if (args.contains("jvm")) {
            for (const auto& v : args["jvm"]) {
                if (v.is_string()) {
                    vi.arguments.jvm.push_back(v.get<std::string>());
                }
            }
        }
        if (args.contains("game")) {
            for (const auto& arg : args["game"]) {
                Argument argument;
                if (arg.is_string()) {
                    argument.value.push_back(arg.get<std::string>());
                } else if (arg.is_object()) {
                    if (arg.contains("value")) {
                        if (arg["value"].is_string()) {
                            argument.value.push_back(arg["value"].get<std::string>());
                        } else if (arg["value"].is_array()) {
                            for (const auto& v : arg["value"]) {
                                if (v.is_string()) {
                                    argument.value.push_back(v.get<std::string>());
                                }
                            }
                        }
                    }
                    if (arg.contains("rules")) {
                        for (const auto& rule : arg["rules"]) {
                            ArgumentRule ar;
                            if (rule.contains("action")) {
                                ar.action = rule["action"].get<std::string>();
                            }
                            if (rule.contains("os") && rule["os"].contains("name")) {
                                ar.os.name = rule["os"]["name"].get<std::string>();
                            }
                            if (rule.contains("features")) {
                                if (rule["features"].contains("is_demo_user")) {
                                    ar.features.is_demo_user = rule["features"]["is_demo_user"].get<bool>();
                                }
                                if (rule["features"].contains("has_server")) {
                                    ar.features.has_server = rule["features"]["has_server"].get<bool>();
                                }
                            }
                            argument.rules.push_back(ar);
                        }
                    }
                }
                vi.arguments.game.push_back(argument);
            }
        }
    }

    if (j.contains("assetIndex")) {
        const auto& ai = j["assetIndex"];
        if (ai.contains("id")) vi.assetIndex.id = ai["id"].get<std::string>();
        if (ai.contains("sha1")) vi.assetIndex.sha1 = ai["sha1"].get<std::string>();
        if (ai.contains("size")) vi.assetIndex.size = ai["size"].get<int>();
        if (ai.contains("totalSize")) vi.assetIndex.totalSize = ai["totalSize"].get<int>();
        if (ai.contains("url")) vi.assetIndex.url = ai["url"].get<std::string>();
    }

    if (j.contains("assets")) vi.assets = j["assets"].get<std::string>();
    if (j.contains("complianceLevel")) vi.complianceLevel = j["complianceLevel"].get<int>();

    if (j.contains("downloads")) {
        const auto& dls = j["downloads"];
        if (dls.contains("client")) {
            const auto& c = dls["client"];
            if (c.contains("sha1")) vi.downloads.client.sha1 = c["sha1"].get<std::string>();
            if (c.contains("size")) vi.downloads.client.size = c["size"].get<int>();
            if (c.contains("url")) vi.downloads.client.url = c["url"].get<std::string>();
        }
        if (dls.contains("server")) {
            const auto& s = dls["server"];
            if (s.contains("sha1")) vi.downloads.server.sha1 = s["sha1"].get<std::string>();
            if (s.contains("size")) vi.downloads.server.size = s["size"].get<int>();
            if (s.contains("url")) vi.downloads.server.url = s["url"].get<std::string>();
        }
        if (dls.contains("windows_server")) {
            const auto& ws = dls["windows_server"];
            if (ws.contains("sha1")) vi.downloads.windows_server.sha1 = ws["sha1"].get<std::string>();
            if (ws.contains("size")) vi.downloads.windows_server.size = ws["size"].get<int>();
            if (ws.contains("url")) vi.downloads.windows_server.url = ws["url"].get<std::string>();
        }
    }

    if (j.contains("id")) vi.id = j["id"].get<std::string>();
    if (j.contains("uid")) vi.uid = j["uid"].get<std::string>();

    if (j.contains("javaVersion")) {
        const auto& jv = j["javaVersion"];
        if (jv.contains("component")) vi.javaVersion.component = jv["component"].get<std::string>();
        if (jv.contains("majorVersion")) vi.javaVersion.majorVersion = jv["majorVersion"].get<int>();
        if (jv.contains("minVersion")) vi.javaVersion.minVersion = jv["minVersion"].get<int>();
        if (jv.contains("advisedMaxVersion")) vi.javaVersion.advisedMaxVersion = jv["advisedMaxVersion"].get<int>();
    }

    if (j.contains("protocol_version")) vi.protocol_version = j["protocol_version"].get<std::string>();

    if (j.contains("libraries")) {
        for (const auto& lib : j["libraries"]) {
            Library library;
            if (lib.contains("downloads")) {
                const auto& dls = lib["downloads"];
                if (dls.contains("artifact")) {
                    const auto& art = dls["artifact"];
                    if (art.contains("path")) library.artifact.path = art["path"].get<std::string>();
                    if (art.contains("sha1")) library.artifact.sha1 = art["sha1"].get<std::string>();
                    if (art.contains("size")) library.artifact.size = art["size"].get<int>();
                    if (art.contains("url")) library.artifact.url = art["url"].get<std::string>();
                }
                if (dls.contains("classifiers")) {
                    for (auto it = dls["classifiers"].begin(); it != dls["classifiers"].end(); ++it) {
                        Library::Classifier classifier;
                        classifier.path = it.value().contains("path") ? it.value()["path"].get<std::string>() : "";
                        classifier.sha1 = it.value().contains("sha1") ? it.value()["sha1"].get<std::string>() : "";
                        classifier.size = it.value().contains("size") ? it.value()["size"].get<int>() : 0;
                        classifier.url = it.value().contains("url") ? it.value()["url"].get<std::string>() : "";
                        library.classifiers[it.key()] = classifier;
                    }
                }
            }
            if (lib.contains("extract") && lib["extract"].contains("exclude")) {
                for (const auto& ex : lib["extract"]["exclude"]) {
                    library.extract.exclude.push_back(ex.get<std::string>());
                }
            }
            if (lib.contains("name")) library.name = lib["name"].get<std::string>();
            if (lib.contains("url")) library.url = lib["url"].get<std::string>();
            if (lib.contains("natives")) {
                const auto& n = lib["natives"];
                if (n.contains("linux")) library.natives.linux = n["linux"].get<std::string>();
                if (n.contains("osx-aarch64")) library.natives.osx = n["osx-aarch64"].get<std::string>();
                else if (n.contains("osx")) library.natives.osx = n["osx"].get<std::string>();
                if (n.contains("windows")) library.natives.windows = n["windows"].get<std::string>();
                if (n.contains("arch")) library.natives.arch = n["arch"].get<std::string>();
            }
            if (lib.contains("rules")) {
                for (const auto& rule : lib["rules"]) {
                    Library::Rule r;
                    if (rule.contains("action")) {
                        r.action = rule["action"].get<std::string>();
                    }
                    if (rule.contains("os") && rule["os"].contains("name")) {
                        r.os.name = rule["os"]["name"].get<std::string>();
                    }
                    library.rules.push_back(r);
                }
            }
            vi.libraries.push_back(library);
        }
    }

    if (j.contains("logging") && j["logging"].contains("client")) {
        const auto& log = j["logging"]["client"];
        if (log.contains("argument")) vi.logging.argument = log["argument"].get<std::string>();
        if (log.contains("file")) {
            const auto& f = log["file"];
            if (f.contains("id")) vi.logging.file.id = f["id"].get<std::string>();
            if (f.contains("sha1")) vi.logging.file.sha1 = f["sha1"].get<std::string>();
            if (f.contains("size")) vi.logging.file.size = f["size"].get<int>();
            if (f.contains("url")) vi.logging.file.url = f["url"].get<std::string>();
        }
        if (log.contains("type")) vi.logging.type = log["type"].get<std::string>();
    }

    if (j.contains("mainClass")) vi.mainClass = j["mainClass"].get<std::string>();
    if (j.contains("minimumLauncherVersion")) vi.minimumLauncherVersion = j["minimumLauncherVersion"].get<int>();
    if (j.contains("releaseTime")) vi.releaseTime = j["releaseTime"].get<std::string>();
    if (j.contains("time")) vi.time = j["time"].get<std::string>();
    if (j.contains("type")) vi.type = j["type"].get<std::string>();

    return vi;
}
