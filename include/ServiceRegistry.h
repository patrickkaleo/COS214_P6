#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

/**
 * @file ServiceRegistry.h
 * @brief Owns the external-service adapters and finds them by name.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include <map>

class ExternalService;

/**
 * @class ServiceRegistry
 * @brief Name-to-adapter lookup shared by every workflow instance.
 *
 * Definitions refer to integrations by name only, so a task asks the registry for its
 * configured service at run time. Each industry registers its own adapters; the engine
 * code is identical for both.
 */
class ServiceRegistry {
private:
    std::map<std::string, ExternalService*> services; ///< Owned adapters keyed by name().

public:
    ServiceRegistry() = default;
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    /// Deletes every registered adapter.
    ~ServiceRegistry();

    /**
     * @brief Registers an adapter under its name(), taking ownership.
     *
     * An adapter already registered under the same name is deleted and replaced, which
     * lets tests swap a real adapter for a stub.
     *
     * @param service Newly allocated adapter; ignored if nullptr.
     */
    void add(ExternalService* service);

    /**
     * @brief Finds an adapter by name.
     * @param name Service name from the workflow definition.
     * @return ExternalService* The adapter, or nullptr if none is registered.
     */
    ExternalService* find(const std::string& name) const;

    /// @return true if a service with @p name is registered.
    bool has(const std::string& name) const;

    /// @return Number of registered services.
    size_t size() const;
};

#endif // SERVICEREGISTRY_H
