#ifndef APPROVALROUTING_H
#define APPROVALROUTING_H

class ApprovalRouting : RoutingStrategy {

public:
	ApprovalHandler* chain;

	Participan select(const Task& task, const InstanceData& data, const ParticipantDirectory& directory);

	RoutingStrategy* clone();
};

#endif
