#pragma once

#include "patches.h"
#include "game_hooks.h"
#include "derandom.h"
#include "custom_hud.h"

#include <thread>
#include <mutex>

class PreciseTimerPatch : public Patch {
	bool is_valid;
	
	bool cancel;
	std::mutex cancel_mutex;
	std::shared_ptr<CustomHudPatch> chp;
	std::shared_ptr<GameHooks> gh;
	DWORD thread;
	HANDLE hThread;

private:
	virtual bool _perform() override;
	virtual bool _undo() override;
	
	static DWORD __stdcall precise_timer_thread_func(void* ptr);

public:
	PreciseTimerPatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<CustomHudPatch> chp);

	virtual bool valid() override;
};