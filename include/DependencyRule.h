#ifndef DEPENDENCYRULE_H
#define DEPENDENCYRULE_H
#include <string>
using namespace std;

/**
 * @class DependencyRule
 */
class DependencyRule
{
public:
	// getters
	string getFrom();
	string getTo();
	string getOutcome();

	// setters
	void setFrom(string);
	void setTo(string);
	void setOutcome(string);

	// constructor
	DependencyRule(string, string, string);

private:
	string fromId;
	string toID;
	string outcome;
};

#endif
