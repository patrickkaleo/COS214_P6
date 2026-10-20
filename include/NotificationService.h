#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

class NotificationService {

private:
	string channel;

public:
	void onEvent(string eventCode, string sourceId);
};

#endif
