#include "../include/autostart.hpp"

std::filesystem::path autostart_dir_= "/etc/xdg/autostart";


bool AutostartManager::addAutostartEntry(const std::string& name, const std::string& exec, const std::string& comment, bool isEnabled){


    std::filesystem::path dotdesktopFilePath = autostart_dir_ / (name + ".desktop");
    std::ofstream file(dotdesktopFilePath);
    if (!file.is_open()){
        logE(std::format("Autostart: File {0} did not open successfully. it probably does not exist", dotdesktopFilePath.string()));
        throw std::runtime_error(std::format("File {0} did not open successfully. it probably does not exist", dotdesktopFilePath.string()));
        return false;

    }
    
    file << "[Desktop Entry]\n"
    << "Type=Application\n"
    << "Name=" << name << "\n"
    << "Comment=" << comment << "\n"
    << "Exec=" << exec << "\n"
    << "X-GNOME-Autostart-enabled=" << (isEnabled ? "true" : "false") << "\n";
    file.close();
    logL(std::format("Autostart: File {0} has been added successfully", dotdesktopFilePath.string()));
    return true;
}

bool AutostartManager::removeAutostartEntry(const std::string& name){
    std::filesystem::path desktop_file = autostart_dir_ / (name + ".desktop");
    if (!std::filesystem::exists(desktop_file)) {
        logE(std::format("Autostart: File {0} did not open successfully. it probably does not exist", desktop_file.string()));
        return false;

    }

    try {
        logL(std::format("Autostart: Trying to remove file {0}", desktop_file.string()));
        return std::filesystem::remove(desktop_file);
        logL("Success!");
    } catch (const std::filesystem::filesystem_error& e) {
        logE(std::format("Autostart: File {0} removal failed", desktop_file.string()));
        return false;
    }
}

bool AutostartManager::setAutostartEntryEnabledStatus(const std::string& name, bool status){
    std::filesystem::path desktop_file = autostart_dir_ / (name + ".desktop");
    if (!std::filesystem::exists(desktop_file)) {
        logE(std::format("Autostart: File {0} did not open successfully. it probably does not exist", desktop_file.string()));
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
        logE(std::format("Autostart: File {0} did not open successfully. it probably does not exist", desktop_file.string()));
        return false;
    }

    for (const auto& l : lines) {
        out_file << l << "\n";
    }
    logL(std::format("Autostart: Status {0} has been set for file {1}", status, desktop_file.string()));
    out_file.close();
    return true;
}

std::vector<std::string> AutostartManager::listAutostartEntries(){
    std::vector<std::string> entries;
    size_t i = 0;
    for (const auto& entry : std::filesystem::directory_iterator(autostart_dir_)) {
        if (entry.path().extension() == ".desktop") {
            std::string entryPath = entry.path().stem().string();
            entries.push_back(entryPath);
            i++;
        }
    }
    logL(std::format("Autostart: {0} entries has been found", i));
    return entries;
}

std::unordered_map<std::string, std::string> AutostartManager::getAutostartEntryInfo(const std::string& name){
    std::unordered_map<std::string, std::string> entryInfo;
    std::filesystem::path dotDesktopFile = autostart_dir_ / (name + ".desktop");

    if (!std::filesystem::exists(dotDesktopFile)) {
        logE(std::format("Autostart: {0} does not exist", dotDesktopFile.string()));
        return entryInfo;
    }

    std::ifstream file(dotDesktopFile);
    if (!file.is_open()) {
        logE(std::format("Autostart: Failed to open {0}", dotDesktopFile.string()));
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
