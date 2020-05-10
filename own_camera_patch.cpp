#include "own_camera_patch.h"
#include "debug.h"

#include <cassert>

#define EXEC_SPACE_ALLOC 512

// The extra instructions are to find the exact piece of code, because only 2 instructions give the wrong result.
BYTE kill_player[] = {
    0x2B, 0xC1,                               // SUB  EAX,ECX
    0x89, 0x44, 0x24, 0x18,                   // MOV  dword ptr [ESP - 60h],EAX
    0x0F, 0x88, 0xBE, 0x01, 0x00, 0x00,       // JS   doesn't matter
    0x83, 0xBB, 0x40, 0x01, 0x00, 0x00, 0x00, // CMP  dword ptr [EBX + 140h],0h
};

std::string kill_player_mask = "xxxxxxxxxxxxxxxxxxx";

BYTE kill_player_patched[] = {
    0x90, 0x90,                               // NOP NOP
    0x89, 0x44, 0x24, 0x18,                   // MOV   dword ptr [ESP - 60h],EAX
    0x0F, 0x88, 0xBE, 0x01, 0x00, 0x00,       // JS    doesn't matter
    0x83, 0xBB, 0x40, 0x01, 0x00, 0x00, 0x00, // CMP   dword ptr [EBX + 140h],0h
};

BYTE ghost_spawn_on_start[] = {
    0x85, 0xC0,                         // TEST  EAX,EAX
    0x74, 0x09,                         // JZ    to the ghost spawning code
    0x83, 0xF8, 0x0B,                   // CMP   EAX,Bh
    0x0F, 0x85, 0xA8, 0x00, 0x00, 0x00, // JNZ   over the ghost spawning code
};

std::string ghost_spawn_on_start_mask = "xxxxxxxxxxxxx";

BYTE ghost_spawn_on_start_patched[] = {
    0x90, 0x90, 0x90, 0x90,       // NOP NOP NOP NOP
    0x90, 0x90, 0x90, 0x90,       // NOP NOP NOP NOP
    0xE9, 0xA8, 0x00, 0x00, 0x00, // JMP   over the ghost spawning code
};

BYTE ghost_spawn_on_death[] = {
    0x83, 0x79, 0x58, 0x17,                   // CMP  dword ptr [ECX + 58h],17h
    0x75, 0x0C,                               // JNZ  to the ghost spawning code
    0x8B, 0x41, 0x4C,                         // MOV  EAX,dword ptr [ECX + 4Ch]
    0x80, 0xB8, 0x81, 0x2D, 0x12, 0x00, 0x00, // CMP  dword ptr [EAX + 122D81h],0h
    0x74, 0x70,                               // JZ   over the ghost spawning code
};

std::string ghost_spawn_on_death_mask = "xxxxxxxxxxxxxxxxxx";

BYTE ghost_spawn_on_death_patched[] = {
    0x90, 0x90, 0x90, 0x90, // NOP NOP NOP NOP
    0x90, 0x90, 0x90, 0x90, // NOP NOP NOP NOP
    0x90, 0x90, 0x90, 0x90, // NOP NOP NOP NOP
    0x90, 0x90, 0x90, 0x90, // NOP NOP NOP NOP
    0xEB, 0x70,             // JMP over the ghost spawning code
};

BYTE game_instance_usage[] = {
    0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00, // MOV   EBP, dword ptr[GAME_INSTANCE_ADDRESS]
    0xDD, 0xD8,                         // FSTP  ST0
    0x8B, 0x85, 0x84, 0x06, 0x44, 0x00, // MOV   EAX, dword ptr[EBP + 440684h] = > LAB_00640684
    0xD9, 0x05, 0x00, 0x00, 0x00, 0x00, // FLD   dword ptr[HUGE_FLOAT_CONSTANT]
    0x83, 0xCF, 0xFF,                   // OR    EDI, ffffffffh
};

std::string game_instance_usage_mask = "xx....xxxxxxxxxx....xxx";

size_t game_instance_usage_offset = 2;

BYTE lock_camera_on_flag_owner_1[] = {
    0x8B, 0x42, 0x30,                         //    MOV   EAX,dword ptr [EDX + 30h]
    0x3B, 0xC1,                               //    CMP   EAX,ECX
    0x74, 0x16,                               //    JZ    after_lock_on_flag_owner
    0x3B, 0xC3,                               //    CMP   EAX,EBX
    0x74, 0x0F,                               //    JZ    before_lock_on_flag_owner
    0x83, 0x78, 0x08, 0x03,                   //    CMP   dword ptr [EAX + 8h],3h
    0x74, 0x0C,                               //    JZ    after_lock_on_flag_owner
    0x81, 0x78, 0x0C, 0x1F, 0x04, 0x00, 0x00, //    CMP   dword ptr [EAX + CH],41Fh
    0x74, 0x03,                               //    JZ    after_lock_on_flag_owner
                                              // before_lock_on_flag_owner:
    0x89, 0x4A, 0x30,                         //    MOV   dword ptr [EDX + 30h],ECX
                                              // after_lock_on_flag_owner:
};

