#ifndef CONCRETEAPPROVALHANDLER_H
#define CONCRETEAPPROVALHANDLER_H

/**
 * @file ConcreteApprovalHandler.h
 * @brief One configurable level of authority in an approval chain.
 * @author Angelo Anthony
 * @version 1.0
 */

#include <memory>
#include <string>
#include "ApprovalHandler.h"
#include "ApprovalRule.h"

/**
 * @class ConcreteApprovalHandler
 * @brief ConcreteHandler configured with an approver role, an ApprovalRule and a deadline.
 *
 * handle() works in three steps:
 * 1. If the request was escalated past this level (request.level > 0), it is passed
 *    on with one level fewer.
 * 2. If the rule accepts the request and someone holds the approver role, that
 *    person is returned.
 * 3. Otherwise the request goes to the next handler.
 *
 * Every level of every process uses this one class. Only the role, rule and
 * deadline change.
 */
class ConcreteApprovalHandler : public ApprovalHandler {
private:
    std::string approverRole;           ///< Role whose holders decide at this level.
    std::unique_ptr<ApprovalRule> rule; ///< This level's authority (owned).
    int deadlineHours;                  ///< Time allowed before the task escalates (0 = none).

public:
    /**
     * @param role Role of the approver at this level.
     * @param authority Rule deciding which requests this level may approve (ownership is taken).
     * @param deadline Hours allowed before escalation; 0 means no deadline.
     */
    ConcreteApprovalHandler(const std::string& role, ApprovalRule* authority, int deadline = 0);

    ~ConcreteApprovalHandler() override;

    Participant* handle(ApprovalRequest& request) override;
    ApprovalHandler* clone() const override;
    std::string describe() const override;

    std::string getApproverRole() const;
    int getDeadlineHours() const;
};

#endif // CONCRETEAPPROVALHANDLER_H
