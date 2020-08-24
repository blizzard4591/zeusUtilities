#include "pingthread.h"

#include "ping.h"

#include <QEventLoop>
#include <QTimer>

#include <iostream>

PingThread::PingThread(QString const& targetIp, QObject* parent) : QThread(parent), mTargetIp(targetIp) {
	//
}

PingThread::~PingThread() {
	//
}

void PingThread::run() {
	mPing = new Ping();

	QEventLoop eventLoop;

	delete mPing;
}

void PingThread::doPing(uint64_t pingId, int timeout) {
	if (mPing != nullptr) {
		Ping::PingResponse result;
		mPing->ping(mTargetIp, result);

		emit pingDone(pingId, result);
	}
}
