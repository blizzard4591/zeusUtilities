#include "gpuload.h"

#include <ntstatus.h>

#include <algorithm>
#include <iostream>

#include <string>
#include <QString>
#include <QDateTime>

#include "gpu_query.h"


GpuLoad::GpuLoad() : mIterationCount(0) {
    //
    if (mInstance != nullptr) {
        throw;
    }
    gpuCountersInit();
    mInstance = this;
}

GpuLoad::~GpuLoad() {
    gpuCountersClose();
}

double GpuLoad::getGpuLoadOfCore(std::size_t core) const {
    return 0;
}

std::size_t GpuLoad::getCoreCount() const {
    return 0;
}

void GpuLoad::update() {
    reset();

    if (mIterationCount == 0) {
        gpuCountersPrepareBefore();
    }

    queryGpuCounters();

    ++mIterationCount;
}

void GpuLoad::roundReset() {
    mGpuInformation.clear();
    mHadError = false;
}

void GpuLoad::reset() {
    roundReset();
    mIterationCount = 0;
}

void GpuLoad::updateGpuEngineUtil(uint64_t processId, uint64_t engineId, double value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformation.contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformation.insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformation.find((void*)processId)->second.utilization = value;
    }
}

void GpuLoad::updateGpuMemoryDedicated(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformation.contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformation.insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformation.find((void*)processId)->second.dedicatedMemory = value;
    }
}

void GpuLoad::updateGpuMemoryShared(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformation.contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformation.insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformation.find((void*)processId)->second.sharedMemory = value;
    }
}

void GpuLoad::updateGpuMemoryCommit(uint64_t processId, uint64_t value) {
    if (mInstance != nullptr) {
        if (!mInstance->mGpuInformation.contains((void*)processId)) {
            GpuInfo gpuInfo;
            mInstance->mGpuInformation.insert(std::make_pair((void*)processId, gpuInfo));
        }
        mInstance->mGpuInformation.find((void*)processId)->second.commitMemory = value;
    }
}

void GpuLoad::setHadError() {
    if (mInstance != nullptr) {
        mInstance->mHadError = true;
    }
}

static QDateTime mMeasure;

void MeasureShowTime(int id) {
    std::cout << QDateTime::currentDateTime().toString("dd.MM.yyy hh:mm:ss.zzz").toStdString() << " at ID #" << id << std::endl;
}

void MeasureBefore() {
    mMeasure = QDateTime::currentDateTime();
}

void MeasureAfter(int id) {
    qint64 const msecsUsed = mMeasure.msecsTo(QDateTime::currentDateTime());
    std::cout << "Used " << msecsUsed << "ms for Operation #" << id << "." << std::endl;
}
