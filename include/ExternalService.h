#ifndef EXTERNALSERVICE_H
#define EXTERNALSERVICE_H

class ExternalService {


public:
	virtual ServiceResult invoke(const ServiceRequest& request) = 0;

	virtual string name() = 0;
};

#endif
