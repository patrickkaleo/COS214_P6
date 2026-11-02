#ifndef ROLEBASEDROUTING_H
#define ROLEBASEDROUTING_H

/**
 * @file RoleBasedRouting.h
 * @brief Concrete strategy for assigning tasks based on a participant's role.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "RoutingStrategy.h"
#include <string>
#include <memory>

/**
 * @class RoleBasedRouting
 * @brief Allocates work to the first available participant holding a designated role.
 *
 * In the TIVIDY system, many standard tasks (e.g. "Triage Nurse", "Admissions Administrator")
 * are not tied to an individual person, but to a role qualification. RoleBasedRouting
 * queries the ParticipantDirectory for participants matching the configured role and
 * assigns the task accordingly.
 */
class RoleBasedRouting : public RoutingStrategy {
private:
    std::string role; ///< Target qualification/role name required for this task.

public:
    /**
     * @brief Default constructor. Initializes with an empty role.
     */
    RoleBasedRouting();

    /**
     * @brief Parameterized constructor setting the required role name.
     * @param roleName The name of the role (e.g., "Admissions Administrator", "Triage Nurse").
     */
    explicit RoleBasedRouting(const std::string& roleName);

    /**
     * @brief Virtual destructor.
     */
    virtual ~RoleBasedRouting() override = default;

    /**
     * @brief Selects the first available participant possessing the required role.
     *
     * Queries @p directory for all participants qualified with @c role. Returns the first
     * matching participant, or @c nullptr if none are currently registered.
     *
     * @param task The task requesting an assignee.
     * @param data Instance runtime data (unused by basic role routing).
     * @param directory The participant directory to search.
     * @return std::shared_ptr<Participant> Shared pointer to the assigned participant, or nullptr.
     */
    virtual Participant* select(const Task& task,
                                const InstanceData& data,
                                const ParticipantDirectory& directory) override;

    /**
     * @brief Clones this strategy instance for workflow instantiation.
     * @return RoutingStrategy* Newly allocated clone of this strategy.
     */
    virtual RoutingStrategy* clone() const override;

    std::string getRole() const;
    void setRole(const std::string& roleName);
};

#endif // ROLEBASEDROUTING_H
