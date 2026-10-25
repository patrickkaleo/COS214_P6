#include "UniversityAplicationDefinitionBuilder.h"

#include "DataRuleRouting.h"

#include <map>

WorkItem* UniversityApplicationDefinitionBuilder::findWorkItem(string id)
{
	map<string, WorkItem*>::iterator found = workItems.find(id);
	return found == workItems.end() ? nullptr : found->second;
}

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id)
	: UniversityApplicationDefinitionBuilder(id, "University Application Workflow", 1)
{
}

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id, string name, int version)
	: WorkflowDefinitionBuilder(id, name, version)
{
}

void UniversityApplicationDefinitionBuilder::buildDefinition()
{
	SubProcess* root = new SubProcess("UNI_APP", "University Application");
	definition->setRoot(root);
	workItems["UNI_APP"] = root;

	SubProcess* intake = new SubProcess("INTAKE", "Application Intake");
	addWorkItem(intake, root);
	workItems["INTAKE"] = intake;
	Task* capture = new Task("CAPTURE", "Capture Application");
	addWorkItem(capture, intake);
	workItems["CAPTURE"] = capture;
	Task* verifyIdentity = new Task("VERIFY_IDENTITY", "Verify Identity");
	addWorkItem(verifyIdentity, intake);
	workItems["VERIFY_IDENTITY"] = verifyIdentity;
	Task* verifyDocs = new Task("VERIFY_DOCS", "Verify Documents");
	addWorkItem(verifyDocs, intake);
	workItems["VERIFY_DOCS"] = verifyDocs;
	Task* feeStatus = new Task("FEE_STATUS", "Check Fee Status");
	addWorkItem(feeStatus, intake);
	workItems["FEE_STATUS"] = feeStatus;

	SubProcess* academic = new SubProcess("ACADEMIC", "Academic Review");
	addWorkItem(academic, root);
	workItems["ACADEMIC"] = academic;
	Task* facultyReview = new Task("FACULTY_REVIEW", "Faculty Review");
	addWorkItem(facultyReview, academic);
	workItems["FACULTY_REVIEW"] = facultyReview;
	Task* departmentReview = new Task("DEPARTMENT_REVIEW", "Department Review");
	addWorkItem(departmentReview, academic);
	workItems["DEPARTMENT_REVIEW"] = departmentReview;
	Task* decision = new Task("DECISION", "Admission Decision");
	addWorkItem(decision, academic);
	workItems["DECISION"] = decision;

	SubProcess* finance = new SubProcess("FINANCE", "Financial Assessment");
	addWorkItem(finance, root);
	workItems["FINANCE"] = finance;
	Task* bursaryReview = new Task("BURSARY_REVIEW", "Bursary Review");
	addWorkItem(bursaryReview, finance);
	workItems["BURSARY_REVIEW"] = bursaryReview;
	Task* feeClearance = new Task("FEE_CLEARANCE", "Fee Clearance");
	addWorkItem(feeClearance, finance);
	workItems["FEE_CLEARANCE"] = feeClearance;

	SubProcess* offer = new SubProcess("OFFER", "Offer and Registration");
	addWorkItem(offer, root);
	workItems["OFFER"] = offer;
	Task* issueOffer = new Task("ISSUE_OFFER", "Issue Offer");
	addWorkItem(issueOffer, offer);
	workItems["ISSUE_OFFER"] = issueOffer;
	Task* acceptOffer = new Task("ACCEPT_OFFER", "Accept Offer");
	addWorkItem(acceptOffer, offer);
	workItems["ACCEPT_OFFER"] = acceptOffer;
	Task* registerApplicant = new Task("REGISTER", "Registration");
	addWorkItem(registerApplicant, offer);
	workItems["REGISTER"] = registerApplicant;
	Task* confirmRegistration = new Task("CONFIRM_REGISTRATION", "Confirm Registration");
	addWorkItem(confirmRegistration, offer);
	workItems["CONFIRM_REGISTRATION"] = confirmRegistration;

	SubProcess* outcome = new SubProcess("OUTCOME", "Applicant Outcome");
	addWorkItem(outcome, root);
	workItems["OUTCOME"] = outcome;
	Task* notifyRejection = new Task("NOTIFY_REJECTION", "Notify Applicant of Rejection");
	addWorkItem(notifyRejection, outcome);
	workItems["NOTIFY_REJECTION"] = notifyRejection;
	Task* appeal = new Task("APPEAL", "Consider Appeal");
	addWorkItem(appeal, outcome);
	workItems["APPEAL"] = appeal;
}

