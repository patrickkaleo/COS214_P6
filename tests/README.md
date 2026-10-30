# TIVIDY Modular Testing Framework

Welcome to the **TIVIDY Generic Workflow Engine** unit test suite.

This project uses [doctest](https://github.com/doctest/doctest) configured for **modular multi-file testing**. Each team member maintains their own isolated test file, completely eliminating Git merge conflicts.

---

## Directory Structure

```
tests/
├── README.md                    # This guide
├── main_tests.cpp               # Master test runner entry point (DO NOT EDIT)
├── test_observer_strategy.cpp   # Enock Nyamweya (Observer, Strategy, Participant Directory)
├── test_state_adapter.cpp       # Angela Ramaboea (State, External Service Adapters)
├── test_builder.cpp             # Patrick Simuyemba (Builder & Director) - [To be added]
├── test_command_facade.cpp      # Katlego Sentsho (Command & Facade) - [To be added]
└── test_composite.cpp           # Tebogo Mokgwatsane (Composite & Iterator) - [To be added]
```

---

## How to Run Tests

### 1. Locally with Make
```bash
# Compile and run all registered test suites
make test

# Run tests with Valgrind memory leak checking
make valgrind
```

### 2. Using Docker & Docker Compose
```bash
# Run the test container
docker compose run --rm tests
```

---

## How to Add Your Subsystem Tests (3 Easy Steps)

The build system (`makefile`) automatically searches for and compiles **all** `.cpp` files in the `tests/` folder via `$(wildcard tests/*.cpp)`. You do **not** need to touch the `makefile` or `main_tests.cpp`!

### Step 1: Create your own test file
In the `tests/` folder, create a new file named after your subsystem:
- e.g. `tests/test_builder.cpp` or `tests/test_command.cpp`

### Step 2: Include `doctest.h` and your headers
```cpp
/**
 * @file test_builder.cpp
 * @brief Unit tests for Builder and Director subsystems.
 * @author Your Name
 */

#include "doctest.h"
#include "WorkflowDirector.h"
#include "FileWorkflowDefinitionBuilder.h"
```
> **IMPORTANT**: Do **NOT** put `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` in your file. That definition is already in `tests/main_tests.cpp`.

### Step 3: Write your test suite
```cpp
TEST_SUITE("Your Name - Subsystem Name") {

    TEST_CASE("Feature: brief description of behavior under test") {
        // Arrange
        // Act
        // Assert
        CHECK(actual == expected);
        REQUIRE(pointer != nullptr);
    }

}
```

### Step 4: Run `make test`
```bash
make test
```
Doctest will automatically discover and run your new test cases alongside the rest of the team's tests!

---

## Golden Rules for Team Collaboration

1. **Do NOT edit `tests/main_tests.cpp`**: It is reserved exclusively for doctest's main entry point.
2. **Do NOT edit another member's test file**: Keep your tests inside your own `test_<your_subsystem>.cpp`.
3. **Always run `make test` before pushing to `dev`**: Ensure your tests pass locally before opening a pull request.
