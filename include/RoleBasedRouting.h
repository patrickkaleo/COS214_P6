#ifndef ROLEBASEDROUTING_H
#define ROLEBASEDROUTING_H

class RoleBasedRouting : RoutingStrategy {

public:
	string role;

	Participant select(const Task& task, const InstanceData& data, const ParticipantDirectory& directory);
};

#endif
