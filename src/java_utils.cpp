#include "java_utils.h"
#include <iostream>
#include <cstdio>
#include <string>

// frcoal
bool verifyJavaVersion(int requiredVersion) {
    FILE* pipe = popen("java -version 2>&1", "r");
    if (!pipe) {
        std::cerr << "Failed to check Java version" << std::endl;
        return false;
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    size_t pos = result.find("version \"");
    if (pos == std::string::npos) {
        std::cerr << "Failed to parse Java version" << std::endl;
        return false;
    }

    pos += 9;
    size_t endPos = result.find("\"", pos);
    if (endPos == std::string::npos) {
        std::cerr << "Failed to parse Java version" << std::endl;
        return false;
    }

    std::string versionStr = result.substr(pos, endPos - pos);
    int majorVersion = 0;
    if (versionStr[0] == '1' && versionStr[1] == '.') {
        majorVersion = std::stoi(versionStr.substr(2, versionStr.find('.', 2) - 2));
    } else {
        majorVersion = std::stoi(versionStr.substr(0, versionStr.find('.')));
    }
    std::cout << "Detected current Java version " << majorVersion << std::endl;
    if (majorVersion != requiredVersion) {
        std::cerr << "Java " << requiredVersion << " is required, but found " << majorVersion << std::endl;
        return false;
    }
    return true;
}