// void UniversityApplicationDefinitionBuilder::buildDependencies()
// {
// 	addDependency(new DependencyRule("DECISION", "ISSUE_OFFER", "approved"));
// 	addDependency(new DependencyRule("DECISION", "NOTIFY_REJECTION", "rejected"));
// 	addDependency(new DependencyRule("NOTIFY_REJECTION", "APPEAL", "appeal"));
// 	addDependency(new DependencyRule("BURSARY_REVIEW", "FEE_CLEARANCE", "awarded"));
// 	addDependency(new DependencyRule("ACCEPT_OFFER", "REGISTER", "accepted"));
// 	addDependency(new DependencyRule("REGISTER", "CONFIRM_REGISTRATION", "enrolled"));
// }

// void UniversityApplicationDefinitionBuilder::buildRouting()
// {
// 	setRouting(findWorkItem("CAPTURE"), new RoleBasedRouting("AdmissionsOfficer"));
// 	setRouting(findWorkItem("VERIFY_IDENTITY"), new RoleBasedRouting("Registrar"));
// 	setRouting(findWorkItem("VERIFY_DOCS"), new RoleBasedRouting("Registrar"));
// 	setRouting(findWorkItem("FEE_STATUS"), new RoleBasedRouting("FinanceOfficer"));
// 	setRouting(findWorkItem("FACULTY_REVIEW"), new RoleBasedRouting("FacultyOfficer"));

// 	DataRuleRouting* facultyRouting = new DataRuleRouting("faculty", map<string, string>(), "FacultyOfficer");
// 	facultyRouting->addRule("science", "ScienceFacultyOfficer");
// 	facultyRouting->addRule("commerce", "CommerceFacultyOfficer");
// 	facultyRouting->addRule("humanities", "HumanitiesFacultyOfficer");
// 	setRouting(findWorkItem("DEPARTMENT_REVIEW"), facultyRouting);

// 	setRouting(findWorkItem("DECISION"), new RoleBasedRouting("Dean"));
// 	setRouting(findWorkItem("BURSARY_REVIEW"), new RoleBasedRouting("BursaryOfficer"));
// 	setRouting(findWorkItem("FEE_CLEARANCE"), new RoleBasedRouting("FinanceOfficer"));
// 	setRouting(findWorkItem("ISSUE_OFFER"), new RoleBasedRouting("AdmissionsOfficer"));
// 	setRouting(findWorkItem("ACCEPT_OFFER"), new RoleBasedRouting("AdmissionsOfficer"));
// 	setRouting(findWorkItem("REGISTER"), new RoleBasedRouting("Registrar"));
// 	setRouting(findWorkItem("CONFIRM_REGISTRATION"), new RoleBasedRouting("Registrar"));
// 	setRouting(findWorkItem("NOTIFY_REJECTION"), new RoleBasedRouting("AdmissionsOfficer"));
// 	setRouting(findWorkItem("APPEAL"), new RoleBasedRouting("Dean"));
// }

// void UniversityApplicationDefinitionBuilder::buildIntegrations()
// {
// 	setService(findWorkItem("VERIFY_IDENTITY"), "IdentityVerificationService");
// 	setService(findWorkItem("VERIFY_DOCS"), "DocumentVerificationService");
// 	setService(findWorkItem("FEE_STATUS"), "StudentAccountService");
// 	setService(findWorkItem("BURSARY_REVIEW"), "BursaryService");
// 	setService(findWorkItem("ISSUE_OFFER"), "NotificationService");
// 	setService(findWorkItem("NOTIFY_REJECTION"), "NotificationService");
// 	setService(findWorkItem("CONFIRM_REGISTRATION"), "RegistrationService");
// }

// void UniversityApplicationDefinitionBuilder::buildExtras()
// {
// 	markCheckpoint(findWorkItem("DECISION"));
// 	markCheckpoint(findWorkItem("ACCEPT_OFFER"));
// 	addPriority(*findWorkItem("FACULTY_REVIEW"), 2);
// 	addPriority(*findWorkItem("APPEAL"), 3);
// 	addAudit(findWorkItem("DECISION"));
// 	addAudit(findWorkItem("BURSARY_REVIEW"));
// 	addAudit(findWorkItem("ISSUE_OFFER"));
// 	addAudit(findWorkItem("NOTIFY_REJECTION"));
// }
