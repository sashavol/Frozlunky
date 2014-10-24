#pragma once

#include <string>

#define ITEM_PLASMA_CANNON 517
#define ITEM_JETPACK 522
#define ITEM_FREEZE_RAY 516
#define ITEM_CAPE 521
#define ITEM_SHOTGUN 515
#define ITEM_MATTOCK 510
#define ITEM_TELEPORTER 519
#define ITEM_CLIMBING_GLOVES 504
#define ITEM_SPECTACLES 503
#define ITEM_WEB_GUN 514
#define ITEM_CAMERA 518
#define ITEM_PITCHERS_MITT 505
#define ITEM_PASTE 508
#define ITEM_SPRING_SHOES 506
#define ITEM_SPIKE_SHOES 507
#define ITEM_BOOMERANG 511
#define ITEM_MACHETE 512
#define ITEM_BOMB_BOX 502
#define ITEM_COMPASS 509
#define ITEM_PARACHUTE 520
#define ITEM_BOMB_BAG 501
#define ITEM_ROPES 500

typedef unsigned ItemID;

std::string GetItemFriendlyName(ItemID item);
