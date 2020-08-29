#ifndef PING_H
#define PING_H

#include <cstdint>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QMetaType>

class Ping : public QObject {
    Q_OBJECT
public:
    Ping(QString const& target, int timeout, QObject* parent = nullptr);
    virtual ~Ping();

    struct PingResponse {
        bool hasError;
        uint32_t errorCode;
        uint32_t roundTripTime;
        uint8_t ttl;
        QString target;
    };

    bool ping(PingResponse& pingResponse);
signals:
    void pingDone(quint64 roundId, quint64 pingId, Ping::PingResponse pingResponse);

public slots:
    void doPing(quint64 roundId, quint64 pingId);

private:
    QString const mTarget;
    QString const mTargetIp;
    int const mTimeout;

    QByteArray mSendBuffer;
    QByteArray mReplyBuffer;

    quint64 mCounter;

    static QString resolveHostname(QString const& hostname);
};

Q_DECLARE_METATYPE(Ping::PingResponse)

#endif // PING_H
