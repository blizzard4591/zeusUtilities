#include "cb_object.h"

#include "mainwindow.h"

#include <iostream>

extern "C" {

	void callbackTest(int id) {
		CbObject* const cbObject = CbObject::getObject();
		if (cbObject != nullptr) {
			cbObject->callbackTest(id);
		}
	}

}

MainWindow* CbObject::mMainWindow = nullptr;
CbObject* CbObject::mSelfReference = nullptr;

CbObject::CbObject() : QObject(), mTimer(), mCbCallCount(0), mCbCallCountLast(0), mIsUpdated(false), mMissedFrameCount(0) {
	QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
	mTimer.setSingleShot(false);
	mTimer.setInterval(1000);
	//mTimer.start();
}

CbObject::~CbObject() {
	mTimer.stop();
}

void CbObject::onTimerTimeout() {
	if (mMainWindow == nullptr || mSelfReference == nullptr) {
		return;
	}

	if (mCbCallCount == mCbCallCountLast) {
		mIsUpdated = false;
		++mMissedFrameCount;
	} else {
		mCbCallCountLast = mCbCallCount;
		mIsUpdated = true;
	}
	if (!QMetaObject::invokeMethod(mMainWindow, "setProcessUpdatesState", Qt::QueuedConnection, Q_ARG(bool, mIsUpdated), Q_ARG(quint64, mMissedFrameCount))) {
		std::cerr << "Failed to invoke callback target." << std::endl;
	}
}

void CbObject::callbackTest(int id) {
	if (mMainWindow == nullptr) {
		return;
	}

	++mCbCallCount;

	if (!QMetaObject::invokeMethod(mMainWindow, "incrementCounter", Qt::QueuedConnection)) {
		std::cerr << "Failed to invoke callback target." << std::endl;
	}
}
