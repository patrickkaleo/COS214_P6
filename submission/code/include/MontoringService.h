#ifndef MONTORINGSERVICE_H
#define MONTORINGSERVICE_H

class MontoringService {

private:
	map<striing, int> counts;
	map<string, double> durations;

public:
	void onEvent(const WorkFLowEvent& event);

	string reposrt();
};

#endif
