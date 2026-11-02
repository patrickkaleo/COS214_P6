
/**
 * @file test_command_facade.cpp
 * @brief Unit tests for the Command and Facade patterns.
 */

#include "doctest.h"

#include "TividyWorkflowFacade.h"
#include "WorkflowEngine.h"
#include "WorkItemAction.h"
#include "WorkflowInstance.h"
#include "InstanceData.h"
#include "Participant.h"

TEST_SUITE("Command and Facade")
{

  TEST_CASE("Facade: invalid workflow instance")
  {
    TividyWorkflowFacade facade;
    InstanceData data;

    std::string instanceId =
        facade.startInstance("missing-definition", data);

    CHECK(instanceId.empty());
  }

  TEST_CASE("Facade: unknown instance status")
  {
    TividyWorkflowFacade facade;

    CHECK(facade.getStatus("missing-instance") == "not found");
  }

  TEST_CASE("Facade: reject null actor")
  {
    TividyWorkflowFacade facade;

    CHECK_FALSE(facade.claim(
        "missing-instance", "task1", nullptr));
  }
}

TEST_SUITE("Command - WorkflowEngine")
{

  TEST_CASE("Engine rejects a null command")
  {
    WorkflowEngine engine;

    CHECK_FALSE(engine.executeAction(nullptr));
  }

  TEST_CASE("Engine cannot undo empty history")
  {
    WorkflowEngine engine;

    CHECK_FALSE(engine.undoLastAction());
  }
}
