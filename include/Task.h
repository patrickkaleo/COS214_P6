/**
 * @file Task.h
 * @brief Composite pattern for the TIVIDY generic workflow engine (Leaf Participant).
 *
 * WorkItem  – abstract Component
 * Task      – Leaf
 * SubProcess – Composite
 */

#ifndef TASK_H
#define TASK_H

#include "WorkItem.h"
class RoutingStrategy;
class Participant;
class InstanceContext;

using namespace std;

/**
 * @brief Leaf of the Composite.  Represents an atomic work item.
 */
class Task : public WorkItem {
public:
    Task(const string& id, const string& name);
    ~Task() override;

    void onAvailable() override;
    WorkItem* clone() const override;

    // Task-specific
    void setRouting(RoutingStrategy* r);// takes ownership
    RoutingStrategy* getRouting() const { return routing_; }
    void setServiceName(const string& s) { serviceName_ = s; }
    string getServiceName() const { return serviceName_; }
    void setCheckpoint(bool c) { checkpoint_ = c; }
    bool isCheckpoint() const { return checkpoint_; }
    void setEscalationLevel(int l) { escalationLevel_ = l; }
    int getEscalationLevel() const { return escalationLevel_; }

    Participant* getAssignee() const { return assignee_; }
    void setAssignee(Participant* p) { assignee_ = p; } // not owned

    /**
     * @brief Perform the actual work of the task (may call an ExternalService).
     * Real implementation will use InstanceContext.
     */
    virtual void performWork(InstanceContext& context);

private:
    Participant* assignee_;// not owned
    RoutingStrategy* routing_;// owned
    string serviceName_;
    bool checkpoint_;
    int escalationLevel_;
};

#endif
