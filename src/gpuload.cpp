#include "gpuload.h"

#include <ntstatus.h>

#include <algorithm>
#include <iostream>

#include <string>
#include <QString>

//DEFINE_GUID(GUID_DISPLAY_DEVICE_ARRIVAL, 0x1CA05180, 0xA699, 0x450A, 0x9A, 0x0C, 0xDE, 0x4F, 0xBE, 0x3D, 0xDD, 0x89);

GpuLoad::GpuLoad() : mIterationCount(0) {
    //
}

GpuLoad::~GpuLoad() {
    //
}

double GpuLoad::getGpuLoadOfCore(std::size_t core) const {
    return 0;
}

std::size_t GpuLoad::getCoreCount() const {
    return 0;
}

void GpuLoad::update() {

    ++mIterationCount;
}

