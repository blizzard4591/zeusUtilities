#ifndef CB_OBJECT_H
#define CB_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

	void callbackTest(int id);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <QObject>
#include <QTimer>

class MainWindow;

class CbObject : public QObject {
	Q_OBJECT
public:
	CbObject();
	virtual ~CbObject();

	static CbObject* getObject() { return mSelfReference; }

	static void setMainWindow(MainWindow* mainWindow, CbObject* selfReference) {
		mMainWindow = mainWindow;
		mSelfReference = selfReference;
	}

	void callbackTest(int id);

	quint64 getCurrentMissedFrameCount() const { return mMissedFrameCount; }
public slots:
	void onTimerTimeout();
private:
	static MainWindow* mMainWindow;
	static CbObject* mSelfReference;
	
	QTimer mTimer;
	quint64 mCbCallCount;
	quint64 mCbCallCountLast;
	bool mIsUpdated;
	quint64 mMissedFrameCount;

};
#endif

#endif
