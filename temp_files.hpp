#pragma once
#include <iostream>
#include <filesystem>
#include <vector>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#include <expected>
#include <QTranslator>
#include "mainwindow.h"

std::expected<std::vector<std::filesystem::path>, std::string> get_recursive_folder_content(const std::filesystem::path& folder_path);
std::filesystem::path get_home_directory();
