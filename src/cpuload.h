#ifndef CPULOAD_H
#define CPULOAD_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <Windows.h>
#include <winternl.h>

typedef struct _FULL_SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize; // since VISTA
    ULONG HardFaultCount; // since WIN7
    ULONG NumberOfThreadsHighWatermark; // since WIN7
    ULONGLONG CycleTime; // since WIN7
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR UniqueProcessKey; // since VISTA (requires SystemExtendedProcessInformation)
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    SYSTEM_THREAD_INFORMATION Threads[1]; // SystemProcessInformation
    // SYSTEM_EXTENDED_THREAD_INFORMATION Threads[1]; // SystemExtendedProcessinformation
    // SYSTEM_EXTENDED_THREAD_INFORMATION + SYSTEM_PROCESS_INFORMATION_EXTENSION // SystemFullProcessInformation
} FULL_SYSTEM_PROCESS_INFORMATION, * PFULL_SYSTEM_PROCESS_INFORMATION;

#include <QString>
#include <QStringList>

#include "gpuload.h"

class DeltaValueLI {
public:
    DeltaValueLI(LARGE_INTEGER v) {
        value = v;
        delta = 0;
    }

    void operator =(LARGE_INTEGER const& b) {
        delta = static_cast<int64_t>(b.QuadPart) - static_cast<int64_t>(value.QuadPart);
        value.QuadPart = b.QuadPart;
    }

    LARGE_INTEGER value;
    int64_t delta;

    QString toQString();
    friend std::ostream& operator<<(std::ostream& os, const DeltaValueLI& d);
};

class DeltaValueST {
public:
    DeltaValueST(std::size_t v) {
        value = v;
        delta = 0;
    }

    void operator =(std::size_t const& b) {
        delta = static_cast<int64_t>(b) - static_cast<int64_t>(value);
        value = b;
    }

    std::size_t value;
    int64_t delta;

    QString toQString();
    friend std::ostream& operator<<(std::ostream& os, const DeltaValueST& d);
};

class ProcessInfo {
public:
    ProcessInfo(PFULL_SYSTEM_PROCESS_INFORMATION processInformation);

    void update(PFULL_SYSTEM_PROCESS_INFORMATION processInformation);

    QString const ImageName;
    HANDLE const UniqueProcessId;

    DeltaValueLI UserTime;
    DeltaValueLI KernelTime;
    
    DeltaValueST PeakVirtualSize;
    DeltaValueST VirtualSize;
    DeltaValueST PeakWorkingSetSize;
    DeltaValueST WorkingSetSize;
    DeltaValueST PagefileUsage;
    DeltaValueST PeakPagefileUsage;
    DeltaValueST PrivatePageCount;
    
    QString toQString();
    friend std::ostream& operator<<(std::ostream& os, const ProcessInfo& pi);
};

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
