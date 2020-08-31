#ifndef CPULOAD_H
#define CPULOAD_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <QString>
#include <QStringList>

#include "process_info.h"
#include "gpu_load.h"

class CpuLoad {
public:
    CpuLoad();
    virtual ~CpuLoad();

    void update(std::unordered_map<void*, GpuInfo> const& gpuLoad);
    void reset();

    double getCpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;

    QString const& getStateString() const { return mStateString; }
    QStringList const& getProcessesStrings() const { return mProcessesStrings; }

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

    uint64_t mUserTimeDelta;
    uint64_t mKernelTimeDelta;
    uint64_t mIdleTimeDelta;

    std::vector<double> mCpuLoadPerCore;
    std::unordered_map<void*, ProcessInfo> mProcessHistory;

    void* mProcessInformation;
    std::size_t mProcessInformationSize;

    QString mStateString;
    QStringList mProcessesStrings;
    
    bool mIsArmaRunning;
    QString mArmaImageName;
    uint64_t mArmaPid;
};

#endif
