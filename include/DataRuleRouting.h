#ifndef DATARULEROUTING_H
#define DATARULEROUTING_H

/**
 * @file DataRuleRouting.h
 * @brief Concrete strategy routing tasks dynamically based on workflow instance data.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "RoutingStrategy.h"
#include <string>
#include <map>

/**
 * @class DataRuleRouting
 * @brief Dynamic routing strategy that selects a role based on process form values.
 *
 * Inspects a specified field in the runtime InstanceData (e.g., "severity", "incomeBracket").
 * Maps the extracted value to a target role name using an internal lookup table.
 * If the field is missing or has an unmapped value, it falls back to a default fallback role.
 * Once the role is determined, it queries the ParticipantDirectory for a suitable participant.
 */
class DataRuleRouting : public RoutingStrategy {
private:
    std::string field;                            ///< Form field key to inspect (e.g. "severity").
    std::map<std::string, std::string> valueToRole; ///< Mapping of field values to role names.
    std::string fallbackRole;                     ///< Default role if no mapping matches.

public:
    /**
     * @brief Default constructor.
     */
    DataRuleRouting();

    /**
     * @brief Parameterized constructor.
     * @param fieldName The data field key to evaluate.
     * @param mapping Mapping of specific field values to role strings.
     * @param defaultFallbackRole Fallback role when no rule matches or field is absent.
     */
    DataRuleRouting(const std::string& fieldName,
                    const std::map<std::string, std::string>& mapping,
                    const std::string& defaultFallbackRole = "");

    /**
     * @brief Virtual destructor.
     */
    virtual ~DataRuleRouting() override = default;

    /**
     * @brief Evaluates instance data and assigns a matching participant.
     *
     * Reads @p data.get(field). Looks up the role in @c valueToRole. If found, retrieves
     * a participant with that role from @p directory. If not found or if the field is absent,
     * attempts to find a participant with @c fallbackRole.
     *
     * @param task The task being routed.
     * @param data The workflow instance payload containing form attributes.
     * @param directory The organization participant directory.
     * @return Participant* Pointer to the selected participant, or nullptr if none available.
     */
    virtual Participant* select(const Task& task,
                                const InstanceData& data,
                                const ParticipantDirectory& directory) override;

    /**
     * @brief Clones this strategy instance for workflow template copying.
     * @return RoutingStrategy* Newly allocated copy of this strategy.
     */
    virtual RoutingStrategy* clone() const override;

    /**
     * @brief Adds or updates a rule mapping a data value to a target role.
     * @param value The expected form value (e.g., "high", "critical").
     * @param roleName The role to assign for this value (e.g., "Emergency Doctor").
     */
    void addRule(const std::string& value, const std::string& roleName);

    /**
     * @brief Gets the inspected data field name.
     * @return std::string Field name.
     */
    std::string getField() const;

    /**
     * @brief Sets the data field name to inspect.
     * @param fieldName Field name.
     */
    void setField(const std::string& fieldName);

    /**
     * @brief Gets the fallback role.
     * @return std::string Fallback role.
     */
    std::string getFallbackRole() const;

    /**
     * @brief Sets the fallback role.
     * @param defaultFallbackRole Fallback role name.
     */
    void setFallbackRole(const std::string& defaultFallbackRole);

    /**
     * @brief Returns the complete value-to-role lookup map.
     * @return const std::map<std::string, std::string>& Map reference.
     */
    const std::map<std::string, std::string>& getRuleMap() const;
};

#endif // DATARULEROUTING_H
