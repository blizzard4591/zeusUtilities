#include "gpuload.h"

#include <ntstatus.h>

#include <algorithm>
#include <iostream>

#include <string>
#include <QString>
#include <QDateTime>

#include "main_ph.h"

/*
QDateTime fromFileTime(PFILETIME fileTime) {
    // Definition of FILETIME from MSDN:
    // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    QDateTime origin(QDate(1601, 1, 1), QTime(0, 0, 0, 0), Qt::UTC);

    LARGE_INTEGER time;
    time.HighPart = fileTime->dwHighDateTime;
    time.LowPart = fileTime->dwLowDateTime;

    qint64 const timeInMSecs = time.QuadPart / 10000;

    return origin.addMSecs(timeInMSecs);
}
*/

GpuLoad::GpuLoad() : mIterationCount(0) {
    //
    //HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
    //main_ph(hInstance);
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

