#include "items.h"

#include <map>
#include <sstream>
#include <boost/assign.hpp>

std::map<ItemID, std::string> item_friendly_names = boost::assign::map_list_of
	(ITEM_PLASMA_CANNON, "Plasma Cannon")
	(ITEM_JETPACK, "Jetpack")
	(ITEM_FREEZE_RAY, "Freeze Ray")
	(ITEM_CAPE, "Cape")
	(ITEM_SHOTGUN, "Shotgun")
	(ITEM_MATTOCK, "Mattock")
	(ITEM_TELEPORTER, "Teleporter")
	(ITEM_CLIMBING_GLOVES, "Climbing Gloves")
	(ITEM_SPECTACLES, "Spectacles")
	(ITEM_WEB_GUN, "Web Gun")
	(ITEM_CAMERA, "Camera")
	(ITEM_PITCHERS_MITT, "Pitcher's Mitt")
	(ITEM_PASTE, "Bomb Paste")
	(ITEM_SPRING_SHOES, "Spring Shoes")
	(ITEM_SPIKE_SHOES, "Spike Shoes")
	(ITEM_BOOMERANG, "Boomerang")
	(ITEM_MACHETE, "Machete")
	(ITEM_BOMB_BOX, "Bomb Box")
	(ITEM_COMPASS, "Compass")
	(ITEM_PARACHUTE, "Parachute")
	(ITEM_BOMB_BAG, "Bomb Bag")
	(ITEM_ROPES, "Ropes");

std::string GetItemFriendlyName(ItemID id) {
	auto name_iter = item_friendly_names.find(id);
	if(name_iter != item_friendly_names.end()) {
		return name_iter->second;
	}
	else {
		std::stringstream ss;
		ss << "[" << id << "] (Unknown)";
		return ss.str();
	}
}