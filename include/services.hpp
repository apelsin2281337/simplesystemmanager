#pragma once
#include <systemd/sd-bus.h>
#include <iostream>
#include <vector>

struct ServiceInfo{
    std::string name;
    std::string description;
    std::string status;
};

std::vector<ServiceInfo> get_services();
int execute_service_operation(const char* method, const char* unit_name);

int start_service(const std::string& unit_name);
int stop_service(const std::string& unit_name);
int enable_service(const std::string& unit_name);
int disable_service(const std::string &unit_name);
