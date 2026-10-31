#ifndef WORKITEMINFO_H
#define WORKITEMINFO_H

/**
 * @file WorkItemInfo.h
 * @brief Read-only summary of one work item, handed out by the facade.
 */

#include <string>
#include <vector>

/**
 * @struct WorkItemInfo
 * @brief What a client may see of a work item without holding a pointer into the engine.
 */
struct WorkItemInfo {
    std::string id;                    ///< Work item id.
    std::string name;                  ///< Display name.
    std::string state;                 ///< Current lifecycle state.
    std::string assigneeId;            ///< Assigned participant's id, or empty.
    std::string assigneeName;          ///< Assigned participant's name, or empty.
    std::string outcome;               ///< Outcome or reason, or empty.
    std::string serviceName;           ///< External service this item calls, or empty.
    std::string serviceMessage;        ///< Message from the last service call, or empty.
    std::vector<std::string> outcomes; ///< Outcomes the definition branches on when this item completes.
    int escalationLevel = 0;           ///< Times this item has been escalated.
    int priority = 0;                  ///< Level set by a PriorityDecorator; 0 if none.
    int deadlineHours = 0;             ///< Hours allowed before escalating; 0 if none.
    int depth = 0;                     ///< Distance from the root of the instance's tree.
    bool composite = false;            ///< true for a sub-process, false for a task.
};

#endif // WORKITEMINFO_H
