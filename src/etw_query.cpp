#include "etw_query.h"

#include <iostream>

EtwQuery::EtwQuery() : mTraceSession(), mTraceConsumer(true, true), mEtwProcessThread(), mSessionName("zeusDebug2"), mIsSessionStarted(false) {
	//
}

EtwQuery::~EtwQuery() {
	// Stop the trace session.
	mTraceSession.Stop();
	TraceSession::StopNamedSession(mSessionName.c_str());
	mIsSessionStarted = false;

	mEtwProcessThread.requestInterruption();
	mEtwProcessThread.quit();
	mEtwProcessThread.wait();
}

bool EtwQuery::startTraceSession(uint64_t interestingPid) {
	if (mIsSessionStarted) {
		mTraceSession.Stop();
		TraceSession::StopNamedSession(mSessionName.c_str());
		mIsSessionStarted = false;

		mEtwProcessThread.requestInterruption();
		mEtwProcessThread.quit();
		mEtwProcessThread.wait();
		std::cerr << "Stopped last session before starting new session." << std::endl;
	}

	mTraceConsumer.setInterestingProcessId(interestingPid);
	auto status = mTraceSession.Start(&mTraceConsumer, mSessionName.c_str());

    if (status == ERROR_ALREADY_EXISTS) {
		std::cerr << "Warning: a trace session named \"" << mSessionName << "\" is already running and it will be stopped." << std::endl;

        status = TraceSession::StopNamedSession(mSessionName.c_str());
        if (status == ERROR_SUCCESS) {
            status = mTraceSession.Start(&mTraceConsumer, mSessionName.c_str());
        }
    }

	if (status != ERROR_SUCCESS) {
		std::cerr << "Failed to start trace session" << std::endl;
		return false;
	}
	mIsSessionStarted = true;

	mEtwProcessThread.setTraceHandle(mTraceSession.mTraceHandle);
	mEtwProcessThread.start(QThread::TimeCriticalPriority);

	return true;
}
