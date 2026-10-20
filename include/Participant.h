#ifndef PARTICIPANT_H
#define PARTICIPANT_H

class Participant {

private:
	string participantID;
	string name;
	Role* role;
	int workload;

public:
	bool hasRole(string role);

	int getWorkLoad();
};

#endif
