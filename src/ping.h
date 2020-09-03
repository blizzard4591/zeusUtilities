#ifndef PING_H
#define PING_H

#include <cstdint>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QMetaType>

#include "ping_response.h"

class Ping : public QObject {
    Q_OBJECT
public:
    Ping(QString const& target, quint64 pingId, int timeout, QObject* parent = nullptr);
    virtual ~Ping();

    bool ping(PingResponse& pingResponse);
    quint64 getPingId() const { return mPingId; }
signals:
    void pingDone(quint64 roundId, quint64 pingId, PingResponse pingResponse);

public slots:
    void doPing(quint64 roundId);
private:
    quint64 const mPingId;
    QString const mTarget;
    QString const mTargetIp;
    int const mTimeout;

    QByteArray mSendBuffer;
    QByteArray mReplyBuffer;

    quint64 mCounter;

    static QString resolveHostname(QString const& hostname);
};

#endif // PING_H
