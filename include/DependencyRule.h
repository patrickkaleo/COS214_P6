#ifndef DEPENDENCYRULE_H
#define DEPENDENCYRULE_H
#include <string>

/**
 * @class DependencyRule
 */
class DependencyRule
{
public:
	/**
	 * @brief return from id
	 * @return std::string
	 */
	std::string getFrom();

	/**
	 * @brief return toId
	 * @return std::string
	 */
	std::string getTo();

	/**
	 * @brief return outcome
	 * @return std::string
	 */
	std::string getOutcome();

	// /**
	//  * @brief set from
	//  * @param fromId std::string id
	//  */
	// void setFrom(std::string fromId);

	// /**
	//  * @brief set to id
	//  * @param toId
	//  */
	// void setTo(std::string toId);

	/**
	 * @brief set the outcome of the dependency rule
	 * @param outcome std::string of the outcome
	 */
	void setOutcome(std::string outcome);

	/**
	 * @brief constructor
	 * @param fromId
	 * @param toId
	 * @param outcome
	 */
	DependencyRule(std::string fromId, std::string toId, std::string outcome);

private:
	DependencyRule();
	std::string fromId;
	std::string toId;
	std::string outcome;
};

#endif
