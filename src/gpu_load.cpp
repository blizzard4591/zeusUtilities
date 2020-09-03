#include "gpu_load.h"

#include <algorithm>
#include <iostream>

#include <string>
#include <QString>
#include <QDateTime>

#include "gpu_query.h"


GpuLoad::GpuLoad() {
    //
}

GpuLoad::~GpuLoad() {
    stop();
}

void GpuLoad::update() {
    //
}

void GpuLoad::start() {
    mGpuQueryThread.start();
}

void GpuLoad::stop() {
    if (mGpuQueryThread.isRunning()) {
        mGpuQueryThread.requestInterruption();
        mGpuQueryThread.quit();
        mGpuQueryThread.wait();
    }
}

std::unordered_map<void*, GpuInfo> GpuLoad::getCurrentGpuLoad() const {
    return mGpuQueryThread.getCurrentGpuInfo();
}
