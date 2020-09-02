#ifndef ETW_PROCESS_THREAD_H
#define ETW_PROCESS_THREAD_H

#include <QThread>

#include <windows.h>
#include <evntcons.h> // must include after windows.h

class EtwProcessThread : public QThread {
	Q_OBJECT
public:
	EtwProcessThread();
	virtual ~EtwProcessThread();

	void setTraceHandle(TRACEHANDLE traceHandle);
	virtual void run() override;
private:
	TRACEHANDLE mTraceHandle;
};

#endif
