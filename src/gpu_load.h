#ifndef GPULOAD_H
#define GPULOAD_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <QDateTime>
#include <QString>
#include <QThread>

#include "gpu_info.h"
#include "gpu_query_thread.h"

class GpuLoad {
public:
    GpuLoad();
    virtual ~GpuLoad();

    void update();
    
    void start();
    void stop();

    std::unordered_map<void*, GpuInfo> getCurrentGpuLoad() const;
private:
    GpuQueryThread mGpuQueryThread;
};

#endif
