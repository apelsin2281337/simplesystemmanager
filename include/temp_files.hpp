#pragma once
#include <expected>
#include <vector>
#include <filesystem>
#include <string>

std::expected<std::vector<std::filesystem::path>, std::string>
get_recursive_folder_content(const std::filesystem::path& folder_path);

std::filesystem::path get_home_directory();
