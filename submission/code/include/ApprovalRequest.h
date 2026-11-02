#ifndef APPROVALREQUEST_H
#define APPROVALREQUEST_H

/**
 * @file ApprovalRequest.h
 * @brief Context payload passed along the ApprovalHandler Chain of Responsibility.
 * @author Enock Nyamweya
 * @version 2.0
 */

class Task;
class InstanceData;
class ParticipantDirectory;

class ApprovalRequest {
public:
    const Task* item;
    const InstanceData* data;
    int level;
    const ParticipantDirectory* directory;
    int deadlineHours; ///< Output: deadline of the handler that accepted the request.
    int selectedLevel; ///< Output: selected handler index relative to the chain head.

    ApprovalRequest()
        : item(nullptr), data(nullptr), level(0), directory(nullptr), deadlineHours(0), selectedLevel(-1) {}

    ApprovalRequest(const Task* t, const InstanceData* d, int lvl = 0, const ParticipantDirectory* dir = nullptr)
        : item(t), data(d), level(lvl), directory(dir), deadlineHours(0), selectedLevel(-1) {}
};

#endif // APPROVALREQUEST_H
