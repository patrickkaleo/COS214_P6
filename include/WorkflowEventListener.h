#ifndef WORKFLOWEVENTLISTENER_H
#define WORKFLOWEVENTLISTENER_H

class WorkflowEventListener {


public:
	virtual void onEvent(int eventCode, int sourceID) = 0;
};

#endif
