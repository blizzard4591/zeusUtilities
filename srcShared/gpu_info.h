#ifndef GPU_INFO_H
#define GPU_INFO_H

#include <cstdint>
#include <iostream>

#include <QJsonObject>
#include <QString>

class GpuInfo {
public:
    GpuInfo() : utilization(0.0), dedicatedMemory(0), sharedMemory(0), commitMemory(0) {}

    double utilization;
    uint64_t dedicatedMemory;
    uint64_t sharedMemory;
    uint64_t commitMemory;
    
    QJsonObject toJsonObject(bool verbose) const;
    static GpuInfo fromJsonObject(QJsonObject const& object, bool* okay);

    friend std::ostream& operator<<(std::ostream& os, const GpuInfo& gi);
};

#endif
