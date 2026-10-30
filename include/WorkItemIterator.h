#ifndef WORKITEMITERATOR_H
#define WORKITEMITERATOR_H

class WorkItemIterator {
public:
    virtual ~WorkItemIterator() = default;

    virtual void first() = 0;
    virtual void next() = 0;
    virtual bool isDone() const = 0;
    virtual WorkItem* current() const = 0;
};

#endif
