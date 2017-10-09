#pragma once

#include "patches.h"
#include "game_hooks.h"
#include "derandom.h"

#define CONTROLLER_ERROR -1
#define CONTROLLER_NONE 0
#define CONTROLLER_KEYBOARD 1
#define CONTROLLER_XBOX360 2

inline unsigned type_ctrl_offset();

void force_controller_type(std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp, int pid, int cont);
int controller_type(std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp, int pid);