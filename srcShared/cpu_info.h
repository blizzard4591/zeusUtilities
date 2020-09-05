#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <cstdint>
#include <iostream>

#include <QJsonObject>
#include <QString>

class CpuInfo {
public:
    CpuInfo() : userTimeDelta(0), kernelTimeDelta(0), idleTimeDelta(0), 
        memoryLoad(0), memoryTotal(0), memoryAvail(0), pageTotal(0), pageAvail(0), virtualTotal(0), virtualAvail(0) {}

    uint64_t userTimeDelta;
    uint64_t kernelTimeDelta;
    uint64_t idleTimeDelta;

    qint64 memoryLoad;
    qint64 memoryTotal;
    qint64 memoryAvail;
    qint64 pageTotal;
    qint64 pageAvail;
    qint64 virtualTotal;
    qint64 virtualAvail;

    QJsonObject toJsonObject(bool verbose) const;
    static CpuInfo fromJsonObject(QJsonObject const& object, bool* okay);
    
    friend std::ostream& operator<<(std::ostream& os, const CpuInfo& gi);
private:
    static const QString TAG_USERDELTA_L; static const QString TAG_USERDELTA_S;
    static const QString TAG_KERNELDELTA_L; static const QString TAG_KERNELDELTA_S;
    static const QString TAG_IDLEDELTA_L; static const QString TAG_IDLEDELTA_S;

    static const QString TAG_MEMORYLOAD_L; static const QString TAG_MEMORYLOAD_S;
    static const QString TAG_MEMORYTOTAL_L; static const QString TAG_MEMORYTOTAL_S;
    static const QString TAG_MEMORYAVAIL_L; static const QString TAG_MEMORYAVAIL_S;
    static const QString TAG_PAGETOTAL_L; static const QString TAG_PAGETOTAL_S;
    static const QString TAG_PAGEAVAIL_L; static const QString TAG_PAGEAVAIL_S;
    static const QString TAG_VIRTUALTOTAL_L; static const QString TAG_VIRTUALTOTAL_S;
    static const QString TAG_VIRTUALAVAIL_L; static const QString TAG_VIRTUALAVAIL_S;
};

#endif
