#ifndef GPULOAD_H
#define GPULOAD_H

#include <cstdint>
#include <vector>

#include <QString>
#include <QThread>
#include "phthread.h"

class GpuLoad {
public:
    GpuLoad();
    virtual ~GpuLoad();

    void update();
    double getGpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;
private:
    uint64_t mIterationCount;
    PhThread mPhThread;
    
};

#endif
