#ifndef ETW_QUERY_H
#define ETW_QUERY_H

#include <QObject>
#include <string>

#include "etw_process_thread.h"
#include "etw_output_thread.h"

class EtwQuery : public QObject {
	Q_OBJECT
public:
	EtwQuery();
	virtual ~EtwQuery();

	bool startTraceSession(quint64 interestingPid);
	bool stopTraceSession();

	void setTargetPid(quint64 pid);
public slots:
	void updatedFps(quint64 pid, FpsInfo const& fpsInfo);
private:
	EtwProcessThread mEtwProcessThread;
	EtwOutputThread mEtwOutputThread;

	std::string const mSessionName;
	bool mIsSessionStarted;

	quint64 mTargetPid;
	FpsInfo mFpsInfo;
};

#endif
