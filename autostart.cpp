    #include "autostart.hpp"




bool AutostartManager::addAutostartEntry(const std::string& name, const std::string& exec, const std::string& comment, bool isEnabled){
    std::filesystem::path autostart_dir_= "/etc/xdg/autostart";
    std::filesystem::path dotdesktopFilePath = autostart_dir_ / (name + ".desktop");
    std::ofstream file(dotdesktopFilePath);
    if (!file.is_open()){
        throw std::runtime_error("well that didnt work");
        return false;
    }
    
    file << "[Desktop Entry]\n"
    << "Type=Application\n"
    << "Name=" << name << "\n"
    << "Comment=" << comment << "\n"
    << "Exec=" << exec << "\n"
    << "X-GNOME-Autostart-enabled=" << (isEnabled ? "true" : "false") << "\n";
    return  true;
    file.close();
    }

bool AutostartManager::removeAutostartEntry(const std::string& name){
        std::filesystem::path autostart_dir_= "/etc/xdg/autostart";
        std::filesystem::path desktop_file = autostart_dir_ / (name + ".desktop");
        if (!std::filesystem::exists(desktop_file)) {
            return false;
        }
        
        try {
            return std::filesystem::remove(desktop_file);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error removing file (may need root privileges): " << e.what() << "\n";
            return false;
        }
    }

bool AutostartManager::setAutostartEntryEnabledStatus(const std::string& name, bool status){
    std::filesystem::path autostart_dir_= "/etc/xdg/autostart";
    std::filesystem::path desktop_file = autostart_dir_ / (name + ".desktop");
    if (!std::filesystem::exists(desktop_file)) {
        return false;
    }
    std::ifstream file(desktop_file);
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    bool found = false;
    for (auto& l : lines) {
        if (l.find("X-GNOME-Autostart-enabled=") == 0) {
            l = "X-GNOME-Autostart-enabled=" + std::string(status ? "true" : "false");
            found = true;
            break;
        }
    }

    if (!found) {

        lines.push_back("X-GNOME-Autostart-enabled=" + std::string(status ? "true" : "false"));
    }

    std::ofstream out_file(desktop_file);
    if (!out_file.is_open()) {
        std::cerr << "well that didnt work\n";
        return false;
    }

    for (const auto& l : lines) {
        out_file << l << "\n";
    }
    out_file.close();
    return true;
}

std::vector<std::string> AutostartManager::listAutostartEntries(){
    std::filesystem::path autostart_dir_= "/etc/xdg/autostart";
    std::vector<std::string> entries;

    for (const auto& entry : std::filesystem::directory_iterator(autostart_dir_)) {
        if (entry.path().extension() == ".desktop") {
            entries.push_back(entry.path().stem().string());
        }
    }
    return entries;
}

std::unordered_map<std::string, std::string> AutostartManager::getAutostartEntryInfo(const std::string& name){
    std::filesystem::path autostart_dir_= "/etc/xdg/autostart";
    std::unordered_map<std::string, std::string> entryInfo;
    std::filesystem::path dotDesktopFile = autostart_dir_ / (name + ".desktop");

    if (!std::filesystem::exists(dotDesktopFile)) {
        std::cerr << "Error: File does not exist: " << dotDesktopFile << "\n";
        return entryInfo;
    }

    std::ifstream file(dotDesktopFile);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << dotDesktopFile << "\n";
        return entryInfo;
    }

    std::regex namePattern(R"(Name\s*=\s*(?:")?([^"\n]*)(?:")?)");
    std::regex execPattern(R"(Exec\s*=\s*(?:")?([^"\n]*)(?:")?)");
    std::regex commentPattern(R"(Comment\s*=\s*(?:")?([^"\n]*)(?:")?)");
    std::regex statusPattern(R"(X-GNOME-Autostart-enabled\s*=\s*(?:")?([^"\n]*)(?:")?)");

    std::string line;
    while (std::getline(file, line)) {
        std::smatch matches;
        if (std::regex_search(line, matches, namePattern)) {
            entryInfo["Name"] = matches[1];
        }
        else if (std::regex_search(line, matches, execPattern)) {
            entryInfo["Exec"] = matches[1];
        }
        else if (std::regex_search(line, matches, commentPattern)) {
            entryInfo["Comment"] = matches[1];
        }
        else if (std::regex_search(line, matches, statusPattern)) {
            entryInfo["Status"] = matches[1];
        }
    }

    return entryInfo;
}
