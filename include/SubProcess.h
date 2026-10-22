/**
 * @file SubProcess.h
 * @brief Composite pattern for the TIVIDY generic workflow engine (Composite Participant).
 *
 * WorkItem  – abstract Component
 * Task      – Leaf
 * SubProcess – Composite
 */

#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include "WorkItem.h"
#include <vector>
#include <string>

using namespace std;

/**
 * @brief Composite of the Composite pattern.
 * Owns a collection of child WorkItems (Tasks or nested SubProcesses).
 */
class SubProcess : public WorkItem {
public:
    SubProcess(const string& id, const string& name);
    ~SubProcess() override;

    void onAvailable() override;
    WorkItem* clone() const override;

    // Composite operations
    void add(WorkItem* child); // takes ownership
	//void remove(WorkItem* item);
    void remove(const string& id);
    WorkItem* getChild(int index) const;
    int childCount() const { return static_cast<int>(children_.size()); }
    bool allChildrenFinished() const;

    const vector<WorkItem*>& getChildren() const { return children_; }

	//WorkItemIterator* createIterator();

private:
    vector<WorkItem*> children_;//owned
};
#endif
