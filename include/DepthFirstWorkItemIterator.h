#ifndef DEPTHFIRSTWORKITEMITERATOR_H
#define DEPTHFIRSTWORKITEMITERATOR_H

#include "WorkItemIterator.h"
#include <stack>

class WorkItem;
class SubProcess;

class DepthFirstWorkItemIterator : public WorkItemIterator {
public:
    explicit DepthFirstWorkItemIterator(SubProcess* root);
    ~DepthFirstWorkItemIterator() override = default;

    void first() override;
    void next() override;
    bool isDone() const override;
    WorkItem* current() const override;

private:
    SubProcess* root_;
    std::stack<WorkItem*> stack_;
    WorkItem* currentItem_;

    void pushChildren(WorkItem* item);
};

#endif