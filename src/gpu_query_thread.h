#ifndef GPU_QUERY_THREAD_H
#define GPU_QUERY_THREAD_H

#include <QThread>
#include <QMutex>

#include <unordered_map>

#include <Windows.h>

#include "gpu_info.h"

extern "C" {
    VOID ParseGpuEngineUtilizationCounter(
        _In_ PWSTR InstanceName,
        _In_ DOUBLE InstanceValue
    );

    VOID ParseGpuProcessMemoryDedicatedUsageCounter(
        _In_ PWSTR InstanceName,
        _In_ ULONG64 InstanceValue
    );

    VOID ParseGpuProcessMemorySharedUsageCounter(
        _In_ PWSTR InstanceName,
        _In_ ULONG64 InstanceValue
    );

    VOID ParseGpuProcessMemoryCommitUsageCounter(
        _In_ PWSTR InstanceName,
        _In_ ULONG64 InstanceValue
    );
}

class GpuQueryThread : public QThread {
	Q_OBJECT
public:
	GpuQueryThread();
	virtual ~GpuQueryThread();

    virtual void run() override;

    static void updateGpuEngineUtil(uint64_t processId, uint64_t engineId, double value);
    static void updateGpuMemoryDedicated(uint64_t processId, uint64_t value);
    static void updateGpuMemoryShared(uint64_t processId, uint64_t value);
    static void updateGpuMemoryCommit(uint64_t processId, uint64_t value);
    static void setHadError();
private:
	QMutex mMutex;

    static GpuQueryThread* mInstance;

    std::unordered_map<void*, GpuInfo> mGpuInformationA;
    std::unordered_map<void*, GpuInfo> mGpuInformationB;

    std::unordered_map<void*, GpuInfo>* mGpuInformationCurrent;
    std::unordered_map<void*, GpuInfo>* mGpuInformationNext;

    bool mHadError;
    uint64_t mIterationCount;
};

#endif
