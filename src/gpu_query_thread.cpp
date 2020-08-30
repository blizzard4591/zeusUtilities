#include "gpu_query_thread.h"

#include "gpu_query.h"

#include <algorithm>

#define GPU_DEBUG
#undef GPU_DEBUG

VOID ParseGpuEngineUtilizationCounter(
    _In_ PWSTR InstanceName,
    _In_ DOUBLE InstanceValue
) {
    std::wstring const wInstanceName(InstanceName, wcslen(InstanceName));
    QString const qInstanceName = QString::fromStdWString(wInstanceName);

    // pid_12704_luid_0x00000000_0x0000D503_phys_0_eng_3_engtype_VideoDecode
    QVector<QStringRef> const splits = qInstanceName.splitRef('_');
    QStringRef const& pidPartSr = splits.at(1);
    QStringRef const& luidLowPartSr = splits.at(3);
    QStringRef const& luidHighPartSr = splits.at(4);
    QStringRef const& physPartSr = splits.at(6);
    QStringRef const& engPartSr = splits.at(8);
    QStringRef const& engTypePartSr = splits.at(10);

    if (pidPartSr.size() > 0) {
        bool okA = false;
        bool okB = false;
        ULONG64 processId = pidPartSr.toULongLong(&okA);
        ULONG64 engineId = engPartSr.toULongLong(&okB);
        if (okA && okB) {
#ifdef GPU_DEBUG
            std::cout << "1 - " << qInstanceName.toStdString() << " (" << engineId << "): " << InstanceValue << std::endl;
#endif

            GpuQueryThread::updateGpuEngineUtil(processId, engineId, InstanceValue);
        } else {
            GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
            std::cerr << "Error 2 in Gpu-1." << std::endl;
#endif
        }
    } else {
        GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
        std::cerr << "Error in Gpu-1." << std::endl;
#endif
    }
}

VOID ParseGpuProcessMemoryDedicatedUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
) {
    std::wstring const wInstanceName(InstanceName, wcslen(InstanceName));
    QString const qInstanceName = QString::fromStdWString(wInstanceName);

    // pid_1116_luid_0x00000000_0x0000D3EC_phys_0
    QVector<QStringRef> const splits = qInstanceName.splitRef('_');
    QStringRef const& pidPartSr = splits.at(1);
    QStringRef const& luidLowPartSr = splits.at(3);
    QStringRef const& luidHighPartSr = splits.at(4);
    QStringRef const& physPartSr = splits.at(6);

    if (pidPartSr.size() > 0) {
        bool okA = false;
        ULONG64 processId = pidPartSr.toULongLong(&okA);
        if (okA) {
#ifdef GPU_DEBUG
            std::cout << "2 - " << qInstanceName.toStdString() << ": " << InstanceValue << std::endl;
#endif

            GpuQueryThread::updateGpuMemoryDedicated(processId, InstanceValue);
        } else {
            GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
            std::cerr << "Error 2 in Gpu-2." << std::endl;
#endif
        }
    } else {
        GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
        std::cerr << "Error in Gpu-2." << std::endl;
#endif
    }
}

VOID ParseGpuProcessMemorySharedUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
) {
    std::wstring const wInstanceName(InstanceName, wcslen(InstanceName));
    QString const qInstanceName = QString::fromStdWString(wInstanceName);

    // pid_1116_luid_0x00000000_0x0000D3EC_phys_0
    QVector<QStringRef> const splits = qInstanceName.splitRef('_');
    QStringRef const& pidPartSr = splits.at(1);
    QStringRef const& luidLowPartSr = splits.at(3);
    QStringRef const& luidHighPartSr = splits.at(4);
    QStringRef const& physPartSr = splits.at(6);

    if (pidPartSr.size() > 0) {
        bool okA = false;
        ULONG64 processId = pidPartSr.toULongLong(&okA);
        if (okA) {
#ifdef GPU_DEBUG
            std::cout << "3 - " << qInstanceName.toStdString() << ": " << InstanceValue << std::endl;
#endif

            GpuQueryThread::updateGpuMemoryShared(processId, InstanceValue);
        } else {
            GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
            std::cerr << "Error 2 in Gpu-3." << std::endl;
#endif
        }
    } else {
        GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
        std::cerr << "Error in Gpu-3." << std::endl;
#endif
    }
}

VOID ParseGpuProcessMemoryCommitUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
) {
    std::wstring const wInstanceName(InstanceName, wcslen(InstanceName));
    QString const qInstanceName = QString::fromStdWString(wInstanceName);

    // pid_1116_luid_0x00000000_0x0000D3EC_phys_0
    QVector<QStringRef> const splits = qInstanceName.splitRef('_');
    QStringRef const& pidPartSr = splits.at(1);
    QStringRef const& luidLowPartSr = splits.at(3);
    QStringRef const& luidHighPartSr = splits.at(4);
    QStringRef const& physPartSr = splits.at(6);

    if (pidPartSr.size() > 0) {
        bool okA = false;
        ULONG64 processId = pidPartSr.toULongLong(&okA);
        if (okA) {
#ifdef GPU_DEBUG
            std::cout << "4 - " << qInstanceName.toStdString() << ": " << InstanceValue << std::endl;
#endif

            GpuQueryThread::updateGpuMemoryCommit(processId, InstanceValue);
        } else {
            GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
            std::cerr << "Error 2 in Gpu-4." << std::endl;
#endif
        }
    } else {
        GpuQueryThread::setHadError();
#ifdef GPU_DEBUG
        std::cerr << "Error in Gpu-4." << std::endl;
#endif
    }
}

VOID AquireGpuLock() {
    GpuQueryThread::aquireLock();
}

VOID ReleaseGpuLock() {
    GpuQueryThread::releaseLock();
}

VOID GpuUpdateRoundComplete() {
    GpuQueryThread::roundComplete();
}

GpuQueryThread* GpuQueryThread::mInstance = nullptr;

GpuQueryThread::GpuQueryThread() : mGpuInformationA(), mGpuInformationB(), mGpuInformationCurrent(&mGpuInformationA), mGpuInformationNext(&mGpuInformationB), mHadError(false), mRoundCounter(0) {
    if (mInstance != nullptr) {
        throw;
    }

    mInstance = this;
}

GpuQueryThread::~GpuQueryThread() {
    //
}

std::unordered_map<void*, GpuInfo> GpuQueryThread::getCurrentGpuInfo() const {
    QMutexLocker lock(&mMutex);

    return *mGpuInformationCurrent;
}

void GpuQueryThread::roundComplete() {
    if (mInstance != nullptr) {
        QMutexLocker lock(&mInstance->mMutex);

        std::swap(mInstance->mGpuInformationCurrent, mInstance->mGpuInformationNext);
        mInstance->mHadError = false;
        mInstance->mGpuInformationNext->clear();
        ++mInstance->mRoundCounter;
    }
}

void GpuQueryThread::aquireLock() {
    if (mInstance != nullptr) {
        mInstance->mMutex.lock();
    }
}

void GpuQueryThread::releaseLock() {
    if (mInstance != nullptr) {
        mInstance->mMutex.unlock();
    }
}

void GpuQueryThread::updateGpuEngineUtil(uint64_t processId, uint64_t engineId, double value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformationNext->contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformationNext->insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformationNext->find((void*)processId)->second.utilization = value;
    }
}

void GpuQueryThread::updateGpuMemoryDedicated(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformationNext->contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformationNext->insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformationNext->find((void*)processId)->second.dedicatedMemory = value;
    }
}

void GpuQueryThread::updateGpuMemoryShared(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformationNext->contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformationNext->insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformationNext->find((void*)processId)->second.sharedMemory = value;
    }
}

void GpuQueryThread::updateGpuMemoryCommit(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformationNext->contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformationNext->insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformationNext->find((void*)processId)->second.commitMemory = value;
    }
}

void GpuQueryThread::setHadError() {
    if (mInstance != nullptr) {
        mInstance->mHadError = true;
    }
}

void GpuQueryThread::run() {
    mRoundCounter = 0;
    mGpuInformationA.clear();
    mGpuInformationB.clear();

    while (true) {
        runGpuQueries();
    }
}
