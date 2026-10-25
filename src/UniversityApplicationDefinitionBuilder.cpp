#include "UniversityAplicationDefinitionBuilder.h"

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id)
	: UniversityApplicationDefinitionBuilder(id, "University Application Workflow", 1)
{
}

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id, string name, int version)
	: WorkflowDefinitionBuilder(id, name, version)
{
}

void UniversityApplicationDefinitionBuilder::buildStructure()
{
	addSubProcess("UNI_APP", "University Application", "");

	addSubProcess("INTAKE", "Application Intake", "UNI_APP");
	addTask("CAPTURE", "Capture Application", "INTAKE");
	addTask("VERIFY_DOCS", "Verify Documents", "INTAKE");

	addSubProcess("REVIEW", "Academic Review", "UNI_APP");
	addTask("FACULTY_REVIEW", "Faculty Review", "REVIEW");
	addTask("DECISION", "Admission Decision", "REVIEW");

	addSubProcess("OFFER", "Offer and Registration", "UNI_APP");
	addTask("ISSUE_OFFER", "Issue Offer", "OFFER");
	addTask("REGISTER", "Registration", "OFFER");
	addTask("NOTIFY_REJECTION", "Notify Applicant of Rejection", "OFFER");
}

void UniversityApplicationDefinitionBuilder::buildDependencies()
{
	addDependency("CAPTURE", "VERIFY_DOCS", "*");
	addDependency("VERIFY_DOCS", "FACULTY_REVIEW", "*");
	addDependency("FACULTY_REVIEW", "DECISION", "*");
	addDependency("DECISION", "ISSUE_OFFER", "approved");
	addDependency("DECISION", "NOTIFY_REJECTION", "rejected");
	addDependency("ISSUE_OFFER", "REGISTER", "*");
}

void UniversityApplicationDefinitionBuilder::buildRouting()
{
	setRouting("CAPTURE", new RoleBasedRouting("AdmissionsOfficer"));
	setRouting("VERIFY_DOCS", new RoleBasedRouting("Registrar"));
	setRouting("FACULTY_REVIEW", new RoleBasedRouting("FacultyOfficer"));
	setRouting("DECISION", new RoleBasedRouting("Dean"));
	setRouting("ISSUE_OFFER", new RoleBasedRouting("AdmissionsOfficer"));
	setRouting("NOTIFY_REJECTION", new RoleBasedRouting("AdmissionsOfficer"));
	setRouting("REGISTER", new RoleBasedRouting("Registrar"));
}

void UniversityApplicationDefinitionBuilder::buildIntegrations()
{
	setService("VERIFY_DOCS", "DocumentVerificationService");
	setService("ISSUE_OFFER", "NotificationService");
	setService("NOTIFY_REJECTION", "NotificationService");
}

void UniversityApplicationDefinitionBuilder::buildExtras()
{
	markCheckpoint("DECISION");
	addPriority("FACULTY_REVIEW", 2);
	addAudit("DECISION");
	addAudit("ISSUE_OFFER");
	addAudit("NOTIFY_REJECTION");
}
