
#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "ParticipantDirectory.h"
#include "ServiceRegistry.h"

class WorkflowDefinition;
class WorkflowInstance;
class WorkItemAction;
class WorkflowEventListener;
class InstanceData;

/**
 * @brief Core workflow engine.
 *
 * Manages workflow definitions, running instances,
 * participants, services, listeners, and commands.
 */
class WorkflowEngine
{
private:
  // Registered workflow definitions
  std::map<std::string, WorkflowDefinition *> definitions;

  // Running workflow instances
  std::map<std::string, WorkflowInstance *> instances;

  // Successfully executed commands for undo
  std::vector<WorkItemAction *> actionHistory;

  // Participant management
  ParticipantDirectory directory;

  // External service adapters
  ServiceRegistry services;

  // Registered event listeners
  std::vector<WorkflowEventListener *> listeners;

  // Generates unique instance IDs
  unsigned long nextInstanceNumber;

public:
  WorkflowEngine();
  ~WorkflowEngine();

  WorkflowEngine(const WorkflowEngine &) = delete;
  WorkflowEngine &operator=(const WorkflowEngine &) = delete;

  // Definition management
  std::string registerDefinition(
      WorkflowDefinition *definition);

  // Instance management
  WorkflowInstance *createInstance(
      const std::string &definitionId,
      const InstanceData &data);

  WorkflowInstance *getInstance(
      const std::string &instanceId);

  // Access to shared subsystems
  ParticipantDirectory &getDirectory();

  ServiceRegistry &getServiceRegistry();

  // Observer registration
  void addListener(WorkflowEventListener *listener);

  // Command pattern
  bool executeAction(WorkItemAction *action);

  bool undoLastAction();

  // Deadline processing
  void checkDeadlines(std::time_t now);
};

#endif
