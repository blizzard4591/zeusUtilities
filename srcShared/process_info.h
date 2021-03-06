#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include <Windows.h>
#include <winternl.h>

#include <cstdint>
#include <iostream>

#include <QJsonObject>
#include <QString>

#include "gpu_info.h"

class DeltaValueLI {
public:
    DeltaValueLI(LARGE_INTEGER v) {
        value = v;
        delta = 0;
    }

    DeltaValueLI() : delta(0) { value.QuadPart = 0; }

    void operator =(LARGE_INTEGER const& b) {
        delta = static_cast<int64_t>(b.QuadPart) - static_cast<int64_t>(value.QuadPart);
        value.QuadPart = b.QuadPart;
    }

    LARGE_INTEGER value;
    int64_t delta;

    QJsonObject toJsonObject(bool beVerbose) const;
    static DeltaValueLI fromJsonObject(QJsonObject const& object, bool* okay);

    QString toQString() const;
    friend std::ostream& operator<<(std::ostream& os, const DeltaValueLI& d);
private:
    static const QString TAG_V;
    static const QString TAG_D;
};

class DeltaValueST {
public:
    DeltaValueST(std::size_t v) {
        value = v;
        delta = 0;
    }

    DeltaValueST() : value(0), delta(0) {}

    void operator =(std::size_t const& b) {
        delta = static_cast<int64_t>(b) - static_cast<int64_t>(value);
        value = b;
    }

    std::size_t value;
    int64_t delta;

    QJsonObject toJsonObject(bool beVerbose) const;
    static DeltaValueST fromJsonObject(QJsonObject const& object, bool* okay);

    QString toQString() const;
    friend std::ostream& operator<<(std::ostream& os, const DeltaValueST& d);
private:
    static const QString TAG_V;
    static const QString TAG_D;
};

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

class ProcessInfo {
public:
    ProcessInfo(PFULL_SYSTEM_PROCESS_INFORMATION processInformation);

    void update(PFULL_SYSTEM_PROCESS_INFORMATION processInformation);
    void updateGpuData();
    void updateGpuData(GpuInfo const& gpuInfo);

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

    GpuInfo GpuData;

    QString toQString();
    
    QJsonObject toJsonObject(bool beVerbose) const;
    static ProcessInfo fromJsonObject(QJsonObject const& object, bool* okay);

    friend std::ostream& operator<<(std::ostream& os, const ProcessInfo& pi);
private:
    ProcessInfo(QString const& imageName, HANDLE uniqueProcessId) : ImageName(imageName), UniqueProcessId(uniqueProcessId) {}

    static const QString TAG_IMAGENAME_L; static const QString TAG_IMAGENAME_S;
    static const QString TAG_PID_L; static const QString TAG_PID_S;
    
    static const QString TAG_USERTIME_L; static const QString TAG_USERTIME_S;
    static const QString TAG_KERNELTIME_L; static const QString TAG_KERNELTIME_S;

    static const QString TAG_PEAKVIRTSIZE_L; static const QString TAG_PEAKVIRTSIZE_S;
    static const QString TAG_VIRTSIZE_L; static const QString TAG_VIRTSIZE_S;
    static const QString TAG_PEAKWORKINGSIZE_L; static const QString TAG_PEAKWORKINGSIZE_S;
    static const QString TAG_WORKINGSIZE_L; static const QString TAG_WORKINGSIZE_S;
    static const QString TAG_PAGEFILEUSAGE_L; static const QString TAG_PAGEFILEUSAGE_S;
    static const QString TAG_PEAKPAGEFILEUSAGE_L; static const QString TAG_PEAKPAGEFILEUSAGE_S;
    static const QString TAG_PRIVATEPAGECOUNT_L; static const QString TAG_PRIVATEPAGECOUNT_S;

    static const QString TAG_GPUDATA_L; static const QString TAG_GPUDATA_S;
};

#endif

