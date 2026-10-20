#include "DependencyRule.h"
using namespace std;

DependencyRule::DependencyRule(string fromId, string toId, string outcome) : fromId(fromId), toId(toId), outcome(outcome) {}

string DependencyRule::getFrom() { return this->fromId; }

string DependencyRule::getOutcome() { return this->fromId; }

string DependencyRule::getTo() { return this->fromId; }

void DependencyRule::setOutcome(string outcome) { this->outcome = outcome; }