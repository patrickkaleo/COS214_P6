#ifndef DEPENDENCYRULE_H
#define DEPENDENCYRULE_H
#include <string>

using namespace std;
struct DependencyRule
{
	string fromId;
	string toId;
	string outcome;

	/**
	 * @brief constructor
	 * @param fromId id of the item the rule starts from
	 * @param toId id of the item the rule goes to
	 * @param outcome outcome that satisfies the rule
	 */
	DependencyRule(string fromId, string toId, string outcome) : fromId(fromId), toId(toId), outcome(outcome) {}
};

#endif
