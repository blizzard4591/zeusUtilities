#ifndef PING_H
#define PING_H

#include <cstdint>

#include <QByteArray>
#include <QString>
#include <QMetaType>

class Ping {
public:
    Ping();
    virtual ~Ping();

    struct PingResponse {
        bool hasError;
        uint32_t errorCode;
        uint32_t roundTripTime;
    };

    bool ping(QString const& ip, int timeout, PingResponse& pingResponse);

signals:
    void pingDone(uint64_t pingId, Ping::PingResponse pingResponse);

public slots:
    void doPing(uint64_t pingId, QString const& targetIp, int timeout);

private:
    QByteArray sendBuffer;
    QByteArray replyBuffer;

    uint64_t counter;
};

Q_DECLARE_METATYPE(Ping::PingResponse)

#endif // PING_H
