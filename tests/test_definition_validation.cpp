#include "doctest.h"
#include "FileWorkflowDefinitionBuilder.h"
#include "WorkflowDirector.h"
#include "WorkItemDecorator.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {
class TemporaryDefinition {
public:
    std::string path;
    explicit TemporaryDefinition(const std::string &content) {
        const char *directory = std::getenv("TMPDIR");
#ifdef _WIN32
        if (!directory) directory = std::getenv("TEMP");
        if (!directory) directory = ".";
#else
        if (!directory) directory = "/tmp";
#endif
        static unsigned counter = 0;
        path = std::string(directory) + "/tividy-definition-" +
            std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) +
            "-" + std::to_string(++counter) + ".txt";
        std::ofstream output(path.c_str());
        output << content;
        if (!output) throw std::runtime_error("Unable to write temporary definition fixture");
    }
    ~TemporaryDefinition() { std::remove(path.c_str()); }
};

std::unique_ptr<WorkflowDefinition> loadDocument(const std::string &content) {
    TemporaryDefinition file(content);
    FileWorkflowDefinitionBuilder builder(file.path);
    WorkflowDirector director;
    return std::unique_ptr<WorkflowDefinition>(director.construct(&builder));
}

const std::string header = "DEFINITION\nVALIDATION_FILE|Validation workflow|1\nWORKITEMS\n";
const std::string tree = "process|ROOT|-|Root\ntask|A|ROOT|First\ntask|B|ROOT|Second\n";

class ExposedBuilder : public WorkflowDefinitionBuilder {
public:
    using WorkflowDefinitionBuilder::startDefinition;
    using WorkflowDefinitionBuilder::addProcess;
    using WorkflowDefinitionBuilder::addTask;
    using WorkflowDefinitionBuilder::addDependency;
    using WorkflowDefinitionBuilder::setRouting;
    void buildStructure() override {}
    void buildDependencies() override {}
    void buildServices() override {}
    void buildRouting() override {}
    void buildExtras() override {}
};
}

