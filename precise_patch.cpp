#include "precise_timer.h"

std::wstring _timeinfo_wstring(TimeInfo ti) {
	std::wstring min = std::to_wstring(ti.minutes);
	if(min.size() < 2)
		min = std::wstring(L"0")+min;
	std::wstring sec = std::to_wstring(ti.seconds);
	if(sec.size() < 2)
		sec = std::wstring(L"0")+sec;
	std::wstring ms = std::to_wstring(int(ti.milliseconds));
	if(ms.size() < 3) {
		for(int i = ms.size(); i < 3; i++) {
			ms = std::wstring(L"0")+ms;
		}
	}
	return min + L":" + sec + L"." + ms;
}

DWORD __stdcall PreciseTimerPatch::precise_timer_thread_func(void* ptr) {
	PreciseTimerPatch* p = reinterpret_cast<PreciseTimerPatch*>(ptr);

	while(true) {
		p->cancel_mutex.lock();
		if(p->cancel) {
			p->cancel_mutex.unlock();
			return 0;
		}
		p->cancel_mutex.unlock();

		if(!p->chp->is_active()) {
			p->chp->perform();
		}

		int state = p->gh->game_state();
		if(state != STATE_MAINMENU && state != STATE_INTRO && state != STATE_CHARSELECT) {
			TimeInfo ti = p->gh->game_timer();
			p->chp->set_text(_timeinfo_wstring(p->gh->level_timer()) + L" / " + _timeinfo_wstring(p->gh->game_timer()));
		}
		else {
			p->chp->set_text(L"");
		}

		Sleep(16);
	}
	return 0;
}

bool PreciseTimerPatch::_perform() {
	if(!chp->is_active()) {
		chp->perform();
	}

	hThread = CreateThread(NULL, 0, precise_timer_thread_func, (void*)this, 0, &thread);
	return true;
}

bool PreciseTimerPatch::_undo() {
	cancel_mutex.lock();
	cancel = true;
	cancel_mutex.unlock();
	WaitForSingleObject(hThread, 0);
	
	if(chp->is_active()) {
		chp->undo();
	}
	return true;
}

PreciseTimerPatch::PreciseTimerPatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<CustomHudPatch> chp) : 
	Patch(dp->spel), 
	is_valid(true),
	cancel(false),
	chp(chp),
	gh(gh)
{
	is_valid = chp->valid() && gh->valid();
}

bool PreciseTimerPatch::valid() {
	return is_valid;
}