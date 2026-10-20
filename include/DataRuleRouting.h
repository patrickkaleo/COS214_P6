#ifndef DATARULEROUTING_H
#define DATARULEROUTING_H

class DataRuleRouting : RoutingStrategy {

private:
	string field;
	map<string, string> valueToRole;
	int fallbackRole;

public:
	Participant* select(const Task& task, const InstanceData& data, const ParticipantDirectory& directory);

	RoutingStrategy* clone();
};

#endif
