#pragma once

#include "patches.h"
#include "spelunky.h"

class OwnCameraPatch : public Patch {
private:
    bool is_valid;
    Address kill_player_address;
    Address ghost_spawn_on_start_address;
    Address ghost_spawn_on_death_address;
    Address game_instance_usage_address;
    Address game_instance_address;
    Address lock_camera_on_flag_owner_1_address;
    Address lock_camera_on_flag_owner_2_address;
    Address lock_camera_on_flag_owner_1_data;
    Address lock_camera_on_flag_owner_2_data;

public:
    OwnCameraPatch(int pid, std::shared_ptr<Spelunky> spel);
    ~OwnCameraPatch();

    virtual bool valid() override;

private:
    virtual bool _perform() override;
    virtual bool _undo() override;
};
