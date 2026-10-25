#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include <string>
#include <vector>
using namespace std;

#include "DependencyRule.h"
#include "WorkItem.h"
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
	WorkItem *root;
	vector<DependencyRule *> rules;

public:
	/**
	 * @brief constructor
	 * @param id id of workflow defintion
	 * @param name name of workflow defintion
	 * @param version version of workflow defintion
	 */
	WorkflowDefinition(string id, string name, int version);
	~WorkflowDefinition();

	WorkflowDefinition(const WorkflowDefinition &) = delete;
	WorkflowDefinition &operator=(const WorkflowDefinition &) = delete;

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
	 * @brief return the root
	 * @return WorkItem*
	 */
	WorkItem *getRoot();

	/**
	 * @brief set root process
	 * @param root WorkItem to be added
	 */
	void setRoot(WorkItem *root);

	/**
	 * @brief return the dependency rules for a given workflow definition
	 * @return vector<DependencyRule>&
	 */
	vector<DependencyRule *> getRules();

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param newRule new dependency rule wit hdefined to, from and outcome
	 */
	void addRule(DependencyRule *newRule);

	/**
	 * @brief clone and return root process for an instance
	 * @return WorkItem*
	 */
	WorkItem *instantiateTree();
};

#endif
