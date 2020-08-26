#ifndef CPULOAD_H
#define CPULOAD_H

#include <cstdint>
#include <vector>

#include <Windows.h>
#include <winternl.h>

class CpuLoad {
public:
    CpuLoad();
    virtual ~CpuLoad();

    void update();
    double getCpuLoadOfCore(std::size_t core) const;
    std::size_t getCoreCount() const;
private:
    uint64_t mIterationCount;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION mLastValues;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION mCurrentValues;

    DWORD mProcessorCount;

    std::vector<double> mCpuLoadPerCore;

    void* mProcessInformation;
    std::size_t mProcessInformationSize;
};

#endif
