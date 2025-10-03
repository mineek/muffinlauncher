#pragma once

#include <string>

bool downloadFileEasy(const std::string& url, std::string& outContent, bool quiet = false, bool cache = false);
