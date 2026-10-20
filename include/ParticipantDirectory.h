#ifndef PARTICIPANTDIRECTORY_H
#define PARTICIPANTDIRECTORY_H

class ParticipantDirectory {

private:
	vector<Participants*> participants;

public:
	vector<Participant*> withRole(string role);

	Participant* findById(string id);
};

#endif
