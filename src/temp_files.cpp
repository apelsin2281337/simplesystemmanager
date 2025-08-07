#include "include/temp_files.hpp"


std::expected<std::vector<std::filesystem::path>, std::string> get_recursive_folder_content(const std::filesystem::path& folder_path) {
    std::vector<std::filesystem::path> result;

    if (!std::filesystem::exists(folder_path)) {
        return std::unexpected("Folder does not exist");
    }
    if (!std::filesystem::is_directory(folder_path)) {
        return std::unexpected("Path is not a directory");
    }
    if (std::filesystem::is_empty(folder_path)) {
        return std::unexpected("Folder is empty");
    }

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
            result.push_back(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        return std::unexpected(std::string("Filesystem error: ") + e.what());
    }

    return result;
}

std::filesystem::path get_home_directory() {
    if (const char* sudo_user = std::getenv("SUDO_USER")) {
        if (struct passwd* pw = getpwnam(sudo_user)) {
            return pw->pw_dir;
        }
    }

    if (const char* home = std::getenv("HOME")) {
        return home;
    }

    if (struct passwd* pw = getpwuid(getuid())) {
        return pw->pw_dir;
    }

    throw std::runtime_error("Failed to determine home directory");
}

