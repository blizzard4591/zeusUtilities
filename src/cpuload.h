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

class ProcessInfo {
public:
    ProcessInfo(HANDLE UniqueProcessId, QString const& ImageName)

    ProcessInfo(PFULL_SYSTEM_PROCESS_INFORMATION processInformation);

    ULONG const NumberOfThreads;
    LARGE_INTEGER const UserTime;
    LARGE_INTEGER const KernelTime;
    QString const ImageName;
    HANDLE const UniqueProcessId;
    ULONG const HandleCount;
    ULONG const SessionId;
    SIZE_T const PeakVirtualSize;
    SIZE_T const VirtualSize;
    SIZE_T const PeakWorkingSetSize;
    SIZE_T const WorkingSetSize;
    SIZE_T const PagefileUsage;
    SIZE_T const PeakPagefileUsage;
    SIZE_T const PrivatePageCount;
};

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
    std::vector<std::unordered_map<void*, ProcessInfo>> mProcessHistory;
    std::size_t mProcessHistorySize;
    std::size_t mProcessHistoryHead;
    std::size_t mProcessHistoryTail;

    void* mProcessInformation;
    std::size_t mProcessInformationSize;
};

#endif