std::string lock_camera_on_flag_owner_1_mask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

BYTE lock_camera_on_flag_owner_1_prepend[] = {
    0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00, //    MOV   EBP,dword ptr [GAME_INSTANCE_ADDRESS]
    0x8B, 0x85, 0x00, 0x00, 0x00, 0x00, //    MOV   EAX,dword ptr [EBP + player_offsets[pid]]
    0x39, 0xD8,                         //    CMP   EAX,EBX
    0x74, 0x18,                         //    JZ    after_lock_on_desired_player
    0x39, 0x98, 0x40, 0x01, 0x00, 0x00, //    CMP   dword ptr [EAX + 140h],EBX
    0x7F, 0x0B,                         //    JG    before_lock_on_desired_player
    0x8B, 0xA8, 0x80, 0x02, 0x00, 0x00, //    MOV   EBP,dword ptr [EAX + 280h]
    0x38, 0x5D, 0x7C,                   //    CMP   byte ptr [EBP + 7Ch],BL
    0x74, 0x05,                         //    JZ    after_lock_on_desired_player
                                        // before_lock_on_desired_player:
    0x89, 0x42, 0x30,                   //    MOV   dword ptr [EDX + 30h],EAX
    0xEB, 0x1D,                         //    JMP   after_lock_on_flag_owner
                                        // after_lock_on_desired_player:
};

BYTE lock_camera_on_flag_owner_2[] = {
    0x8B, 0x41, 0x30,       //    MOV   EAX,dword ptr [ECX + 30h]
    0x3B, 0xC6,             //    CMP   EAX,ESI
    0x74, 0x0D,             //    JZ    after_lock_on_flag_owner
    0x3B, 0xC3,             //    CMP   EAX,EBX
    0x74, 0x06,             //    JZ    before_lock_on_flag_owner
    0x83, 0x78, 0x08, 0x03, //    CMP   dword ptr [EAX + 8h],3h
    0x74, 0x03,             //    JZ    after_lock_on_flag_owner
                            // before_lock_on_flag_owner:
    0x89, 0x71, 0x30,       //    MOV   dword ptr [ECX + 30h],ESI
                            // after_lock_on_flag_owner:
};

std::string lock_camera_on_flag_owner_2_mask = "xxxxxxxxxxxxxxxxx";

BYTE lock_camera_on_flag_owner_2_prepend[] = {
    0x8B, 0x2D, 0x00, 0x00, 0x00, 0x00, //    MOV   EBP,dword ptr [GAME_INSTANCE_ADDRESS]
    0x8B, 0x85, 0x00, 0x00, 0x00, 0x00, //    MOV   EAX,dword ptr [EBP + player_offsets[pid]]
    0x39, 0xD8,                         //    CMP   EAX,EBX
    0x74, 0x18,                         //    JZ    after_lock_on_desired_player
    0x39, 0x98, 0x40, 0x01, 0x00, 0x00, //    CMP   dword ptr [EAX + 140h],EBX
    0x7F, 0x0B,                         //    JG    before_lock_on_desired_player
    0x8B, 0xA8, 0x80, 0x02, 0x00, 0x00, //    MOV   EBP,dword ptr [EAX + 280h]
    0x38, 0x5D, 0x7C,                   //    CMP   byte ptr [EBP + 7Ch],BL
    0x74, 0x05,                         //    JZ    after_lock_on_desired_player
                                        // before_lock_on_desired_player:
    0x89, 0x41, 0x30,                   //    MOV   dword ptr [ECX + 30h],EAX
    0xEB, 0x14,                         //    JMP   after_lock_on_flag_owner
                                        // after_lock_on_desired_player:
};

size_t game_instance_address_offset = 2;
size_t player_address_offset        = 8;

Address instance_player_offsets[] = {
    0x00440684,
    0x00440688,
};

