/**
 * @file test_definition_description.cpp
 * @brief Tests that a workflow's description comes from the # lines in its definition file.
 * @author Angela Ramaboea
 */

#include "doctest.h"

#include "FileWorkflowDefinitionBuilder.h"
#include "TividyWorkflowFacade.h"
#include "WorkflowDirector.h"

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

namespace {

const char* const BODY =
    "DEFINITION\nDESC_TEST|Description Test|1\n\n"
    "# A note about this section, not part of the description.\n"
    "WORKITEMS\nprocess|ROOT|-|Root\ntask|A|ROOT|Task A\n\n"
    "ROUTING\nA|role|Clerk\n";

/// Builds a definition from a temporary file holding this text.
std::string descriptionOf(const std::string& text) {
    const std::string path = "tests/description_test.tmp";
    {
        std::ofstream file(path.c_str());
        file << text;
    }
    FileWorkflowDefinitionBuilder builder(path);
    WorkflowDirector director;
    std::unique_ptr<WorkflowDefinition> definition(director.construct(&builder));
    std::remove(path.c_str());
    REQUIRE(definition != nullptr);
    return definition->getDescription();
}

} // namespace

TEST_SUITE("Workflow descriptions from definition files") {

    TEST_CASE("The # lines above the first section become the description") {
        CHECK(descriptionOf(std::string("# First line.\n# Second line.\n\n") + BODY) == "First line.\nSecond line.");
    }

    TEST_CASE("A bare # keeps an empty line between paragraphs") {
        CHECK(descriptionOf(std::string("# One.\n#\n# Two.\n") + BODY) == "One.\n\nTwo.");
    }

    TEST_CASE("Empty # lines around the description are dropped") {
        CHECK(descriptionOf(std::string("#\n# Only.\n#\n") + BODY) == "Only.");
    }

    TEST_CASE("Comments inside sections are not part of the description") {
        CHECK(descriptionOf(std::string("# Top.\n") + BODY).find("note about") == std::string::npos);
    }

    TEST_CASE("A file without # lines has no description") {
        CHECK(descriptionOf(BODY).empty());
    }

    TEST_CASE("The facade reports the name and description of a registered workflow") {
        TividyWorkflowFacade facade;
        const std::string id = facade.registerDefinitionFromFile("definitions/university_bursary.definition");
        REQUIRE(id == "UNI_BURSARY");

        CHECK(facade.definitionName(id) == "University and Bursary Application");
        CHECK(facade.definitionDescription(id).find("University application and bursary funding.") == 0);
        CHECK(facade.definitionName("missing").empty());
        CHECK(facade.definitionDescription("missing").empty());
    }

    TEST_CASE("Every prepared workflow has a description") {
        TividyWorkflowFacade facade;
        for (const char* path : {"definitions/university_bursary.definition", "definitions/shipment_exception.definition", "definitions/hospital_admission.definition", "definitions/insurance_claim.definition", "definitions/production_order.definition"}) {
            const std::string id = facade.registerDefinitionFromFile(path);
            REQUIRE_FALSE(id.empty());
            CHECK_FALSE(facade.definitionDescription(id).empty());
        }
    }
}
