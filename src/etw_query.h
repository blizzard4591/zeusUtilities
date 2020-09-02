#ifndef ETW_QUERY_H
#define ETW_QUERY_H

#include <QObject>
#include <string>

#include "etw_trace_consumer.h"
#include "etw_trace_session.h"
#include "etw_process_thread.h"

class EtwQuery : public QObject {
	Q_OBJECT
public:
	EtwQuery();
	virtual ~EtwQuery();

	bool startTraceSession(uint64_t interestingPid);
private:
	TraceSession mTraceSession;
	PMTraceConsumer mTraceConsumer;
	EtwProcessThread mEtwProcessThread;

	std::string const mSessionName;
	bool mIsSessionStarted;
};

#endif
