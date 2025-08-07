#include "include/services.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

#define DESTINATION "org.freedesktop.systemd1"
#define PATH        "/org/freedesktop/systemd1"
#define INTERFACE   "org.freedesktop.systemd1.Manager"


int start_service(const std::string& unit_name){
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        std::cerr << "Failed to connect to system bus: " << strerror(-r) << std::endl;
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "StartUnit",
                           &error,
                           &reply,
                           "ss",
                           unit_name.c_str(),
                           "replace");

    if (r < 0) {
        std::cerr << "Failed to start unit file: " << error.message << std::endl;
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    // Clean up
    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    std::cout << "Successfully disabled unit file: " << unit_name << std::endl;
    return 0;

}

int stop_service(const std::string& unit_name){
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        std::cerr << "Failed to connect to system bus: " << strerror(-r) << std::endl;
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "StopUnit",
                           &error,
                           &reply,
                           "ss",
                           unit_name.c_str(),
                           "replace");
    if (r < 0) {
        std::cerr << "Failed to disable unit file: " << error.message << std::endl;
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    // Clean up
    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    std::cout << "Successfully disabled unit file: " << unit_name << std::endl;
    return 0;

}
int enable_service(const std::string& unit_name) {
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        std::cerr << "Failed to connect to system bus: " << strerror(-r) << std::endl;
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "EnableUnitFiles",
                           &error,
                           &reply,
                           "asbb",
                           1,                    // Number of unit files
                           unit_name.c_str(),     // Unit name
                           false,                 // Runtime (false = permanent)
                           true);                 // Force (replace symlinks)

    if (r < 0) {
        std::cerr << "Failed to enable unit file: " << error.message << std::endl;
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    // Reload systemd to apply changes
    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "Reload",
                           &error,
                           nullptr,
                           "");

    if (r < 0) {
        std::cerr << "Warning: Failed to reload systemd: " << error.message << std::endl;
    }

    sd_bus_error_free(&error);
    if (reply) sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    std::cout << "Successfully enabled unit file: " << unit_name << std::endl;
    return 0;
}

int disable_service(const std::string &unit_name) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    sd_bus *bus = nullptr;
    int r;

    r = sd_bus_open_system(&bus);
    if (r < 0) {
        std::cerr << "Failed to connect to system bus: " << strerror(-r) << std::endl;
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "DisableUnitFiles",
                           &error,
                           &reply,
                           "asb",
                           1,                    // Number of unit files
                           unit_name.c_str(),    // Unit name
                           false);               // Runtime (false = permanent)

    if (r < 0) {
        std::cerr << "Failed to disable unit file: " << error.message << std::endl;
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    // Reload systemd to apply changes
    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "Reload",
                           &error,
                           nullptr,
                           "");

    if (r < 0) {
        std::cerr << "Warning: Failed to reload systemd: " << error.message << std::endl;
    }

    sd_bus_error_free(&error);
    if (reply) sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    std::cout << "Successfully disabled unit file: " << unit_name << std::endl;
    return 0;
}

std::vector<ServiceInfo> get_services() {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    sd_bus *bus = nullptr;
    std::vector<ServiceInfo> services;

    int r = sd_bus_open_system(&bus);
    if (r < 0) {
        std::cerr << "Failed to connect to system bus: " << strerror(-r) << std::endl;
        return services;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "ListUnits",
                           &error,
                           &reply,
                           "");
    const char *signature = sd_bus_message_get_signature(reply, true);
    std::cerr << "Message signature: " << (signature ? signature : "NULL") << std::endl;
    if (r < 0) {
        std::cerr << "Failed to call ListUnits: " << error.message << std::endl;
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return services;
    }

    r = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
    if (r < 0) {
        std::cerr << "Failed to enter array container: " << strerror(-r) << std::endl;
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return services;
    }

    while ((r = sd_bus_message_enter_container(reply, 'r', "ssssssouso")) > 0) {
        ServiceInfo service;
        const char *name = nullptr;
        const char *description = nullptr;
        const char *active_state = nullptr;

        r = sd_bus_message_read(reply, "ssssssouso",
                                &name, &description, nullptr,
                                &active_state, nullptr, nullptr,
                                nullptr, nullptr, nullptr, nullptr);
        if (r < 0) {
            std::cerr << "Failed to read message: " << strerror(-r) << std::endl;
            sd_bus_message_exit_container(reply);
            break;
        }

        if (name && strstr(name, ".service") != nullptr) {
            service.name = name;
            service.description = description ? description : "N/A";
            service.status = active_state ? active_state : "N/A";
            services.push_back(service);
        }

        sd_bus_message_exit_container(reply);
    }

    sd_bus_message_exit_container(reply);

    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return services;
}
/*
int main() {
    if (geteuid() != 0) {
        std::cout << "This program requires root-priveleges. restart it using sudo/doas <executable-name>\n";
        std::cout << "Or use sudo/doas !!\n";
        return -1;
    }
    std::vector<ServiceInfo> services = get_services();
    for (const auto& service : services) {
        std::cout << "• " << service.name << " - " << service.description
                  << " (" << service.status << ")\n";
    }

    enable_service("dummy.service");
    return 0;
}
*/
