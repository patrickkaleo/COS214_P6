#ifndef ROUTINGSTRATEGY_H
#define ROUTINGSTRATEGY_H

/**
 * @file RoutingStrategy.h
 * @brief Abstract Strategy interface for task assignment in the TIVIDY engine.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <memory>

// Forward declarations to decouple strategy interface from concrete context and directory classes
class Task;
class InstanceData;
class ParticipantDirectory;
class Participant;

/**
 * @class RoutingStrategy
 * @brief Abstract base strategy interface defining the task allocation contract.
 *
 * Implements the GoF Strategy (Behavioural) pattern. In the TIVIDY workflow management
 * system, tasks must be allocated to human participants or automated services dynamically.
 * Instead of hardcoding assignment rules inside the Task class, Task delegates assignment
 * to a RoutingStrategy implementation.
 *
 * Employs modern C++11 smart pointers:
 * - `select()` returns `std::shared_ptr<Participant>` for safe, flyweight reference sharing.
 * - `clone()` returns `std::unique_ptr<RoutingStrategy>` enforcing clear ownership semantics.
 *
 * @see RoleBasedRouting
 * @see DataRuleRouting
 * @see ApprovalRouting
 */
class RoutingStrategy {
public:
    /**
     * @brief Virtual destructor ensuring clean disposal of concrete strategy subclasses.
     */
    virtual ~RoutingStrategy() = default;

    /**
     * @brief Selects an eligible participant to be assigned to the given task.
     *
     * @param task The task requesting routing and assignment.
     * @param data The runtime instance payload containing form/process attributes.
     * @param directory The organization directory containing active participants and roles.
     * @return Participant* Pointer to the chosen participant, or nullptr if no candidate found.
     */
    virtual Participant* select(const Task& task,
                                const InstanceData& data,
                                const ParticipantDirectory& directory) = 0;

    /**
     * @brief Creates a deep polymorphic copy of this strategy instance (Prototype pattern).
     *
     * Enables workflow definitions and task templates to be cloned when new workflow
     * instances are spawned.
     *
     * @return RoutingStrategy* Newly allocated clone of this routing strategy (caller owns).
     */
    virtual RoutingStrategy* clone() const = 0;
};

#endif // ROUTINGSTRATEGY_H
