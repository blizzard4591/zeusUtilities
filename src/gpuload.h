#ifndef GPULOAD_H
#define GPULOAD_H

#include <cstdint>
#include <vector>

#include <QString>

class GpuLoad {
public:
    GpuLoad();
    virtual ~GpuLoad();

    void update();
    double getGpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;
private:
    uint64_t mIterationCount;

    
};

#endif
