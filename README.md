# TIVIDY

TIVIDY is a generic workflow engine. A workflow is a text definition: a tree of processes and tasks, dependency rules, routing, services, and optional priority or audit wrappers. The engine keeps those definitions and runs separate instances from them. Callers use `TividyWorkflowFacade`, and `./tividy` is an interactive terminal interface (a TUI) that runs both industries through it.

## Running the interface

From the project directory, run `make run`. The interface reads `definitions/` and `scenarios/`, so it has to start here.

It opens by asking which workflow you want, then who you are from its participants. The main menu then covers only that workflow; `0` goes back to choose another, and exits from there. Menu option 5 changes participant, and a work item's menu offers **Act as the assignee**. Actions marked as allowed are the ones the engine will accept. The others stay selectable so that you can see the engine refuse them and why. In a table, `▸` marks work you can act on now. Colour turns off when output is not a terminal or `NO_COLOR` is set.

| To see | Enter or do |
|---|---|
| Education, completion | Accept the sample data, complete each task, and choose `approved` for the decision and the bursary |
| Identity rejection | Surname `Wrong`: completing Verify Identity rejects the application and ends the instance |
| Appeal branch | Reject Verify Funding Documents: approval and payment are cancelled and the appeal opens |
| Data routing | Income bracket `low`, `mid` or `high` routes Approve Bursary to a different role |
| Payment failure | Amount `200000` is over the bank limit: completing Pay Bursary escalates it |
| Logistics, completion | Accept the sample data and choose `reroute` or `hold` at Assess Impact |
| Service failure | Waybill `WB9999` or hub `Polokwane`: the call fails and the task escalates until someone qualified claims it |
| Deadline | With Approve Bursary or Assess Impact open, main menu option 9 with `49` hours escalates it |
| Independence | Start two instances of one workflow and advance only one |
| Undo | `u` on an instance screen restores the snapshot taken before the last action. It cannot recall an external call such as a payment |

## Adding a workflow

No code changes or rebuild are needed. When the interface starts, it builds every `definitions/*.txt` file and reads every `scenarios/*.scenario` file. A workflow appears in the opening list once a scenario names its id. Anything that can't run yet appears in a **Not ready** box with the reason, and the rest carry on working.

