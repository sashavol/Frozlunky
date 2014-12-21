#include "known_entities.h"
#include <boost/assign.hpp>
#include <map>

namespace KnownEntities {
	/*static int entity_ids[] = {1,2,3,4,5,7,9,10,11,12,13,14,15,16,17,18,19,21,22,23,24,25,26,27,28,30,31,32,
		33,34,35,36,37,38,39,40,41,42,43,44,45,46,48,49,50,51,53,54,55,56,57,59,60,61,62,63,64,65,66,67,68,
		69,70,71,72,74,75,76,77,78,79,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,
		103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,120,121,122,123,124,125,126,127,
		142,153,154,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
		177,178,179,180,181,182,183,184,187,188,189,190,192,193,194,195,196,197,198,203,204,206,207,210,
		211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,
		235,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,301,302,303,304,
		305,306,307,445,500,501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,
		520,521,522,523,524,525,526,527,528,529,530,531,532,533,1000,1001,1002,1003,1004,1005,1006,1007,
		1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1019,1020,1021,1022,1023,1024,1025,1026,
		1027,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1038,1039,1040,1041,1042,1043,1044,1045,
		1046,1047,1048,1049,1050,1051,1052,1053,1055,1056,1057,1058,1059,1060,1061,1062,1063,1064,1065,
		1066,1067,1068,2001,3000,3001,3002,3003,3004,3005,9095,9097};*/
	
