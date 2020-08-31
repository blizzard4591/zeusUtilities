#include "cpuload.h"

#include <winternl.h>
#include <ntstatus.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include <string>
#include <QString>

CpuLoad::CpuLoad() : mIterationCount(0), mLastValues(nullptr), mCurrentValues(nullptr), mProcessorCount(0), mProcessHistory(), mStateString(), mProcessesStrings(), mIsArmaRunning(false) {
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

    mUserTimeDelta = 0;
    mLastKernelTime = 0;
    mLastIdleTime = 0;
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

    mUserTimeDelta = 0;
    mLastKernelTime = 0;
    mLastIdleTime = 0;
}

double CpuLoad::getCpuLoadOfCore(std::size_t core) const {
    return mCpuLoadPerCore.at(core);
}

std::size_t CpuLoad::getCoreCount() const {
    return mProcessorCount;
}

void CpuLoad::update(std::unordered_map<void*, GpuInfo> const& gpuLoad) {
    mStateString = "";
    mProcessesStrings.clear();
    mIsArmaRunning = false;

    // Make room for new data, swapping the current to last
    std::swap(mCurrentValues, mLastValues);

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
        mUserTimeDelta = userTime - mLastUserTime;
        mKernelTimeDelta = kernelTime - mLastKernelTime;
        mIdleTimeDelta = idleTime - mLastIdleTime;
    }

    mLastUserTime = userTime;
    mLastKernelTime = kernelTime;
    mLastIdleTime = idleTime;

    double overallTimeDelta = (mUserTimeDelta + mKernelTimeDelta);
    if (overallTimeDelta == 0.0) {
        overallTimeDelta = 1.0;
    }

    // userDelta, kernelDelta, idleDelta
    mStateString += QStringLiteral("%1;%2;%3").arg(mUserTimeDelta).arg(mKernelTimeDelta).arg(mIdleTimeDelta);

    // Memory information
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex) == 0) {
        std::cerr << "Failed to fetch memory information: " << GetLastError() << std::endl;
    }
    uint64_t const totalPhysicalMemory = statex.ullTotalPhys;

    // memory load, total mem, free mem, total page, free page, total virt, free virt
    mStateString += QStringLiteral(";%1;%2;%3;%4;%5;%6;%7").arg(statex.dwMemoryLoad).arg(statex.ullTotalPhys).arg(statex.ullAvailPhys).arg(statex.ullTotalPageFile).arg(statex.ullAvailPageFile).arg(statex.ullTotalVirtual).arg(statex.ullAvailVirtual);

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

            bool const processUsesGpuMoreThan5Percent = gpuLoad.contains(handle) && (gpuLoad.at(handle).utilization >= 0.05);
            bool const processUsesGpuMemoryMoreThan100MB = gpuLoad.contains(handle) && (gpuLoad.at(handle).dedicatedMemory >= (100uLL * 1024uLL * 1024uLL));

            // For debugging
            /*
            uint64_t intProcessHandle = (uint64_t)handle;
            if (intProcessHandle == 16780 || intProcessHandle == 31056) {
                std::cout << mProcessHistory.at(handle) << " (" << percentLoad << "%)" << std::endl;
            }
            */

            bool const isArma32Bit = mProcessHistory.at(handle).ImageName.compare(QStringLiteral("arma3.exe"), Qt::CaseInsensitive) == 0;
            bool const isArma64Bit = mProcessHistory.at(handle).ImageName.compare(QStringLiteral("arma3_x64.exe"), Qt::CaseInsensitive) == 0;

            if (mProcessHistory.at(handle).ImageName.contains(QStringLiteral("arma3")) || (percentLoad >= 10.0) || (percentMemory >= 10.0) || processUsesGpuMoreThan5Percent || processUsesGpuMemoryMoreThan100MB) {
                if ((mProcessHistory.at(handle).ImageName.compare(QStringLiteral("Memory Compression")) != 0) && (mProcessHistory.at(handle).ImageName.compare(QStringLiteral("dwm.exe")) != 0)) {
                    mProcessesStrings.append(mProcessHistory.at(handle).toQString());
                }
            }

            if (isArma32Bit || isArma64Bit) {
                mIsArmaRunning = true;
                mArmaImageName = mProcessHistory.at(handle).ImageName;
                mArmaPid = (uint64_t)handle;
            }

            if (((uint64_t)handle == 2856) && (gpuLoad.contains(handle))) {
                std::cout << "GPU ff: " << gpuLoad.at(handle) << std::endl;
            }

            //std::string imageName = ws2s(wImageName);
            //std::cout << "#Threads: " << pointer->NumberOfThreads << ",Image:" << imageName.toStdString() << ",VirtSize:" << pointer->VirtualSize << std::endl;

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
