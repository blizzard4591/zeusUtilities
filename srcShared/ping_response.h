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
    static PingResponse fromJsonObject(QJsonObject const& object, bool* okay);
private:
    static const QString TAG_HASERROR_L; static const QString TAG_HASERROR_S;
    static const QString TAG_ERRORCODE_L; static const QString TAG_ERRORCODE_S;
    static const QString TAG_RTT_L; static const QString TAG_RTT_S;
    static const QString TAG_TTL_L; static const QString TAG_TTL_S;
    static const QString TAG_TARGET_L; static const QString TAG_TARGET_S;
};

Q_DECLARE_METATYPE(PingResponse)

#endif
