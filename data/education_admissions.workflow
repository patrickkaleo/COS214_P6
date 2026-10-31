# Education: Faculty Admissions and Bursary Processing.
# Sections and fields are described in FileWorkflowDefinitionBuilder.h.

DEFINITION
EDU-ADM|Faculty Admissions and Bursary Processing|1

WORKITEMS
process|ADMISSIONS|-|Faculty Admissions
process|INTAKE|ADMISSIONS|Application Intake
task|CAPTURE|INTAKE|Capture Application
task|VERIFY_ID|INTAKE|Verify Applicant Identity
task|REVIEW|ADMISSIONS|Academic Review
task|OFFER|ADMISSIONS|Issue Admission Offer
task|REGRET|ADMISSIONS|Send Regret Letter
task|BURSARY|ADMISSIONS|Approve Bursary Award
task|PAY|ADMISSIONS|Pay Bursary
task|CLOSE|ADMISSIONS|Close Application

DEPENDENCIES
CAPTURE|VERIFY_ID|captured
INTAKE|REVIEW|completed
REVIEW|OFFER|admit
REVIEW|REGRET|decline
OFFER|BURSARY|bursary requested
OFFER|CLOSE|no bursary
BURSARY|PAY|approved
BURSARY|CLOSE|declined
PAY|CLOSE|*
REGRET|CLOSE|*

ROUTING
CAPTURE|role|Admissions Clerk
VERIFY_ID|role|Admissions Clerk
REVIEW|data|faculty|Admissions Officer|Engineering|Engineering Reviewer|Humanities|Humanities Reviewer
OFFER|role|Admissions Officer
REGRET|role|Admissions Officer
BURSARY|approval|amount|Financial Aid Officer|50000|Faculty Manager|150000|Dean
PAY|role|Finance Clerk
CLOSE|role|Admissions Clerk

SERVICES
service|VERIFY_ID|IdentityVerificationService
service|PAY|PaymentGateway

EXTRAS
priority|BURSARY|2
audit|BURSARY
audit|PAY
deadline|BURSARY|48
