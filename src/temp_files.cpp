#include "include/temp_files.hpp"


std::expected<std::vector<std::filesystem::path>, std::string> get_recursive_folder_content(const std::filesystem::path& folder_path) {
    std::vector<std::filesystem::path> result;

    if (!std::filesystem::exists(folder_path)) {
        logE(std::format("Folder does not exist: {}", folder_path.string()));
        return std::unexpected("Folder does not exist");
    }
    if (!std::filesystem::is_directory(folder_path)) {
        logE(std::format("Path is not a directory: {}", folder_path.string()));
        return std::unexpected("Path is not a directory");
    }
    if (std::filesystem::is_empty(folder_path)) {
        logL(std::format("Temp files: Folder is empty: {}", folder_path.string()));
        return std::unexpected("Folder is empty");
    }

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
            result.push_back(entry.path());
        }
        logL(std::format("Temp files: Found {} files in {}", result.size(), folder_path.string()));
    } catch (const std::filesystem::filesystem_error& e) {
        logE(std::format("Filesystem error in {}: {}", folder_path.string(), e.what()));
        return std::unexpected(std::string("Filesystem error: ") + e.what());
    }

    return result;
}

std::filesystem::path get_home_directory() {
    if (const char* sudo_user = std::getenv("SUDO_USER")) {
        if (struct passwd* pw = getpwnam(sudo_user)) {
            logL(std::format("Temp files: Using home directory of SUDO_USER: {}", pw->pw_dir));
            return pw->pw_dir;
        }
    }

    if (const char* home = std::getenv("HOME")) {
        logL(std::format("Temp files: Using HOME environment variable: {}", home));
        return home;
    }

    if (struct passwd* pw = getpwuid(getuid())) {
        logL(std::format("Temp files: Using home directory from getpwuid: {}", pw->pw_dir));
        return pw->pw_dir;
    }

    logF("Failed to determine home directory");
    throw std::runtime_error("Failed to determine home directory");
}