1. Add `definitions/my_workflow.txt`. The `#` lines above the first section are its description, and the format is under [Definition file](#definition-file).

   ```
   # What this workflow does.

   DEFINITION
   MY_WORKFLOW|My Workflow|1

   WORKITEMS
   process|ROOT|-|My Workflow
   task|FIRST|ROOT|First Task
   task|SECOND|ROOT|Second Task

   DEPENDENCIES
   FIRST|SECOND|*

   ROUTING
   FIRST|role|Clerk
   SECOND|role|Manager
   ```

2. Add `scenarios/my_workflow.scenario`. `WORKFLOW` must match the id on the definition's `DEFINITION` line, and at least one `PARTICIPANT` is required.

   ```
   WORKFLOW|MY_WORKFLOW
   INDUSTRY|Retail
   EXPECTED|What should happen when it finishes.
   PARTICIPANT|p1|Sam Clerk|Clerk
   PARTICIPANT|p2|Alex Boss|Manager
   INPUT|orderNumber|Order number|A100
   OUTCOMES|SECOND|approved,declined
   ```

   `INDUSTRY`, `EXPECTED`, `INPUT` and `OUTCOMES` are optional. `INPUT` is key, prompt and default value. `OUTCOMES` is the choices offered when completing that task; without it you type an outcome. A participant may hold several comma-separated roles.

Each scenario owns its participants: work is only routed to, and only accepted from, people in that scenario's file. The same names, ids and roles can appear in any number of scenarios, and two people are simply different participants. A workflow that calls a new external system also needs an adapter registered in `Tui::initialise`; until then, that task escalates when it runs.

## How a workflow runs

```mermaid
flowchart TD
  file["Definition file"] --> director["WorkflowDirector"]
  director --> builder["FileWorkflowDefinitionBuilder"]
  builder --> definition["WorkflowDefinition"]
  definition --> engine["WorkflowEngine"]
  facade["TividyWorkflowFacade"] --> director
  facade --> engine
  engine --> instance["WorkflowInstance"]
  instance --> root["Root process makeAvailable"]
```

The facade asks the director to build a definition, then asks the engine to keep it and start an instance. The instance is a clone of the definition tree.

```mermaid
flowchart LR
  created["Created"] --> available["Available"]
  available --> assigned["Assigned"]
  assigned --> progress["InProgress"]
  progress --> completed["Completed"]
  progress --> rejected["Rejected"]
  available --> escalated["Escalated"]
  assigned --> escalated
  progress --> escalated
  escalated --> assigned
  available --> cancelled["Cancelled"]
  assigned --> cancelled
  progress --> cancelled
  escalated --> cancelled
```

1. A definition file is split into `DEFINITION`, `WORKITEMS`, `DEPENDENCIES`, `ROUTING`, `SERVICES`, and `EXTRAS`. Blank lines and lines that start with `#` are ignored.
2. `WorkflowDirector` drives `FileWorkflowDefinitionBuilder` in that same order. `construct` then returns the `WorkflowDefinition`. The builder no longer owns it.
3. The definition's root is a `SubProcess`. Processes own their children. A task is a leaf. Two live definitions cannot reserve the same id. A second definition built with an id that is already reserved keeps an empty id.
4. `WorkflowEngine::registerDefinition` stores the definition when the id is non-empty and not already registered. Otherwise it deletes the definition and returns an empty string. `registerDefinitionFromFile` does this from a path.
5. `startInstance` clones the definition tree into a `WorkflowInstance` with its own `InstanceData`. The definition stays the template. The instance id is `instance-1`, `instance-2`, and so on. `start` calls `makeAvailable` on the root.
6. Each work item delegates its lifecycle to a state object. A legal path is Created, Available, Assigned, InProgress, then Completed or Rejected. Available, Assigned, and InProgress can escalate to Escalated, which can be assigned again. Any non-final state can be cancelled. A state refuses a request that is not legal from there.
7. A task that becomes available asks its routing strategy for a participant. `RoleBasedRouting` uses one role name. `DataRuleRouting` reads one field from the instance data, maps values to roles, and uses the fallback role when the value is missing or unknown.
```mermaid
flowchart TD
  finished["Item finishes"] --> mediator["DependencyMediator"]
  mediator --> open["Open the next item"]
  mediator --> wait["Wait for the other incoming rules"]
  mediator --> skip["Cancel this branch"]
  mediator --> parent["Reject the parent process"]
```

8. `DependencyMediator` decides what opens next from the definition's rules. A child with no incoming rule opens when its process opens. Several such children run together. A later item waits until every incoming rule is decided. It opens when at least one rule is satisfied, and it is cancelled when none are. `"*"` matches any successful completion. `"rejected"` matches `reject()`, not the reason text. A finished subprocess completes with outcome `"completed"`. If an item is rejected and no rule handles `"rejected"`, the parent is rejected and the rest of that branch is cancelled. The mediator is bound to a tree with `registerTree`. The instance created by the engine starts from an empty mediator until that registration is done.
9. Claim, start, complete, reject, delegate, and cancel go through the facade as commands. The engine keeps a command that succeeds and deletes one that fails. `undoLast` undoes the newest successful command.
10. `WorkflowInstance` can store a `WorkflowSnapshot` of item states, assignees, outcomes, and instance data, and can restore that snapshot.
11. State changes are published through the `EventDispatcher` singleton. Listeners such as `AuditLogger` record them. The engine owns listeners passed to `addListener`.
12. A task with a service name calls the `ExternalService` registered under that name. Adapters wrap the identity registry, the bank gateway, and the carrier tracking system.
13. Approval can be a chain of handlers. The task knows the first handler. Each handler approves or passes the request on. Deleting the head deletes the chain.

## Definition file

```
DEFINITION
id|name|version

WORKITEMS
process|id|parentId|name
task|id|parentId|name

DEPENDENCIES
fromId|toId|outcome

ROUTING
itemId|role|roleName
itemId|data|field|fallbackRole|value|role|value|role

SERVICES
service|itemId|serviceName

EXTRAS
checkpoint|itemId
priority|itemId|level
audit|itemId
```

```mermaid
flowchart LR
  parent["Parent process"] --> audit["AuditDecorator"]
  audit --> priority["PriorityDecorator"]
  priority --> task["Task"]
```

A process whose parent id is `-` is the root. `priority` sets the task escalation level and wraps the tree node in a `PriorityDecorator`. `audit` wraps that node in an `AuditDecorator`. The definition owns the logger, because the decorator does not. Wrapping replaces the node in its parent, and the decorator owns the previous node. Checkpoint and service lines are applied to the task, so a checkpoint for an item is listed before that item is wrapped.

`definitions/university_bursary.txt` and `definitions/shipment_exception.txt` are full examples. `tests/builder.txt` is the fixture for the builder tests.

## Layout

```
include/          public headers
src/              engine sources
definitions/      workflow files
scenarios/        industry files: participants and input data for a workflow
tests/            doctest suites and tests/builder.txt
makefile          tividy and tividy_tests
Dockerfile        Ubuntu image used by docker compose
```

`src/decorator.cpp` is not part of the build. The decorator types are compiled from their own `.cpp` files.

## Build and run

Requirements are g++ with C++11, make, and, for the memory check, Valgrind. From the project directory:

```bash
make
make test
make valgrind
./tividy
```

`make test` builds `tividy_tests` and runs every suite. `make clean` removes `obj/`, `tividy`, and `tividy_tests`.

Docker builds the same targets and runs the tests:

```bash
docker compose run --rm tests
```

The `tividy` compose service copies `tividy` and `tividy_tests` onto the host and then runs `./tividy`.

Doxygen output goes to `docs/html/` after `make docs`.
