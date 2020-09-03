#ifndef PING_RESPONSE_H
#define PING_RESPONSE_H

#include <cstdint>

#include <QString>
#include <QMetaType>
#include <QJsonObject>

struct PingResponse {
    bool hasError;
    uint32_t errorCode;
    uint32_t roundTripTime;
    uint8_t ttl;
    QString target;

    QJsonObject toJsonObject(bool verbose);
};

Q_DECLARE_METATYPE(PingResponse)

#endif
