#include "LogisticsDefinitionBuilder.h"

LogisticsDefinitionBuilder::LogisticsDefinitionBuilder(string id, string name, int version)
	: WorkflowDefinitionBuilder(id, name, version)
{
}

void LogisticsDefinitionBuilder::buildDefinition()
{
	// build the structure and their task with the relevant pririties and all 
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
}
