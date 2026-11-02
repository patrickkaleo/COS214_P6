/**
 * @file ServiceRegistry.cpp
 * @brief Implementation of the external-service adapter registry.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "ServiceRegistry.h"

void ServiceRegistry::add(ExternalService* service) {
    if (service == nullptr) {
        return;
    }

    // Resetting the slot deletes any adapter previously registered under this name
    std::unique_ptr<ExternalService>& slot = services[service->name()];
    if (slot.get() != service) {
        slot.reset(service);
    }
}

ExternalService* ServiceRegistry::find(const std::string& name) const {
    std::map<std::string, std::unique_ptr<ExternalService>>::const_iterator it = services.find(name);
    return it == services.end() ? nullptr : it->second.get();
}

bool ServiceRegistry::has(const std::string& name) const {
    return services.find(name) != services.end();
}

size_t ServiceRegistry::size() const {
    return services.size();
}
