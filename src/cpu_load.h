#ifndef CPULOAD_H
#define CPULOAD_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include "cpu_info.h"
#include "process_info.h"
#include "gpu_load.h"

class CpuLoad {
public:
    CpuLoad();
    virtual ~CpuLoad();

    void update(double minCpuUtil, double minMemUtil, double minGpuUtil, std::unordered_map<void*, GpuInfo> const& gpuLoad, bool doVerboseJson);
    void reset();

    double getCpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;

    QJsonObject const& getStateAsJsonObject() const { return mStateObject; }
    QJsonArray const& getProcessesAsJsonArray() const { return mProcessesArray; }

    bool isArmaRunning() const { return mIsArmaRunning; }
    QString const& getArmaImageName() const { return mArmaImageName; }
    uint64_t getArmaPid() const { return mArmaPid; }
private:
    uint64_t mIterationCount;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION mLastValues;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION mCurrentValues;

    DWORD mProcessorCount;
    uint64_t mLastUserTime;
    uint64_t mLastKernelTime;
    uint64_t mLastIdleTime;

    std::vector<double> mCpuLoadPerCore;
    std::unordered_map<void*, ProcessInfo> mProcessHistory;

    void* mProcessInformation;
    std::size_t mProcessInformationSize;

    QJsonObject mStateObject;
    QJsonArray mProcessesArray;
    
    bool mIsArmaRunning;
    QString mArmaImageName;
    uint64_t mArmaPid;
};

#endif
