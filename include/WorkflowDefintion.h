#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include <string>
#include <vector>
using namespace std;

#include "DependencyRule.h"
#include "SubProcess.h"
#include "RoutingStrategy.h"

/**
 * @class WorkflowDefition
 * @brief A template for a workflow eg a workflow to review a bursary application or a a university application
 */
class WorkflowDefinition
{
private:
	string id;
	string name;
	int version;
	SubProcess* root;
	vector<DependencyRule> rules;

public:
	/**
	 * @brief constructor
	 * @param id id of workflow defintion
	 * @param name name of workflow defintion
	 * @param version version of workflow defintion
	 */
	WorkflowDefinition(string id, string name, int version);
	~WorkflowDefinition();

	WorkflowDefinition(const WorkflowDefinition&) = delete;
	WorkflowDefinition& operator=(const WorkflowDefinition&) = delete;

	/**
	 * @brief return id of the workflow definition
	 * @return string
	 */
	string getId();

	/**
	 * @brief set id of the workflow definition
	 * @param id new id
	 */
	void setId(string id);

	/**
	 * @brief return name of the workflow definition
	 * @return string
	 */
	string getName();

	/**
	 * @brief set name of the workflow definition
	 * @param name new name
	 */
	void setName(string name);

	/**
	 * @brief return version of the workflowDefinition
	 * @return int
	 */
	int getVersion();

	/**
	 * @brief set version of the workflow definition
	 * @param version new version
	 */
	void setVersion(int version);

	/**
	 * @brief return the root Work item / subProcess
	 * @return SubProcess*
	 */
	SubProcess* getRoot();

	/**
	 * @brief set root process
	 * @param root subprocess to be added
	 */
	void setRoot(SubProcess* root);

	/**
	 * @brief return the dependency rules for a given workflow definition
	 * @return vector<DependencyRule>&
	 */
	vector<DependencyRule>& getRules();

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param fromId string id of where the dependency rule is from
	 * @param toId string id of the dependency rule if going to
	 * @param outcome string description of the evaluation of the dependenc yrule for a given workflow instance
	 */
	void addRule(string fromId, string toId, string outcome);

	/**
	 * @brief clone and return root process for an instance 
	 * @return SubProcess*
	 */
	SubProcess* instantiateTree();
};

#endif
