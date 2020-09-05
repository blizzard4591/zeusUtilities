#include "etw_query.h"

#include <iostream>

#include "etw_trace_consumer.h"
#include "etw_trace_session.h"

TraceSession gSession;
static PMTraceConsumer* gPMConsumer = nullptr;

void CheckLostReports(ULONG* eventsLost, ULONG* buffersLost) {
	auto status = gSession.CheckLostReports(eventsLost, buffersLost);
	(void)status;
}

void DequeueAnalyzedInfo(
	std::vector<ProcessEvent>* processEvents,
	std::vector<std::shared_ptr<PresentEvent>>* presentEvents) {
	gPMConsumer->DequeueProcessEvents(*processEvents);
	gPMConsumer->DequeuePresentEvents(*presentEvents);
}

double QpcDeltaToSeconds(uint64_t qpcDelta) {
	return (double)qpcDelta / gSession.mQpcFrequency.QuadPart;
}

uint64_t SecondsDeltaToQpc(double secondsDelta) {
	return (uint64_t)(secondsDelta * gSession.mQpcFrequency.QuadPart);
}

double QpcToSeconds(uint64_t qpc) {
	return QpcDeltaToSeconds(qpc - gSession.mStartQpc.QuadPart);
}


EtwQuery::EtwQuery() : mEtwProcessThread(), mEtwOutputThread(), mSessionName("zeusDebug"), mIsSessionStarted(false), mTargetPid(0), mFpsInfo() {
	//
	if (!QObject::connect(&mEtwOutputThread, SIGNAL(updatedFps(quint64, FpsInfo const&)), this, SLOT(updatedFps(quint64, FpsInfo const&)), Qt::QueuedConnection)) {
		throw;
	}
}

EtwQuery::~EtwQuery() {
	// Stop the trace session.
	gSession.Stop();
	TraceSession::StopNamedSession(mSessionName.c_str());
	mIsSessionStarted = false;

	mEtwProcessThread.requestInterruption();
	mEtwOutputThread.requestInterruption();

	mEtwProcessThread.quit();
	mEtwProcessThread.wait();

	mEtwOutputThread.quit();
	mEtwOutputThread.wait();
}

bool EtwQuery::startTraceSession(quint64 interestingPid) {
	stopTraceSession();

	bool const simple = false;
	gPMConsumer = new PMTraceConsumer(true, simple);

	setTargetPid(interestingPid);

	auto status = gSession.Start(gPMConsumer, mSessionName.c_str());

    if (status == ERROR_ALREADY_EXISTS) {
		std::cerr << "Warning: a trace session named \"" << mSessionName << "\" is already running and it will be stopped." << std::endl;

        status = TraceSession::StopNamedSession(mSessionName.c_str());
        if (status == ERROR_SUCCESS) {
            status = gSession.Start(gPMConsumer, mSessionName.c_str());
        }
    }

	if (status != ERROR_SUCCESS) {
		std::cerr << "Failed to start trace session" << std::endl;
		
		delete gPMConsumer;
		gPMConsumer = nullptr;

		return false;
	}
	mIsSessionStarted = true;

	QThread::msleep(50);
	mEtwProcessThread.setTraceHandle(gSession.mTraceHandle);
	mEtwProcessThread.start(QThread::TimeCriticalPriority);

	QThread::msleep(50);
	mEtwOutputThread.start();
	QThread::msleep(250);

	// Tell OutputThread to record
	mEtwOutputThread.setOutputRecordingState(true);

	return true;
}

bool EtwQuery::stopTraceSession() {
	if (mIsSessionStarted) {
		gSession.Stop();
		TraceSession::StopNamedSession(mSessionName.c_str());
		mIsSessionStarted = false;

		mEtwProcessThread.requestInterruption();
		mEtwOutputThread.requestInterruption();

		mEtwProcessThread.quit();
		mEtwProcessThread.wait();

		mEtwOutputThread.quit();
		mEtwOutputThread.wait();
	}
	return true;
}

void EtwQuery::setTargetPid(quint64 pid) {
	if (mTargetPid != pid) {
#ifndef NDEBUG
		std::cout << "Switching target pid on EtwQuery from " << mTargetPid << " to " << pid << "." << std::endl;
#endif
		mFpsInfo = FpsInfo();
		mTargetPid = pid;

		if (gPMConsumer != nullptr) {
			gPMConsumer->setInterestingProcessId(mTargetPid);
		}
		mEtwOutputThread.setTargetPid(mTargetPid);
	}
}

void EtwQuery::updatedFps(quint64 pid, FpsInfo const& fpsInfo) {
	if (mTargetPid == pid) {
		mFpsInfo = fpsInfo;
	}
}
