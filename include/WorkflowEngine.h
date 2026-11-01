
#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "HistoryLog.h"
#include "ParticipantDirectory.h"
#include "ServiceRegistry.h"

class Participant;
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

  // Pre-action snapshots, one stack per instance. Undo restores the newest.
  HistoryLog history;

  // Participant management
  ParticipantDirectory directory;

  // Named rosters, for callers whose workflows each own their participants
  std::map<std::string, ParticipantDirectory> rosters;

  // External service adapters
  ServiceRegistry services;

  // Registered event listeners
  std::vector<WorkflowEventListener *> listeners;

  // Generates unique instance IDs
  unsigned long nextInstanceNumber;
  std::string lastError;

public:
  WorkflowEngine();
  ~WorkflowEngine();

  WorkflowEngine(const WorkflowEngine &) = delete;
  WorkflowEngine &operator=(const WorkflowEngine &) = delete;

  // Definition management
  std::string registerDefinition(
      WorkflowDefinition *definition);

  // Instance management
  // An instance routes only to the participants of its roster
  WorkflowInstance *createInstance(
      const std::string &definitionId,
      const InstanceData &data,
      const std::string &roster = "");

  WorkflowInstance *getInstance(
      const std::string &instanceId) const;
  std::vector<std::string> definitionIds() const;
  WorkflowDefinition *getDefinition(const std::string &definitionId) const;
  std::vector<std::string> instanceIds() const;
  std::vector<std::string> historyDescriptions() const;
  const std::string &getLastError() const { return lastError; }

  // Access to shared subsystems
  ParticipantDirectory &getDirectory();

  // The named roster, created on first use; an empty name is the shared directory
  ParticipantDirectory &getRoster(const std::string &roster);

  // Finds a participant in the named roster, or nullptr
  Participant *findParticipant(const std::string &id, const std::string &roster) const;
  std::vector<Participant *> participantsIn(const std::string &roster) const;
  const ParticipantDirectory &getDirectory() const;

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
