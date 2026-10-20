#ifndef APPROVALREQUEST_H
#define APPROVALREQUEST_H

class ApprovalRequest {

public:
	const Task* item;
	const InstanceData* data;
	int level;
private:
	const ParticipantDirectory* directory;
};

#endif
