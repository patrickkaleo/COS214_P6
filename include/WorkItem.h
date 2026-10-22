/**
 * @file WorkItem.h
 * @brief Composite pattern for the TIVIDY generic workflow engine (abstract Component Participant).
 *
 * WorkItem  – abstract Component
 * Task      – Leaf
 * SubProcess – Composite
 */

#ifndef WORKITEM_H
#define WORKITEM_H

class Participant;
class WorkItemState;
class SubProcess;
class WorkflowMediator;
class WorkflowEventListener;
class WorkflowEvent;

using namespace std;

#include <string>
#include <vector>

/**
 * @brief Abstract Component of the Composite pattern.
 *
 * Every piece of work (leaf task or composite sub-process) is a WorkItem.
 * Lifecycle operations are forwarded to the current WorkItemState object
 * (State pattern).  The class is also the Subject of Observer and a
 * Colleague of the Mediator.
 */
class WorkItem {
public:
    WorkItem(const string& id, const string& name);
    virtual ~WorkItem();

    // lifecycle (delegate to state)
    virtual bool makeAvailable();
    virtual bool assign(Participant* p);
    virtual bool start();
    virtual bool complete(const string& outcome);
    virtual bool reject(const string& reason);
    virtual bool cancel();
    virtual bool escalate();

    // state management
    void setState(WorkItemState* s);
    WorkItemState* getState() const { return state_; }
    bool isFinished() const;

    // identity / data
    string getId() const { return id_; }
    string getName() const { return name_; }
    string getOutcome() const { return outcome_; }
    void setOutcome(const string& o) { outcome_ = o; }

    // structure
    SubProcess* getParent() const { return parent_; }
    void setParent(SubProcess* p) { parent_ = p; }

    // Observer support
    void attach(WorkflowEventListener* l);
    void detach(WorkflowEventListener* l);
	void notify(const WorkflowEvent& event);
    //void notify(const string& eventType);
	//void notifyListeners(workFlowEvent event); - From generated .h

    // Mediator
    void setMediator(WorkflowMediator* m) { mediator_ = m; }
    WorkflowMediator* getMediator() const { return mediator_; }

    // Prototype (needed by Builder / Instance cloning)
    virtual WorkItem* clone() const = 0;

    /**
     * @brief Hook called by AvailableState after the item becomes available.
     * Leaf: performs routing.  Composite: asks mediator to open entry children.
     */
    virtual void onAvailable() = 0;

protected:
    string id_;
    string name_;
    string outcome_;
    WorkItemState* state_; // owned
    SubProcess* parent_; // not owned
    vector<WorkflowEventListener*> listeners_; // not owned
    WorkflowMediator* mediator_; // not owned
	//RoutingStrategy* routingStrategy; - From generated .h
};
#endif
