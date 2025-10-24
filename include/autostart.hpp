#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <regex>


#include "../include/logger.hpp"

class AutostartManager{
public:
    AutostartManager();
    static bool addAutostartEntry(const std::string& name, const std::string& exec, const std::string& filename = "", const std::string& comment = "default one", bool isEnabled = true);
    static bool removeAutostartEntry(const std::string& filename);
    static bool setAutostartEntryEnabledStatus(const std::string& filename, bool status = true);
    static std::vector<std::string> listAutostartEntries();
    static std::unordered_map<std::string, std::string> getAutostartEntryInfo(const std::string& filename);

    
};
