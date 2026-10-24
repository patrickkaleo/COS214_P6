/**
 * @file AuditDecorator.h
 * @brief Concrete Decorator that records a detailed audit trail around every
 *        lifecycle operation on a WorkItem.
 * @version 1.0
 */

#ifndef AUDITDECORATOR_H
#define AUDITDECORATOR_H

#include "WorkItemDecorator.h"
#include <string>

// Forward declaration (Class in AuditLogger.h)
class AuditLogger;
class Participant;

/**
 * @class AuditDecorator
 * @brief Logs before/after details of every operation, then forwards to the
 *        inner WorkItem. The AuditLogger pointer is NOT owned (it lives in
 *        the WorkflowEngine / listener list).
 *
 * Use (from the Builder):
 * @code
 *   WorkItem* item = new Task("t2", "Bursary Payment");
 *   item = new AuditDecorator(item, engineAuditLogger);
 *   parent->add(item);
 * @endcode
 */
class AuditDecorator : public WorkItemDecorator {
public:
    /**
     * @brief Constructs an audit decorator.
     * @param inner  WorkItem to wrap (ownership transferred).
     * @param logger AuditLogger used to record entries (not owned).
     */
    AuditDecorator(WorkItem* inner, AuditLogger* logger);

    /** @brief Default destructor (inner is deleted by base; logger is not owned). */
    ~AuditDecorator() override = default;

    // audited lifecycle overrides

    /** @brief Log then forward makeAvailable. */
    bool makeAvailable() override;

    /** @brief Log then forward assign. */
    bool assign(Participant* participant) override;

    /** @brief Log then forward start. */
    bool start() override;

    /** @brief Log then forward complete. */
    bool complete(const std::string& outcome) override;

    /** @brief Log then forward reject. */
    bool reject(const std::string& reason) override;

    /** @brief Log then forward cancel. */
    bool cancel() override;

    /** @brief Log then forward escalate. */
    bool escalate() override;

    /**
     * @brief Log then forward onAvailable to the inner item.
     */
    void onAvailable() override;

    /**
     * @brief Deep-copy: clone the inner item and wrap it in a new AuditDecorator
     *        that shares the same logger pointer.
     * @return Newly allocated AuditDecorator*; caller owns it.
     */
    WorkItem* clone() const override;

private:
    AuditLogger* logger_;   /**< Not owned. */

    /**
     * @brief Helper that records an action against this item via the logger.
     * @param action Short description of the operation (e.g. "complete").
     */
    void audit(const std::string& action) const;
};

#endif // AUDITDECORATOR_H
