#include "LogisticsDefinitionBuilder.h"

LogisticsDefinitionBuilder::LogisticsDefinitionBuilder(string id, string name, int version)
	: WorkflowDefinitionBuilder(id, name, version)
{
}

void LogisticsDefinitionBuilder::buildDefinition()
{
	WorkItem* root = new SubProcess("SHIP_EX", "Shipment Exception");
	definition->setRoot(root);

	WorkItem* report = new SubProcess("REPORT", "Exception Report");
	addWorkItem(report, root);
	WorkItem* logException = new Task("LOG_EXCEPTION", "Log Exception");
	addWorkItem(logException, report);
	WorkItem* classify = new Task("CLASSIFY", "Classify Severity");
	addWorkItem(classify, report);
	WorkItem* collectEvidence = new Task("COLLECT_EVIDENCE", "Collect Evidence");
	addWorkItem(collectEvidence, report);

	WorkItem* investigate = new SubProcess("INVESTIGATE", "Investigation");
	addWorkItem(investigate, root);
	WorkItem* assess = new Task("ASSESS", "Assess Impact");
	addWorkItem(assess, investigate);
	WorkItem* customsCheck = new Task("CUSTOMS_CHECK", "Customs Check");
	addWorkItem(customsCheck, investigate);
	WorkItem* carrierQuery = new Task("CARRIER_QUERY", "Query Carrier");
	addWorkItem(carrierQuery, investigate);

	WorkItem* handle = new SubProcess("HANDLE", "Exception Handling");
	addWorkItem(handle, root);
	WorkItem* reroute = new Task("REROUTE", "Reroute Shipment");
	addWorkItem(reroute, handle);
	WorkItem* hold = new Task("HOLD", "Hold Shipment");
	addWorkItem(hold, handle);
	WorkItem* returnToSender = new Task("RETURN_TO_SENDER", "Return To Sender");
	addWorkItem(returnToSender, handle);
	WorkItem* replaceShipment = new Task("REPLACE_SHIPMENT", "Replace Shipment");
	addWorkItem(replaceShipment, handle);

	WorkItem* close = new SubProcess("CLOSE", "Case Closure");
	addWorkItem(close, root);
	WorkItem* notifyCustomer = new Task("NOTIFY_CUSTOMER", "Notify Customer");
	addWorkItem(notifyCustomer, close);
	WorkItem* claimRefund = new Task("CLAIM_REFUND", "Claim Refund");
	addWorkItem(claimRefund, close);
	WorkItem* closeCase = new Task("CLOSE_CASE", "Close Case");
	addWorkItem(closeCase, close);

	addDependency(new DependencyRule("CLASSIFY", "CUSTOMS_CHECK", "customs"));
	addDependency(new DependencyRule("ASSESS", "REROUTE", "reroute"));
	addDependency(new DependencyRule("ASSESS", "HOLD", "hold"));
	addDependency(new DependencyRule("ASSESS", "RETURN_TO_SENDER", "return"));
	addDependency(new DependencyRule("ASSESS", "REPLACE_SHIPMENT", "replace"));
	addDependency(new DependencyRule("HOLD", "CLAIM_REFUND", "damaged"));
	addDependency(new DependencyRule("REROUTE", "NOTIFY_CUSTOMER", "dispatched"));
	addDependency(new DependencyRule("NOTIFY_CUSTOMER", "CLOSE_CASE", "acknowledged"));

	setRouting(logException, new RoleBasedRouting("WarehouseClerk"));

	DataRuleRouting* severityRouting = new DataRuleRouting("severity", map<string, string>(), "ExceptionAnalyst");
	severityRouting->addRule("low", "WarehouseClerk");
	severityRouting->addRule("high", "ExceptionAnalyst");
	severityRouting->addRule("critical", "LogisticsCoordinator");
	setRouting(classify, severityRouting);

	setRouting(collectEvidence, new RoleBasedRouting("WarehouseClerk"));
	setRouting(assess, new RoleBasedRouting("LogisticsCoordinator"));
	setRouting(customsCheck, new RoleBasedRouting("CustomsOfficer"));
	setRouting(carrierQuery, new RoleBasedRouting("Dispatcher"));
	setRouting(reroute, new RoleBasedRouting("Dispatcher"));
	setRouting(hold, new RoleBasedRouting("WarehouseSupervisor"));
	setRouting(returnToSender, new RoleBasedRouting("LogisticsCoordinator"));
	setRouting(replaceShipment, new RoleBasedRouting("WarehouseSupervisor"));
	setRouting(notifyCustomer, new RoleBasedRouting("CustomerService"));
	setRouting(claimRefund, new RoleBasedRouting("FinanceOfficer"));
	setRouting(closeCase, new RoleBasedRouting("LogisticsCoordinator"));

	setService(classify, "TrackingService");
	setService(carrierQuery, "CarrierAdapter");
	setService(customsCheck, "CustomsAdapter");
	setService(reroute, "CarrierAdapter");
	setService(notifyCustomer, "NotificationService");
	setService(claimRefund, "ClaimsService");

	markCheckpoint(assess);
	markCheckpoint(closeCase);
	addPriority(reroute, 3);
	addPriority(hold, 2);
	addPriority(replaceShipment, 3);
	addAudit(reroute);
	addAudit(notifyCustomer);
	addAudit(claimRefund);
	addAudit(closeCase);
}
