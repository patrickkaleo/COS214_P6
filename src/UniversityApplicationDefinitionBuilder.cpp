#include "UniversityAplicationDefinitionBuilder.h"

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id) 
{
	this->definition->setId(id);
}

void UniversityApplicationDefinitionBuilder::buildStructure()
{
	addSubProcess("UNI_APP", "University Application", 5);

	addSubProcess("INTAKE", "Application Intake", "UNI_APP");
	addTask("CAPTURE", "Capture Application", "INTAKE");
	addTask("VERIFY_DOCS", "Verify Documents", "INTAKE");

	addSubProcess("REVIEW", "Academic Review", "UNI_APP");
	addTask("FACULTY_REVIEW", "Faculty Review", "REVIEW");
	addTask("DECISION", "Admission Decision", "REVIEW");

	addSubProcess("OFFER", "Offer and Registration", "UNI_APP");
	addTask("ISSUE_OFFER", "Issue Offer", "OFFER");
	addTask("REGISTER", "Registration", "OFFER");
}

void UniversityApplicationDefinitionBuilder::buildDependencies()
{
	vector<string> reviewOrder;
	reviewOrder.push_back("CAPTURE");
	reviewOrder.push_back("VERIFY_DOCS");
	reviewOrder.push_back("FACULTY_REVIEW");
	reviewOrder.push_back("DECISION");
	// addSequence(reviewOrder);

	addDependency("DECISION", "ISSUE_OFFER", "approved");
	addDependency("ISSUE_OFFER", "REGISTER", "*");
}

void UniversityApplicationDefinitionBuilder::buildRouting()
{
	setRouting("CAPTURE", new RoleBasedRouting("AdmissionsOfficer"));
	setRouting("VERIFY_DOCS", new RoleBasedRouting("Registrar"));
	setRouting("FACULTY_REVIEW", new RoleBasedRouting("FacultyOfficer"));
	setRouting("DECISION", new RoleBasedRouting("Dean"));
	setRouting("ISSUE_OFFER", new RoleBasedRouting("AdmissionsOfficer"));
	setRouting("REGISTER", new RoleBasedRouting("Registrar"));
}

void UniversityApplicationDefinitionBuilder::buildIntegrations()
{
	setService("VERIFY_DOCS", "DocumentVerificationService");
	setService("ISSUE_OFFER", "NotificationService");
}

void UniversityApplicationDefinitionBuilder::buildExtras()
{
	markCheckpoint("DECISION");
	addPriority("FACULTY_REVIEW", 2);
	addAudit("DECISION");
	addAudit("ISSUE_OFFER");
}
