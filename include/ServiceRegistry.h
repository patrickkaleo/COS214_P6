#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

class ServiceRegistry {

private:
	map<string, ExternalService*> services;

public:
	void add(ExternalService* service);

	ExternalService* find(string name);
};

#endif
