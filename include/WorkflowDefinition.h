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
 * @brief Finished workflow product. Holds the root work-item tree and the dependency rules.
 * Two live definitions cannot share an id.
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
	 * @param id id of workflow defintion. Empty and duplicate ids are not accepted.
	 * @param name name of workflow defintion
	 * @param version version of workflow defintion
	 */
	WorkflowDefinition(string id, string name, int version);

	/**
	 * @brief Release the reserved id and delete the root tree and dependency rules.
	 */
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
	 * @return false when the id is empty or already used by another definition
	 */
	bool setId(string id);
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
	 * @return vector<DependencyRule*> the rules stored on this definition
	 */
	vector<DependencyRule *> getRules();

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param newRule rule with fromId, toId and outcome. This definition takes ownership.
	 */
	void addRule(DependencyRule *newRule);

	/**
	 * @brief Clone the root work item so a new instance can run from that root.
	 * @return WorkItem* deep copy of the root, or nullptr when the definition has no root
	 */
	WorkItem *instantiateTree();
};

#endif
