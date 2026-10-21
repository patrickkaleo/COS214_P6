/**
 * @file ServiceRegistry.cpp
 * @brief Implementation of the external-service adapter registry.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "ServiceRegistry.h"
#include "ExternalService.h"

ServiceRegistry::~ServiceRegistry() {
    for (std::map<std::string, ExternalService*>::iterator it = services.begin(); it != services.end(); ++it) {
        delete it->second;
    }
}

void ServiceRegistry::add(ExternalService* service) {
    if (service == nullptr) {
        return;
    }

    std::string key = service->name();
    std::map<std::string, ExternalService*>::iterator existing = services.find(key);
    if (existing != services.end()) {
        if (existing->second == service) {
            return;
        }
        delete existing->second;
    }
    services[key] = service;
}

ExternalService* ServiceRegistry::find(const std::string& name) const {
    std::map<std::string, ExternalService*>::const_iterator it = services.find(name);
    return it == services.end() ? nullptr : it->second;
}

bool ServiceRegistry::has(const std::string& name) const {
    return services.find(name) != services.end();
}

size_t ServiceRegistry::size() const {
    return services.size();
}
