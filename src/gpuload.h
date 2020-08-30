#ifndef GPULOAD_H
#define GPULOAD_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <QDateTime>
#include <QString>
#include <QThread>

#include <Windows.h>


class GpuLoad {
public:
    GpuLoad();
    virtual ~GpuLoad();

    void update();
    void reset();

    double getGpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;
    std::unordered_map<void*, GpuInfo> const& getCurrentGpuLoad() const { return mGpuInformation; }

    static void updateGpuEngineUtil(uint64_t processId, uint64_t engineId, double value);
    static void updateGpuMemoryDedicated(uint64_t processId, uint64_t value);
    static void updateGpuMemoryShared(uint64_t processId, uint64_t value);
    static void updateGpuMemoryCommit(uint64_t processId, uint64_t value);
    static void setHadError();
private:
    static GpuLoad* mInstance;

    void roundReset();

    std::unordered_map<void*, GpuInfo> mGpuInformation;
    bool mHadError;
    uint64_t mIterationCount;
};

#endif
