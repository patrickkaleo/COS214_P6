# TIVIDY tests

The engine is described in the [root README](../README.md). This directory is the doctest suite. `main_tests.cpp` supplies `main`. Every other `tests/*.cpp` file is picked up by the makefile, so a new test file does not need a makefile change.

Do not put `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` in a suite file. That definition is already in `main_tests.cpp`.

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