	static std::map<int, std::string> fast_entities;
	static map_type entities = boost::assign::map_list_of
		(100, "Chest")
		(101, "Crate")
		(102, "Single Gold Bar")
		(103, "Gold Pyramid")
		(104, "Emerald (Large)")
		(105, "Sapphire (Large)")
		(106, "Ruby (Large)")
		(107, "Auto-exploding Bomb")
		(108, "Auto-dropping Rope")
		(109, "109 [Unknown]")
		(110, "110 [Unknown]")
		(111, "111 [Unknown]")
		(112, "Rock")
		(113, "Pot")
		(114, "Skull")
		(115, "Cobweb")
		(116, "Sticky Honey")
		(117, "Bullet")
		(118, "Gold Nugget (Large)")
		(119, "Chest (2)")
		(120, "Boulder")
		(121, "Push Block")
		(122, "Arrow (with tip)")
		(123, "123")
		(124, "Gold Nugget (Small)")
		(125, "Emerald (Small)")
		(126, "Sapphire (Small)")
		(127, "Ruby (Small)")
		(142, "Giant Spider Cobweb Projectile")
		(153, "Locked Chest (Udjat Eye)")
		(154, "Golden Key (Udjat Eye)")
		(156, "Used Parachute")
		(157, "157 [Unknown]")
		(158, "Static Swing Attack Animation")
		(159, "Psychic Attack Bubbling")
		(160, "UFO Projectile")
		(161, "Blue Falling Platform")
		(162, "Lantern")
		(163, "Torch Light")
		(164, "Snowball")
		(165, "Vomit Fly")
		(166, "Fast Level Start Event Trigger")
		(167, "Crush Animation")
		(168, "Ending Cutscene Timed Erupting Floor")
		(169, "Ending Cutscene Timed Erupting Lava")
		(170, "5-Second Win Scheduler")
		(171, "Co-op Player Flag")
		(172, "Piranha Skeleton")
		(173, "Diamond")
		(174, "Worm Uvula Entrance")
		(175, "Activated Worm")
		(176, "Magma Cauldron")
		(177, "Wide Light Emitter")
		(178, "Spike Ball (Detached)")
		(179, "Breaking Chain Projectile")
		(180, "Tutorial Journal")
		(181, "Journal Page")
		(182, "Worm Breakable Meat")
		(183, "Cracking Ice Platform")
		(184, "Leaf")
		(187, "Decoy Chest")
		(188, "Roulette Wheel")
		(189, "Roulette Wheel Pin")
		(190, "Roulette Barricade")
		(192, "Snail Bubble")
		(193, "Cobra Venom Projectile")
		(194, "Falling Icicle Projectile")
		(195, "Broken Ice Projectiles")
		(196, "Water Splash Projectile")
		(197, "Activated Alien Laser Projectile")
		(198, "198 [Unknown]")
		(203, "Freeze Ray Projectile")
		(204, "204 [Unknown]")
		(206, "206 [Unknown]")
		(207, "207 [Unknown]")
		(210, "Mattock Head")
		(211, "Character Coffin")
		(212, "Character Ghost")
		(213, "Turret Projectile")
		(214, "214 [Unknown]")
		(215, "Vertically-Moving Mothership Platform")
		(216, "Arrow Shaft")
		(217, "Olmec Enemy Spawn Projectile")
		(218, "Water Splash")
		(219, "Player Entity")
		(220, "Ball & Chain (without Chain)")
		(221, "Smoke Poof")
		(222, "222 [Unknown]")
		(223, "Olmec")
		(224, "Ending Cutscene Camel")
		(225, "Kill Target")
		(226, "Activated Kill Target Laser")
		(227, "Breaking Mothership Lights")
		(228, "Broken Web Pouch")
		(229, "Ankh Respawn Timed Event Trigger")
		(230, "Ankh Respawn Animation")
		(231, "231 [Unknown]")
		(232, "Breaking Animation")
		(233, "Magma Flame Animation")
		(234, "Anubis II Spawner")
		(235, "TNT")
		(236, "Breaking Spinner Spider Thread")
		(237, "Destroyed Cobweb")
		(238, "238 [Unknown Animation]")
		(239, "Decoy Yang")
		(240, "Zero-Value Gold Nugget")
		(241, "241 [Unknown]")
		(242, "Ending Cutscene Exploding Firework")
		(243, "243 [Unknown]")
		(244, "Haunted Castle Banner")
		(245, "Mounted Lightable Torch")
		(246, "Unlit Torch")
		(247, "Purple Target")
		(248, "Unopenable Mystery Box")
		(249, "249 [Unknown]")
		(250, "Crowned Skull")
		(251, "251 [Unknown]")
		(252, "Eggplant")
		(253, "253 [Unknown: Floats in background]")
		(301, "Single-Tile Exploding Animation")
		(302, "302 [Unknown]")
		(303, "Bounce Trap Animation")
		(304, "304 [Unknown]")
		(305, "305 [Unknown]")
		(306, "Lit Torch Animation")
		(307, "Extinguished Torch Animation")
		(445, "445 [Unknown]")
		(500, "Rope Pile")
		(501, "Bomb Bag")
		(502, "Bomb Box")
		(503, "Spectacles")
		(504, "Climbing Gloves")
		(505, "Pitcher's Mitt")
		(506, "Spring Shoes")
		(507, "Spike Shoes")
		(508, "Bomb Paste")
		(509, "Compass")
		(510, "Mattock")
		(511, "Boomerang")
		(512, "Machete")
		(513, "Crysknife")
		(514, "Web Gun")
		(515, "Shotgun")
		(516, "Freeze Ray")
		(517, "Plasma Cannon")
		(518, "Camera")
		(519, "Teleporter")
		(520, "Parachute")
		(521, "Cape")
		(522, "Jetpack")
		(523, "Shield")
		(524, "Queen Bee Jelly")
		(525, "Idol")
		(526, "Kapala")
		(527, "Udjat Eye")
		(528, "Ankh")
		(529, "Hedjet")
		(530, "Scepter")
		(531, "Hell Bible")
		(532, "Vlad's Cape")
		(533, "Vlad's Amulet")
		(1000, "1000 [Unknown]")
		(1001, "Snake")
		(1002, "Spider")
		(1003, "Bat")
		(1004, "Caveman")
		(1005, "Damsel")
		(1006, "Shopkeeper")
		(1007, "Blue Frog")
		(1008, "Mantrap")
		(1009, "Yeti")
		(1010, "UFO")
		(1011, "Hawk Man")
		(1012, "Skeleton")
		(1013, "Piranha")
		(1014, "Mummy")
		(1015, "Monkey")
		(1016, "Alien Lord")
		(1017, "Ghost")
		(1018, "Giant Spider")
		(1019, "Jiang Shi")
		(1020, "Vampire")
		(1021, "Orange Frog")
		(1022, "Tunnel Man")
		(1023, "Old Bitey")
		(1024, "Golden Scarab")
		(1025, "Yeti King")
		(1026, "Little Alien")
		(1027, "Fish Critter")
		(1028, "Vlad")
		(1029, "Scorpion")
		(1030, "Imp")
		(1031, "Blue Devil")
		(1032, "Bee")
		(1033, "Anubis")
		(1034, "Queen Bee")
		(1035, "Bacterium")
		(1036, "Cobra")
		(1037, "Spinner Spider")
		(1038, "Big Frog")
		(1039, "Mammoth")
		(1040, "Alien Tank")
		(1041, "Tiki Man")
		(1042, "Scorpion Fly")
		(1043, "Snail")
		(1044, "Croc Man")
		(1045, "Green Knight")
		(1046, "Worm Egg")
		(1047, "Worm Baby")
		(1048, "Alien Queen")
		(1049, "Black Knight")
		(1050, "Golden Monkey")
		(1051, "Female Damsel")
		(1052, "Horse Head")
		(1053, "Ox Face")
		(1055, "King Yama Body")
		(1056, "King Yama Head")
		(1057, "King Yama Fist")
		(1058, "Turret")
		(1059, "Blue Frog Critter")
		(1060, "Alien Queen Eye")
		(1061, "Spiderling Critter")
		(1062, "Fish Critter")
		(1063, "Rat Critter")
		(1064, "Penguin Critter")
		(1065, "Little Alien (Horizontally Moving)")
		(1066, "1066 [Unknown]")
		(1067, "Locust Critter")
		(1068, "Maggot Critter")
		(2001, "Background Dark Square Shadow")
		(3001, "3001 [Unknown]")
		(3002, "Green-Glowing Ground Attachment");

	static void ensure_fast() {
		if(fast_entities.empty()) {
			for(const std::pair<int, std::string>& e : entities) {
				fast_entities[e.first] = e.second;
			}
		}
	}

	const map_type& All() {
		return entities;
	}

	std::string GetName(int entity) {
		ensure_fast();

		auto namep = fast_entities.find(entity);
		if(namep == fast_entities.end()) {
			return std::to_string(entity) + std::string(" [Unknown]");
		}

		return namep->second;
	}

	int GetID(const std::string& name) {
		if(name.find("]") == name.size()-1) {
			try {
				return std::stoi(name.substr(0, name.find(" [")));
			}
			catch(std::exception&) {
				return 0;
			}
		}
		
		for(const std::pair<int, std::string>& e : entities) {
			if(e.second == name)
				return e.first;
		}
		
		return 0;
	}
}