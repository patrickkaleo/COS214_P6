#include "UniversityAplicationDefinitionBuilder.h"

UniversityApplicationDefinitionBuilder::UniversityApplicationDefinitionBuilder(string id, string name, int version)
	: WorkflowDefinitionBuilder(id, name, version)
{
}

void UniversityApplicationDefinitionBuilder::buildDefinition()
{
	// create the relevant work items 
	WorkItem* root = new SubProcess("UNI_APP", "University Application");
	definition->setRoot(root);

	WorkItem* intake = new SubProcess("INTAKE", "Application Intake");
	addWorkItem(intake, root);
	WorkItem* capture = new Task("CAPTURE", "Capture Application");
	addWorkItem(capture, intake);
	WorkItem* verifyIdentity = new Task("VERIFY_IDENTITY", "Verify Identity");
	addWorkItem(verifyIdentity, intake);
	WorkItem* verifyDocs = new Task("VERIFY_DOCS", "Verify Documents");
	addWorkItem(verifyDocs, intake);
	WorkItem* feeStatus = new Task("FEE_STATUS", "Check Fee Status");
	addWorkItem(feeStatus, intake);

	WorkItem* academic = new SubProcess("ACADEMIC", "Academic Review");
	addWorkItem(academic, root);
	WorkItem* facultyReview = new Task("FACULTY_REVIEW", "Faculty Review");
	addWorkItem(facultyReview, academic);
	WorkItem* departmentReview = new Task("DEPARTMENT_REVIEW", "Department Review");
	addWorkItem(departmentReview, academic);
	WorkItem* decision = new Task("DECISION", "Admission Decision");
	addWorkItem(decision, academic);

	WorkItem* finance = new SubProcess("FINANCE", "Financial Assessment");
	addWorkItem(finance, root);
	WorkItem* bursaryReview = new Task("BURSARY_REVIEW", "Bursary Review");
	addWorkItem(bursaryReview, finance);
	WorkItem* feeClearance = new Task("FEE_CLEARANCE", "Fee Clearance");
	addWorkItem(feeClearance, finance);

	WorkItem* offer = new SubProcess("OFFER", "Offer and Registration");
	addWorkItem(offer, root);
	WorkItem* issueOffer = new Task("ISSUE_OFFER", "Issue Offer");
	addWorkItem(issueOffer, offer);
	WorkItem* acceptOffer = new Task("ACCEPT_OFFER", "Accept Offer");
	addWorkItem(acceptOffer, offer);
	WorkItem* registerApplicant = new Task("REGISTER", "Registration");
	addWorkItem(registerApplicant, offer);
	WorkItem* confirmRegistration = new Task("CONFIRM_REGISTRATION", "Confirm Registration");
	addWorkItem(confirmRegistration, offer);

	WorkItem* outcome = new SubProcess("OUTCOME", "Applicant Outcome");
	addWorkItem(outcome, root);
	WorkItem* notifyRejection = new Task("NOTIFY_REJECTION", "Notify Applicant of Rejection");
	addWorkItem(notifyRejection, outcome);
	WorkItem* appeal = new Task("APPEAL", "Consider Appeal");
	addWorkItem(appeal, outcome);

	// add dependencies to the work items 
	addDependency(new DependencyRule("DECISION", "ISSUE_OFFER", "approved"));
	addDependency(new DependencyRule("DECISION", "NOTIFY_REJECTION", "rejected"));
	addDependency(new DependencyRule("NOTIFY_REJECTION", "APPEAL", "appeal"));
	addDependency(new DependencyRule("BURSARY_REVIEW", "FEE_CLEARANCE", "awarded"));
	addDependency(new DependencyRule("ACCEPT_OFFER", "REGISTER", "accepted"));
	addDependency(new DependencyRule("REGISTER", "CONFIRM_REGISTRATION", "enrolled"));

	// swt routing for the different tasks 
	setRouting(capture, new RoleBasedRouting("AdmissionsOfficer"));
	setRouting(verifyIdentity, new RoleBasedRouting("Registrar"));
	setRouting(verifyDocs, new RoleBasedRouting("Registrar"));
	setRouting(feeStatus, new RoleBasedRouting("FinanceOfficer"));
	setRouting(facultyReview, new RoleBasedRouting("FacultyOfficer"));

	DataRuleRouting* facultyRouting = new DataRuleRouting("faculty", map<string, string>(), "FacultyOfficer");
	facultyRouting->addRule("science", "ScienceFacultyOfficer");
	facultyRouting->addRule("commerce", "CommerceFacultyOfficer");
	facultyRouting->addRule("humanities", "HumanitiesFacultyOfficer");
	setRouting(departmentReview, facultyRouting);

	setRouting(decision, new RoleBasedRouting("Dean"));
	setRouting(bursaryReview, new RoleBasedRouting("BursaryOfficer"));
	setRouting(feeClearance, new RoleBasedRouting("FinanceOfficer"));
	setRouting(issueOffer, new RoleBasedRouting("AdmissionsOfficer"));
	setRouting(acceptOffer, new RoleBasedRouting("AdmissionsOfficer"));
	setRouting(registerApplicant, new RoleBasedRouting("Registrar"));
	setRouting(confirmRegistration, new RoleBasedRouting("Registrar"));
	setRouting(notifyRejection, new RoleBasedRouting("AdmissionsOfficer"));
	setRouting(appeal, new RoleBasedRouting("Dean"));

	//add services 
	setService(verifyIdentity, "IdentityVerificationService");
	setService(verifyDocs, "DocumentVerificationService");
	setService(feeStatus, "StudentAccountService");
	setService(bursaryReview, "BursaryService");
	setService(issueOffer, "NotificationService");
	setService(notifyRejection, "NotificationService");
	setService(confirmRegistration, "RegistrationService");

	// mark as checkpoint for certain work items 
	markCheckpoint(decision);
	markCheckpoint(acceptOffer);
	addPriority(facultyReview, 2);
	addPriority(appeal, 3);
	addAudit(decision);
	addAudit(bursaryReview);
	addAudit(issueOffer);
	addAudit(notifyRejection);
}