TEST_SUITE("Definition validation") {
    TEST_CASE("Malformed records are rejected rather than silently dropping workflow behavior") {
        const std::vector<std::pair<std::string, std::string> > invalid = {
            {"missing item name", header + tree + "task|BROKEN|ROOT\n"},
            {"unknown item kind", header + tree + "not-a-task|GHOST|ROOT|Ghost\n"},
            {"missing dependency outcome", header + tree + "DEPENDENCIES\nA|B\n"},
            {"missing routing role", header + tree + "ROUTING\nA|role\n"},
            {"unknown service task", header + tree + "SERVICES\nservice|MISSING|Nowhere\n"},
            {"missing priority", header + tree + "EXTRAS\npriority|A\n"},
            {"missing deadline", header + tree + "EXTRAS\ndeadline|A\n"},
            {"unknown deadline target", header + tree + "EXTRAS\ndeadline|MISSING|24\n"},
            {"deadline on process", header + tree + "EXTRAS\ndeadline|ROOT|24\n"},
            {"zero deadline", header + tree + "EXTRAS\ndeadline|A|0\n"},
            {"negative deadline", header + tree + "EXTRAS\ndeadline|A|-1\n"},
            {"partial deadline", header + tree + "EXTRAS\ndeadline|A|24oops\n"},
            {"out of range deadline", header + tree + "EXTRAS\ndeadline|A|87601\n"},
            {"overflow deadline", header + tree + "EXTRAS\ndeadline|A|999999999999999999999999\n"},
            {"unknown extra", header + tree + "EXTRAS\nmagic|A\n"},
            {"unknown routing", header + tree + "ROUTING\nA|magic|Officer\n"},
            {"unpaired data mapping", header + tree + "ROUTING\nA|data|risk|Officer|high\n"},
            {"duplicate mapping", header + tree + "ROUTING\nA|data|risk|Officer|high|Senior|high|Junior\n"},
            {"duplicate routing", header + tree + "ROUTING\nA|role|Officer\nA|role|Senior\n"},
            {"service on process", header + tree + "SERVICES\nservice|ROOT|Example\n"},
            {"unknown service keyword", header + tree + "SERVICES\nother|A|Example\n"},
            {"checkpoint on process", header + tree + "EXTRAS\ncheckpoint|ROOT\n"},
            {"missing audit item", header + tree + "EXTRAS\naudit|MISSING\n"},
            {"negative priority", header + tree + "EXTRAS\npriority|A|-1\n"},
            {"partial numeric priority", header + tree + "EXTRAS\npriority|A|2oops\n"},
            {"partial numeric version", "DEFINITION\nVALIDATION_FILE|Name|2oops\nWORKITEMS\n" + tree},
            {"nonpositive version", "DEFINITION\nVALIDATION_FILE|Name|0\nWORKITEMS\n" + tree},
            {"empty id", "DEFINITION\n|Name|1\nWORKITEMS\n" + tree},
            {"extra field", header + tree + "EXTRAS\naudit|A|ignored\n"},
            {"duplicate section", header + tree + "WORKITEMS\n"},
            {"record outside section", "orphan|record\n" + header + tree},
            {"empty file", ""}
        };
        for (const auto &example : invalid) {
            INFO(example.first);
            CHECK_THROWS_AS(loadDocument(example.second), std::invalid_argument);
        }
        // Failed builders release their partial products and definition ID reservation.
        std::unique_ptr<WorkflowDefinition> valid = loadDocument(header + tree);
        REQUIRE(valid.get() != nullptr);
        CHECK(valid->getId() == "VALIDATION_FILE");
    }

    TEST_CASE("Invalid hierarchy and dependency graphs fail before instance creation") {
        const std::vector<std::pair<std::string, std::string> > invalid = {
            {"no root", header + "task|A|MISSING|First\n"},
            {"two roots", header + tree + "process|OTHER|-|Other root\n"},
            {"duplicate id", header + tree + "task|A|ROOT|Duplicate\n"},
            {"task root", header + "task|A|-|Wrong root\n"},
            {"unknown parent", header + tree + "task|C|MISSING|Child\n"},
            {"task parent", header + tree + "task|C|A|Child\n"},
            {"parent cycle", header + tree + "process|P|Q|P\nprocess|Q|P|Q\n"},
            {"unknown source", header + tree + "DEPENDENCIES\nMISSING|A|*\n"},
            {"unknown target", header + tree + "DEPENDENCIES\nA|MISSING|*\n"},
            {"self dependency", header + tree + "DEPENDENCIES\nA|A|*\n"},
            {"dependency cycle", header + tree + "DEPENDENCIES\nA|B|*\nB|A|*\n"},
            {"duplicate rule", header + tree + "DEPENDENCIES\nA|B|*\nA|B|*\n"},
            {"parent waits for its child", header + tree + "DEPENDENCIES\nA|ROOT|*\n"},
            {"child waits for parent completion", header + tree + "DEPENDENCIES\nROOT|A|*\n"}
        };
        for (const auto &example : invalid) {
            INFO(example.first);
            CHECK_THROWS_AS(loadDocument(example.second), std::invalid_argument);
        }
    }

    TEST_CASE("Forward parent references and checkpoint after decoration are supported") {
        std::unique_ptr<WorkflowDefinition> valid = loadDocument(header +
            "task|A|INNER|First\nprocess|INNER|ROOT|Nested\nprocess|ROOT|-|Root\n"
            "EXTRAS\naudit|A\npriority|A|2\ncheckpoint|A\ndeadline|A|24\naudit|ROOT\n");
        REQUIRE(valid.get() != nullptr);
        WorkItem *root = valid->getRoot();
        REQUIRE(root != nullptr);
        REQUIRE(root->childCount() == 1);
        WorkItem *item = root->getChild(0)->getChild(0);
        REQUIRE(item != nullptr);
        CHECK(item->getId() == "A");
        while (WorkItemDecorator *layer = dynamic_cast<WorkItemDecorator *>(item)) item = layer->getInner();
        Task *task = dynamic_cast<Task *>(item);
        REQUIRE(task != nullptr);
        CHECK(task->isCheckpoint());
        CHECK(task->getEscalationLevel() == 2);
        CHECK(task->getDeadlineHours() == 24);
    }

    TEST_CASE("Base builder rejects destructive replacement and remains usable") {
        ExposedBuilder builder;
        builder.startDefinition("VALIDATION_PROGRAMMATIC", "Programmatic", 1);
        builder.addProcess("ROOT", "Original root", "-");
        builder.addTask("A", "First", "ROOT");
        CHECK_THROWS_AS(builder.addProcess("OTHER", "Replacement", "-"), std::invalid_argument);
        CHECK_THROWS_AS(builder.addTask("A", "Duplicate", "ROOT"), std::invalid_argument);
        CHECK_THROWS_AS(builder.addTask("B", "Orphan", "MISSING"), std::invalid_argument);
        CHECK_THROWS_AS(builder.setRouting("MISSING", new RoleBasedRouting("Officer")), std::invalid_argument);
        std::unique_ptr<WorkflowDefinition> result(builder.getResult());
        REQUIRE(result.get() != nullptr);
        CHECK(result->getRoot()->getId() == "ROOT");
        CHECK(result->getRoot()->childCount() == 1);
        CHECK(result->getRoot()->getChild(0)->getId() == "A");
        CHECK(builder.getResult() == nullptr);
    }

    TEST_CASE("Shared validator protects definitions constructed without a builder") {
        WorkflowDefinition definition("VALIDATION_DIRECT", "Direct", 1);
        CHECK_THROWS_AS(WorkflowDefinitionBuilder::validateDefinition(&definition), std::invalid_argument);
        SubProcess *root = new SubProcess("ROOT", "Root");
        root->add(new Task("A", "First"));
        definition.setRoot(root);
        CHECK_NOTHROW(WorkflowDefinitionBuilder::validateDefinition(&definition));
        definition.addRule(new DependencyRule("A", "MISSING", "*"));
        CHECK_THROWS_AS(WorkflowDefinitionBuilder::validateDefinition(&definition), std::invalid_argument);
        WorkflowDirector director;
        CHECK(director.construct(nullptr) == nullptr);
    }

    TEST_CASE("Validation errors identify the offending section") {
        try {
            loadDocument(header + tree + "ROUTING\nA|data|risk|Officer|unpaired\n");
            FAIL("Malformed routing was accepted");
        } catch (const std::invalid_argument &error) {
            CHECK(std::string(error.what()).find("ROUTING record 1") != std::string::npos);
        }
    }
}
