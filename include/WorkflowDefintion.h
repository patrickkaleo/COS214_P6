#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include <string>
#include <vector>
using namespace std;

#include "SubProcess.h"
#include "DependencyRule.h"

/**
 * @class WorkflowDefition
 * @brief A template for a workflow eg a workflow to review a bursary application or a a university application
 */
class WorkflowDefinition
{

private:
	string definitonId;
	vector<string> sequence;
	string name;
	int version;
	SubProcess *rootProcess;
	vector<DependencyRule *> dependencies;

public:
	/**
	 * @brief constructor 
	 * @param name name of workflow defintion
	 * @param id of workflow defintion
	 */
	WorkflowDefinition(string name , int id);
	
	/**
	 * @brief return the root Work item / subProcess
	 * @return SubProcess* 
	 */
	const SubProcess *getRoot();
	
	/**
	 * @brief return the dependency rules for a given workflow definition
	 * @return vector<DependencyRule*>
	 */
	const vector<DependencyRule*> getRules();
	
	/**
	 * @brief return version of the workflowDefinition
	 * @return int
	 */
	int getVersion();
	
	/**
	 * @brief start the execution of the tasks in the work flow defition
	 * @return SubProcess*
	 */
	SubProcess *instantiateTree();
	
	/**
	 * @brief set root process 
	 * @param sp subprocess to be added 
	 */
	void setRoot(SubProcess *sp);
	
	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param dependency
	 */
	void addDendency(DependencyRule *dependency);
	
	/**
	 * @brief set the seuence of excution for the workflow 
	 * @param ids sequence of ids detailing the order of execution for the workflow definition
	 */
	void setSequence(vector<string> ids) ;
};

#endif
