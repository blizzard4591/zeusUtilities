#include "phthread.h"

extern "C" {

#include <Windows.h>

extern INT main_ph(HINSTANCE Instance);
}

PhThread::~PhThread() {
	//
}

void PhThread::run() {
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	int const result = main_ph(hInstance);

	emit phTerminated(result);
}
