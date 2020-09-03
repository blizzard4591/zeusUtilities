#ifndef ETW_OUTPUT_THREAD_H
#define ETW_OUTPUT_THREAD_H

#include <QThread>

#include <windows.h>
#include <evntcons.h> // must include after windows.h

#include "fps_info.h"

#include "etw_trace_consumer.h"

#include <vector>
#include <unordered_map>

struct SwapChainData {
	enum { PRESENT_HISTORY_MAX_COUNT = 120 };
	std::shared_ptr<PresentEvent> mPresentHistory[PRESENT_HISTORY_MAX_COUNT];
	uint32_t mPresentHistoryCount;
	uint32_t mNextPresentIndex;
	uint32_t mLastDisplayedPresentIndex;
};

struct EtwProcessInfo {
	std::string mModuleName;
	std::unordered_map<uint64_t, SwapChainData> mSwapChain;
	HANDLE mHandle;
	bool mTargetProcess;
};

class EtwOutputThread : public QThread {
	Q_OBJECT
public:
	EtwOutputThread();
	virtual ~EtwOutputThread();

	FpsInfo const& getFpsInfo() const { return mFpsInfo; }

	virtual void run() override;
	void setTargetPid(quint64 pid);

	void setOutputRecordingState(bool record);

	static quint64 targetPid;
signals:
	void updatedFps(quint64 pid, FpsInfo const& fpsInfo);
private:
	//
	FpsInfo mFpsInfo;

	void ProcessEvents(
		std::vector<ProcessEvent>* processEvents,
		std::vector<std::shared_ptr<PresentEvent>>* presentEvents,
		std::vector<uint64_t>* recordingToggleHistory,
		std::vector<std::pair<uint32_t, uint64_t>>* terminatedProcesses);
	void UpdateConsole(uint32_t processId, EtwProcessInfo const& processInfo);
};

#endif