OwnCameraPatch::OwnCameraPatch(int pid, std::shared_ptr<Spelunky> spel) : Patch(spel),
        is_valid(false),
        lock_camera_on_flag_owner_1_data(NULL),
        lock_camera_on_flag_owner_2_data(NULL) 
{
    kill_player_address = spel->find_exec_mem(kill_player, kill_player_mask);
    if (kill_player_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that kills player outside of camera." << std::endl);
        return;
    }

    ghost_spawn_on_start_address = spel->find_exec_mem(ghost_spawn_on_start, ghost_spawn_on_start_mask);
    if (ghost_spawn_on_start_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that spawns an player ghost on level start." << std::endl);
        return;
    }

    ghost_spawn_on_death_address = spel->find_exec_mem(ghost_spawn_on_death, ghost_spawn_on_death_mask);
    if (ghost_spawn_on_death_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that spawns a player ghost on death." << std::endl);
        return;
    }

    game_instance_usage_address = spel->find_exec_mem(game_instance_usage, game_instance_usage_mask);
    if (game_instance_usage_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that uses a game instance." << std::endl);
        return;
    }

    spel->read_mem(game_instance_usage_address + game_instance_usage_offset, &game_instance_address, sizeof(game_instance_address));

    lock_camera_on_flag_owner_1_address = spel->find_exec_mem(lock_camera_on_flag_owner_1, lock_camera_on_flag_owner_1_mask);
    if (lock_camera_on_flag_owner_1_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that locks camera on flag owner." << std::endl);
        return;
    }

    lock_camera_on_flag_owner_2_address = spel->find_exec_mem(lock_camera_on_flag_owner_2, lock_camera_on_flag_owner_2_mask);
    if (lock_camera_on_flag_owner_2_address == 0x0) {
        DBG_EXPR(std::cout << "[OwnCameraPatch] Failed to find code that locks camera on flag owner." << std::endl);
        return;
    }

    lock_camera_on_flag_owner_1_data = spel->allocate(EXEC_SPACE_ALLOC, true);
    spel->write_mem(lock_camera_on_flag_owner_1_data, lock_camera_on_flag_owner_1_prepend, sizeof(lock_camera_on_flag_owner_1_prepend));
    spel->write_mem(lock_camera_on_flag_owner_1_data + game_instance_address_offset, &game_instance_address, sizeof(game_instance_address));
    spel->write_mem(lock_camera_on_flag_owner_1_data + player_address_offset, &instance_player_offsets[pid], sizeof(instance_player_offsets[pid]));
    
    lock_camera_on_flag_owner_2_data = spel->allocate(EXEC_SPACE_ALLOC, true);
    spel->write_mem(lock_camera_on_flag_owner_2_data, lock_camera_on_flag_owner_2_prepend, sizeof(lock_camera_on_flag_owner_2_prepend));
    spel->write_mem(lock_camera_on_flag_owner_2_data + game_instance_address_offset, &game_instance_address, sizeof(game_instance_address));
    spel->write_mem(lock_camera_on_flag_owner_2_data + player_address_offset, &instance_player_offsets[pid], sizeof(instance_player_offsets[pid]));

    is_valid = true;
}

OwnCameraPatch::~OwnCameraPatch() {
    if (lock_camera_on_flag_owner_1_data != NULL) {
        spel->release(lock_camera_on_flag_owner_1_data);
    }

    if (lock_camera_on_flag_owner_2_data != NULL) {
        spel->release(lock_camera_on_flag_owner_2_data);
    }
}

bool OwnCameraPatch::valid() {
    return is_valid;
}

bool OwnCameraPatch::_perform() {
    assert(is_valid);

    spel->write_mem(kill_player_address, kill_player_patched, sizeof(kill_player_patched));
    spel->write_mem(ghost_spawn_on_start_address, ghost_spawn_on_start_patched, sizeof(ghost_spawn_on_start_patched));
    spel->write_mem(ghost_spawn_on_death_address, ghost_spawn_on_death_patched, sizeof(ghost_spawn_on_death_patched));
    spel->jmp_build(lock_camera_on_flag_owner_1_address, sizeof(lock_camera_on_flag_owner_1), lock_camera_on_flag_owner_1_data, sizeof(lock_camera_on_flag_owner_1_prepend));
    spel->jmp_build(lock_camera_on_flag_owner_2_address, sizeof(lock_camera_on_flag_owner_2), lock_camera_on_flag_owner_2_data, sizeof(lock_camera_on_flag_owner_2_prepend));

    DBG_EXPR(std::cout << "[OwnCameraPatch] Own camera patch applied." << std::endl);

    return true;
}

bool OwnCameraPatch::_undo() {
    assert(is_valid);

    spel->write_mem(kill_player_address, kill_player, sizeof(kill_player));
    spel->write_mem(ghost_spawn_on_start_address, ghost_spawn_on_start, sizeof(ghost_spawn_on_start));
    spel->write_mem(ghost_spawn_on_death_address, ghost_spawn_on_death, sizeof(ghost_spawn_on_death));
    spel->write_mem(lock_camera_on_flag_owner_1_address, lock_camera_on_flag_owner_1, sizeof(lock_camera_on_flag_owner_1));
    spel->write_mem(lock_camera_on_flag_owner_2_address, lock_camera_on_flag_owner_2, sizeof(lock_camera_on_flag_owner_2));

    DBG_EXPR(std::cout << "[OwnCameraPatch] Own camera patch unapplied." << std::endl);

    return true;
}
