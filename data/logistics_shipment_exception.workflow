# Logistics: Shipment Exception Handling.
# Sections and fields are described in FileWorkflowDefinitionBuilder.h.

DEFINITION
LOG-EXC|Shipment Exception Handling|1

WORKITEMS
process|EXCEPTION|-|Shipment Exception
task|LOG|EXCEPTION|Log Exception
task|TRACK|EXCEPTION|Check Courier Tracking
task|ASSESS|EXCEPTION|Assess Exception
process|RECOVERY|EXCEPTION|Recover Shipment
task|REROUTE|RECOVERY|Rebook With Carrier
task|NOTIFY|RECOVERY|Notify Customer
task|CLAIM|EXCEPTION|File Damage Claim
task|CLOSE|EXCEPTION|Close Exception

DEPENDENCIES
LOG|TRACK|logged
TRACK|ASSESS|*
ASSESS|RECOVERY|reroute
ASSESS|CLAIM|write off
RECOVERY|CLOSE|completed
CLAIM|CLOSE|*

ROUTING
LOG|role|Warehouse Clerk
TRACK|role|Warehouse Clerk
ASSESS|data|severity|Exception Analyst|low|Warehouse Supervisor|high|Exception Analyst|critical|Operations Manager
REROUTE|role|Exception Analyst
NOTIFY|role|Customer Liaison
CLAIM|role|Claims Handler
CLOSE|role|Warehouse Supervisor

SERVICES
service|TRACK|TrackingService
service|REROUTE|CarrierAdapter

EXTRAS
priority|REROUTE|3
audit|REROUTE
deadline|ASSESS|24
