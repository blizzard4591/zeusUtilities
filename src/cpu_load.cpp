#include "cpu_load.h"

#include <Windows.h>
#include <winternl.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include <string>
#include <QString>

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

CpuLoad::CpuLoad() : 
    mIterationCount(0), mLastValues(nullptr), mCurrentValues(nullptr), mProcessorCount(0), mLastUserTime(0), 
    mLastKernelTime(0), mLastIdleTime(0), mProcessHistory(),
    mStateObject(), mProcessesArray(), mIsArmaRunning(false), mArmaImageName(), mArmaPid() {
    SYSTEM_INFO info = { 0 };
    GetSystemInfo(&info);
    mProcessorCount = info.dwNumberOfProcessors;

    mLastValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];
    mCurrentValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];

    for (DWORD i = 0; i < mProcessorCount; ++i) {
        mCpuLoadPerCore.push_back(0.0);
    }

    mProcessInformationSize = 1 * sizeof(SYSTEM_PROCESS_INFORMATION) + 10 * sizeof(SYSTEM_THREAD_INFORMATION);
    mProcessInformation = malloc(mProcessInformationSize);
}

CpuLoad::~CpuLoad() {
    if (mLastValues != nullptr) {
        delete[] mLastValues;
    }
    if (mCurrentValues != nullptr) {
        delete[] mCurrentValues;
    }
    if (mProcessInformation != nullptr) {
        delete mProcessInformation;
    }
}

void CpuLoad::reset() {
    if (mLastValues != nullptr) {
        delete[] mLastValues;
    }
    if (mCurrentValues != nullptr) {
        delete[] mCurrentValues;
    }

    mLastValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];
    mCurrentValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];

    mCpuLoadPerCore.clear();
    for (DWORD i = 0; i < mProcessorCount; ++i) {
        mCpuLoadPerCore.push_back(0.0);
    }
}

double CpuLoad::getCpuLoadOfCore(std::size_t core) const {
    return mCpuLoadPerCore.at(core);
}

std::size_t CpuLoad::getCoreCount() const {
    return mProcessorCount;
}

