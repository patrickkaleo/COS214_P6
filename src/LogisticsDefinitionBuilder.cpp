#include "LogisticsDefinitionBuilder.h"
LogisticsDefinitionBuilder::LogisticsDefinitionBuilder( string id, int version) 
{
	this->definition->setId(id);
	this->definition->setName("Logistics Workflow");
	this->definition->setVersion(version);
}

void LogisticsDefinitionBuilder::buildStructure()
{
	addSubProcess("SHIP_EX", "Shipment Exception", "");

	addSubProcess("REPORT", "Exception Report", "SHIP_EX");
	addTask("LOG_EXCEPTION", "Log Exception", "REPORT");
	addTask("CLASSIFY", "Classify Severity", "REPORT");

	addSubProcess("HANDLE", "Exception Handling", "SHIP_EX");
	addTask("ASSESS", "Assess Impact", "HANDLE");
	addTask("REROUTE", "Reroute Shipment", "HANDLE");
	addTask("HOLD", "Hold Shipment", "HANDLE");

	addSubProcess("CLOSE", "Case Closure", "SHIP_EX");
	addTask("NOTIFY_CUSTOMER", "Notify Customer", "CLOSE");
	addTask("CLOSE_CASE", "Close Case", "CLOSE");
}

void LogisticsDefinitionBuilder::buildDependencies()
{
	vector<string> intakeOrder;
	intakeOrder.push_back("LOG_EXCEPTION");
	intakeOrder.push_back("CLASSIFY");
	intakeOrder.push_back("ASSESS");
	// addSequence(intakeOrder);

	addDependency("ASSESS", "REROUTE", "reroute");
	addDependency("ASSESS", "HOLD", "hold");
	addDependency("REROUTE", "NOTIFY_CUSTOMER", "*");
	addDependency("HOLD", "NOTIFY_CUSTOMER", "*");
	addDependency("NOTIFY_CUSTOMER", "CLOSE_CASE", "*");
}

void LogisticsDefinitionBuilder::buildRouting()
{
	setRouting("LOG_EXCEPTION", new RoleBasedRouting("WarehouseClerk"));
	setRouting("CLASSIFY", new RoleBasedRouting("ExceptionAnalyst"));
	setRouting("ASSESS", new RoleBasedRouting("LogisticsCoordinator"));
	setRouting("REROUTE", new RoleBasedRouting("Dispatcher"));
	setRouting("HOLD", new RoleBasedRouting("WarehouseSupervisor"));
	setRouting("NOTIFY_CUSTOMER", new RoleBasedRouting("CustomerService"));
	setRouting("CLOSE_CASE", new RoleBasedRouting("LogisticsCoordinator"));
}

void LogisticsDefinitionBuilder::buildIntegrations()
{
	setService("CLASSIFY", "TrackingService");
	setService("REROUTE", "CarrierAdapter");
	setService("NOTIFY_CUSTOMER", "NotificationService");
}

void LogisticsDefinitionBuilder::buildExtras()
{
	markCheckpoint("ASSESS");
	addPriority("REROUTE", 3);
	addPriority("HOLD", 2);
	addAudit("NOTIFY_CUSTOMER");
	addAudit("CLOSE_CASE");
}
