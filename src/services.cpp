#include "../include/services.hpp"
#include "../include/logger.hpp"

#define DESTINATION "org.freedesktop.systemd1"
#define PATH        "/org/freedesktop/systemd1"
#define INTERFACE   "org.freedesktop.systemd1.Manager"

int start_service(const std::string& unit_name) {
    logL(std::format("Services: Services:  Attempting to start service: {}", unit_name));
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {}", strerror(-r)));
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
        logE(std::format("Services: Failed to start service {}: {}", unit_name, error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    logL(std::format("Services: Successfully started service: {}", unit_name));
    return 0;
}

int stop_service(const std::string& unit_name) {
    logL(std::format("Services: Attempting to stop service: {}", unit_name));
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {}", strerror(-r)));
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
        logE(std::format("Services: Failed to stop service {}: {}", unit_name, error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    logL(std::format("Services: Successfully stopped service: {}", unit_name));
    return 0;
}

int enable_service(const std::string& unit_name) {
    logL(std::format("Services: Attempting to enable service: {}", unit_name));
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {}", strerror(-r)));
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
                           1,
                           unit_name.c_str(),
                           false,
                           true);

    if (r < 0) {
        logE(std::format("Services: Failed to enable service {}: {}", unit_name, error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "Reload",
                           &error,
                           nullptr,
                           "");

    if (r < 0) {
        logE(std::format("Services: Warning: Failed to reload systemd: {}", error.message));
    }

    sd_bus_error_free(&error);
    if (reply) sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    logL(std::format("Services: Successfully enabled service: {}", unit_name));
    return 0;
}

int disable_service(const std::string &unit_name) {
    logL(std::format("Services: Attempting to disable service: {}", unit_name));
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    sd_bus *bus = nullptr;
    int r;

    r = sd_bus_open_system(&bus);
    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {}", strerror(-r)));
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
                           1,
                           unit_name.c_str(),
                           false);

    if (r < 0) {
        logE(std::format("Services: Failed to disable service {}: {}", unit_name, error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return r;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "Reload",
                           &error,
                           nullptr,
                           "");

    if (r < 0) {
        logE(std::format("Services: Warning: Failed to reload systemd: {}", error.message));
    }

    sd_bus_error_free(&error);
    if (reply) sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    logL(std::format("Services: Successfully disabled service: {}", unit_name));
    return 0;
}

std::vector<ServiceInfo> get_services() {
    logL("Fetching list of services");
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    sd_bus *bus = nullptr;
    std::vector<ServiceInfo> services;

    int r = sd_bus_open_system(&bus);
    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {}", strerror(-r)));
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

    if (r < 0) {
        logE(std::format("Services: Failed to call ListUnits: {}", error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return services;
    }

    r = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
    if (r < 0) {
        logE(std::format("Services: Failed to enter array container: {}", strerror(-r)));
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
            logE(std::format("Services: Failed to read message: {}", strerror(-r)));
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

    logL(std::format("Services: Services: Found {0} services", services.size()));
    return services;
}

bool is_service_enabled(const std::string& unit_name) {
    sd_bus *bus = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_open_system(&bus);

    if (r < 0) {
        logE(std::format("Services: Failed to connect to system bus: {0}", strerror(-r)));
        return false;
    }

    r = sd_bus_call_method(bus,
                           DESTINATION,
                           PATH,
                           INTERFACE,
                           "GetUnitFileState",
                           &error,
                           &reply,
                           "s",
                           unit_name.c_str());

    if (r < 0) {
        logE(std::format("Services: Failed to get service {0} state: {1}", unit_name, error.message));
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    const char *state = nullptr;
    r = sd_bus_message_read(reply, "s", &state);
    if (r < 0) {
        logE(std::format("Services: Failed to parse service {0} state: {1}", unit_name, strerror(-r)));
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return false;
    }

    bool is_active = (strcmp(state, "enabled") == 0 || strcmp(state, "static") == 0);

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    logL(std::format("Service {0} is enabled", unit_name));
    return is_active;
}