void CpuLoad::update(double minCpuUtil, double minMemUtil, double minGpuUtil, std::unordered_map<void*, GpuInfo> const& gpuLoad, bool doVerboseJson) {
    mStateObject = QJsonObject();
    mProcessesArray = QJsonArray();
    mIsArmaRunning = false;

    // Make room for new data, swapping the current to last
    std::swap(mCurrentValues, mLastValues);

    CpuInfo cpuInfo;

    uint64_t userTime = 0;
    uint64_t kernelTime = 0;
    uint64_t idleTime = 0;

    ULONG size;
    NtQuerySystemInformation(SystemProcessorPerformanceInformation, mCurrentValues, sizeof(mCurrentValues[0]) * mProcessorCount, &size);
    for (DWORD i = 0; i < mProcessorCount; ++i) {
        userTime += mCurrentValues[i].UserTime.QuadPart;
        kernelTime += mCurrentValues[i].KernelTime.QuadPart;
        idleTime += mCurrentValues[i].IdleTime.QuadPart;
        if (mIterationCount > 0) {
            double current_percent = (mCurrentValues[i].IdleTime.QuadPart - mLastValues[i].IdleTime.QuadPart) * 100.0;
            current_percent /= ((mCurrentValues[i].KernelTime.QuadPart + mCurrentValues[i].UserTime.QuadPart) - (mLastValues[i].KernelTime.QuadPart + mLastValues[i].UserTime.QuadPart));
            current_percent = 100.0 - current_percent;
            mCpuLoadPerCore.at(i) = current_percent;
        }
        cpuInfo.userTimeDelta = userTime - mLastUserTime;
        cpuInfo.kernelTimeDelta = kernelTime - mLastKernelTime;
        cpuInfo.idleTimeDelta = idleTime - mLastIdleTime;
    }

    mLastUserTime = userTime;
    mLastKernelTime = kernelTime;
    mLastIdleTime = idleTime;

    double overallTimeDelta = (cpuInfo.userTimeDelta + cpuInfo.kernelTimeDelta);
    if (overallTimeDelta == 0.0) {
        overallTimeDelta = 1.0;
    }

    // Memory information
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex) == 0) {
        std::cerr << "Failed to fetch memory information: " << GetLastError() << std::endl;
    }
    uint64_t const totalPhysicalMemory = statex.ullTotalPhys;

    // memory load, total mem, free mem, total page, free page, total virt, free virt
    cpuInfo.memoryLoad = statex.dwMemoryLoad;
    cpuInfo.memoryTotal = statex.ullTotalPhys;
    cpuInfo.memoryAvail = statex.ullAvailPhys;
    cpuInfo.pageTotal = statex.ullTotalPageFile;
    cpuInfo.pageAvail = statex.ullAvailPageFile;
    cpuInfo.virtualTotal = statex.ullTotalVirtual;
    cpuInfo.virtualAvail = statex.ullAvailVirtual;

    mStateObject = cpuInfo.toJsonObject(doVerboseJson);

    // Per-process information
    ULONG requiredSize = 0;
    NTSTATUS result;
    
    do {
        if (requiredSize > 0) {
            delete mProcessInformation;
            mProcessInformationSize = static_cast<std::size_t>(1.5 * requiredSize);
            mProcessInformation = malloc(mProcessInformationSize);
            std::cout << "Debug: resizing process info buffer to " << mProcessInformationSize << " Bytes." << std::endl;
        }
        result = NtQuerySystemInformation(SystemProcessInformation, mProcessInformation, static_cast<ULONG>(mProcessInformationSize), &requiredSize);
    } while (result == STATUS_INFO_LENGTH_MISMATCH);
    
    if (result != STATUS_SUCCESS) {
        std::cerr << "Return code was: " << result << " and required size was " << requiredSize << "!" << std::endl;
    } else {
        ULONG currentOffset = 0;
        uint8_t* basePointer = static_cast<uint8_t*>(mProcessInformation);

        std::unordered_set<void*> processesNotSeen;
        for (auto it = mProcessHistory.cbegin(); it != mProcessHistory.cend(); ++it) {
            processesNotSeen.insert(it->second.UniqueProcessId);
        }

        while (true) {
            PFULL_SYSTEM_PROCESS_INFORMATION pointer = reinterpret_cast<PFULL_SYSTEM_PROCESS_INFORMATION>(basePointer + currentOffset);
            void* handle = pointer->UniqueProcessId;
            processesNotSeen.erase(handle);

            if (!mProcessHistory.contains(handle)) {
                mProcessHistory.insert(std::make_pair(handle, ProcessInfo(pointer)));
            } else {
                mProcessHistory.at(handle).update(pointer);
            }

            if (gpuLoad.contains(handle)) {
                mProcessHistory.at(handle).updateGpuData(gpuLoad.at(handle));
            } else {
                mProcessHistory.at(handle).updateGpuData();
            }

            double const percentLoad = 100.0 * (mProcessHistory.at(handle).KernelTime.delta + mProcessHistory.at(handle).UserTime.delta) / overallTimeDelta;
            double const percentMemory = 100.0 * mProcessHistory.at(handle).WorkingSetSize.value / totalPhysicalMemory;

            bool const processUsesGpuMoreThanXPercent = gpuLoad.contains(handle) && (gpuLoad.at(handle).utilization >= minGpuUtil);
            bool const processUsesGpuMemoryMoreThan100MB = gpuLoad.contains(handle) && (gpuLoad.at(handle).dedicatedMemory >= (100uLL * 1024uLL * 1024uLL));

            bool const isArma32Bit = mProcessHistory.at(handle).ImageName.compare(QStringLiteral("arma3.exe"), Qt::CaseInsensitive) == 0;
            bool const isArma64Bit = mProcessHistory.at(handle).ImageName.compare(QStringLiteral("arma3_x64.exe"), Qt::CaseInsensitive) == 0;

            if (mProcessHistory.at(handle).ImageName.contains(QStringLiteral("arma3")) || (percentLoad >= minCpuUtil) || (percentMemory >= minMemUtil) || processUsesGpuMoreThanXPercent || processUsesGpuMemoryMoreThan100MB) {
                if ((mProcessHistory.at(handle).ImageName.compare(QStringLiteral("Memory Compression")) != 0) && (mProcessHistory.at(handle).ImageName.compare(QStringLiteral("dwm.exe")) != 0) && (((uint64_t)handle) != 0)) {
                    mProcessesArray.append(mProcessHistory.at(handle).toJsonObject(doVerboseJson));
                    std::cout << "Process: " << mProcessHistory.at(handle) << std::endl;
                }
            }

            if (isArma32Bit || isArma64Bit) {
                mIsArmaRunning = true;
                mArmaImageName = mProcessHistory.at(handle).ImageName;
                mArmaPid = (uint64_t)handle;
            }

            currentOffset += pointer->NextEntryOffset;
            if (pointer->NextEntryOffset == 0) {
                break;
            }
        }

        for (auto it = processesNotSeen.cbegin(); it != processesNotSeen.cend(); ++it) {
            mProcessHistory.erase(*it);
        }
    }

    ++mIterationCount;
}
