# TIVIDY

A generic workflow engine in C++11, run from an interactive terminal interface. One engine runs two industries, and each industry is supplied entirely as data and external-service adapters.

| Industry | Process | Files |
|---|---|---|
| Education | Faculty admissions and bursary processing | `data/education_admissions.*` |
| Logistics | Shipment exception handling | `data/logistics_shipment_exception.*` |

## Dependencies

- `g++` with C++11 support, and `make`
- Optional: `valgrind`, `doxygen`, Docker
- [doctest](https://github.com/doctest/doctest) is bundled as `include/doctest.h`

## Build, run and test

Run these from the project root, because the data files are read relative to it.

```bash
make            # builds ./tividy and ./tividy_tests
make run        # starts the interface
make test       # runs the unit and integration tests
make valgrind   # runs the tests under valgrind
make docs       # generates Doxygen output in docs/html/
```

With Docker:

```bash
docker compose run --rm tividy
docker compose run --rm tests
```

## Using the interface

You always act as one participant. Option 5 changes who, and a work item's menu offers **Act as the assignee** as a shortcut. Every action stays on the menu even when it is illegal, so you can try one and see the engine refuse it and why.

Choose **1 Explain the industries** for each process, its participants, input data and expected result. Choose **2 Start a workflow instance**, then press Enter to accept the sample data.

External systems are stubs behind real adapters, seeded with test data listed in the input prompts.

### Education

| To see | Enter or do |
|---|---|
| Completion | Default data. Work through the items: `admit`, `bursary requested`, `approved`, then pay and close |
| Decision branch | Complete Academic Review with `decline`: the offer path is skipped and the regret letter opens |
| Data routing | Faculty `Humanities` routes the review to a different reviewer |
| Approval chain | Amount `50000` goes to the Aid Officer, `50001` to the Manager, `150001` to the Dean |
| Rejection | Surname `Wrong`: identity check replies with a mismatch; reject it and the instance terminates |
| Service failure | Amount `150000`: the Manager can approve it, but the bank gateway's R100 000 limit fails and it escalates |
| Deadline | At Approve Bursary, menu option 9 with `49` hours moves it up the chain, one level per check |

### Logistics

| To see | Enter or do |
|---|---|
| Completion and parallel work | Default data. Complete Assess Exception with `reroute`: carrier and customer work open together, and Close waits for both |
| Decision branch | Complete Assess Exception with `write off`: the damage claim opens instead |
| Data routing | Severity `low` and `high` go to different people |
| Escalation | Severity `critical`: nobody holds the role, so the item waits to be claimed |
| Service failure | Waybill `WB9999` or hub `Polokwane`: the call fails, and the item escalates |

### Anywhere

- **Instance screen:** `h` for history, `d` for data, `u` for undo.
- **Main menu:** 4 for your work queue (highest priority first), 7 for the audit trail, 8 for metrics.
- **Independence:** start two instances of one definition and advance only one.
- **Undo:** it restores the instance from the snapshot taken before the action. It cannot recall a call an external system has already handled, such as a payment.

## Adding a workflow

No engine code changes are needed. Write a `.workflow` definition (format in `include/FileWorkflowDefinitionBuilder.h`) and a `.scenario` file, then add the scenario's path to `SCENARIO_FILES` in `src/main.cpp`. A new external system also needs an adapter implementing `ExternalService`, registered in `Tui::initialise`.

## Project layout

```
include/, src/   engine and interface (src/main.cpp)
data/            industry definitions and scenarios
tests/           doctest suites, one file per member (see tests/README.md)
docs/            UML, specifications and the Practical 6 report
```
