#ifndef ROUTINGSTRATEGY_H
#define ROUTINGSTRATEGY_H

class RoutingStrategy {


public:
	virtual RoutingStratefy* clone() = 0;

	virtual Participant select(const Task& task, const InstanceData& data, const ParticipantDirectory& directory) = 0;
};

#endif
