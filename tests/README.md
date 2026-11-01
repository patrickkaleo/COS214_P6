# TIVIDY tests

The engine is described in the [root README](../README.md). This directory is the doctest suite. `main_tests.cpp` supplies `main`. Every other `tests/*.cpp` file is picked up by the makefile, so a new test file does not need a makefile change.

Do not put `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` in a suite file. That definition is already in `main_tests.cpp`.

## Definitions and contexts

The suite does not read `context/*.context`. A context file is for the terminal interface: which definition to offer, who the people are, what to ask, and which outcome buttons to show. The engine runs a definition file. A test does the context's job itself. It loads the definition, adds participants, and writes the field values into `InstanceData` before `startInstance`.

```cpp
TividyWorkflowFacade facade;
std::string id = facade.registerDefinitionFromFile("definitions/university_bursary.definition");
facade.addParticipant("edu-admissions", "Lerato Khumalo", {"AdmissionsOfficer"});
InstanceData data;
data.set("surname", "Mokoena");
data.set("idNumber", "0501015800087");
facade.startInstance(id, data);
```

`tests/builder.txt` is a definition in that same format. It is not a context. `test_builder.cpp` builds it through `WorkflowDirector` and `FileWorkflowDefinitionBuilder`.

| Definition | Loaded by |
| --- | --- |
| `tests/builder.txt` | `test_builder.cpp` |
| `definitions/university_bursary.definition` | `test_definition_description.cpp`, `test_roles_and_delegation.cpp`, `test_scenario_flows.cpp` |
| `definitions/shipment_exception.definition` | `test_definition_description.cpp`, `test_scenario_rosters.cpp`, `test_scenario_flows.cpp` |

`test_scenario_flows.cpp` and `test_scenario_rosters.cpp` run those definitions through `TividyWorkflowFacade`. The word scenario in those test file names is an industry run, not a `.context` file. The healthcare, insurance, and manufacturing definitions are for the interface. This suite does not load them.

How a definition and a `.context` file have to agree, and which pairs the interface offers, is in the root README under [Using definitions and contexts](../README.md#using-definitions-and-contexts).

## Files

| File | What it covers |
| --- | --- |
| `main_tests.cpp` | Doctest entry point |
| `test_builder.cpp` | Builder and director, using `builder.txt` |
| `test_composite.cpp` | `WorkItem`, `Task`, `SubProcess`, and iteration |
| `test_decorator.cpp` | `PriorityDecorator` and `AuditDecorator` |
| `test_mediator_cor_iterator.cpp` | Mediator, approval chain, and iterator (Angelo Anthony) |
| `test_memento.cpp` | `WorkflowSnapshot` save and restore |
| `test_command_facade.cpp` | Commands and `TividyWorkflowFacade` |
| `test_observer_strategy.cpp` | Listeners, `EventDispatcher`, routing strategies (Enock Nyamweya) |
| `test_state_adapter.cpp` | Work-item states and external-service adapters (Angela Ramaboea) |
| `builder.txt` | Onboarding definition read by `test_builder.cpp` |

`builder.txt` includes a nested review process, role and data routing, services, checkpoints, priority, audit, and lines that the builder must ignore.

## Run

From the project root:

```bash
make test
make valgrind
docker compose run --rm tests
```

One suite can be selected with a doctest filter:

```bash
./tividy_tests -tc="*Builder*"
```

Add cases in the suite file for that subsystem. Run `make test` before pushing.
