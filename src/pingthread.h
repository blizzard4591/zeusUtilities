#ifndef PINGTHREAD_H
#define PINGTHREAD_H

#include <cstdint>
#include <QThread>
#include <QObject>

#include "ping.h"

class PingThread : QThread {
public:
	PingThread(QString const& targetIp, QObject* parent = nullptr);
	virtual ~PingThread();

signals:
	void pingDone(uint64_t pingId, Ping::PingResponse pingResponse);

public slots:
	void doPing(uint64_t pingId, int timeout);

protected:
	virtual void run() override;
private:
	QString const mTargetIp;
	Ping* mPing;
};

#endif