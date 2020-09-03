#include "etw_process_thread.h"

#include <iostream>

EtwProcessThread::EtwProcessThread() : mTraceHandle(0) {
	//
}

EtwProcessThread::~EtwProcessThread() {
	//
}

void EtwProcessThread::setTraceHandle(TRACEHANDLE traceHandle) {
	mTraceHandle = traceHandle;
}

void EtwProcessThread::run() {
	if (this->priority() != QThread::TimeCriticalPriority) {
		this->setPriority(QThread::TimeCriticalPriority);
	}

	auto status = ProcessTrace(&mTraceHandle, 1, NULL, NULL);
	(void)status;

#ifndef NDEBUG
	std::cerr << "ETW Thread terminated." << std::endl;
#endif // NDEBUG
}
