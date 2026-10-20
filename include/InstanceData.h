#ifndef INSTANCEDATA_H
#define INSTANCEDATA_H

class InstanceData {

private:
	map<string, string> fields;

public:
	string get(string key);

	void set(string key, string value);

	bool has(string key);
};

#endif
