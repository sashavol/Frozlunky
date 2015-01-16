#include "known_entities.h"
#include <boost/assign.hpp>
#include <map>
#include <boost/algorithm/string.hpp>
#include "tile_util.h"

//clears all flags from entity, does not need to be used externally for most use cases
int raw_entity(int entity) {
	return entity &= ~ENTITY_FLAGS;
}

namespace KnownEntities {
	using namespace boost::algorithm;

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
	
	
	/*
		(109, "109 [Unknown]")
		(110, "110 [Unknown]")
		(111, "111 [Unknown]")
		(123, "123")
		(157, "157 [Unknown]")
		(198, "198 [Unknown]")
		(204, "204 [Unknown]")
		(206, "206 [Unknown]")
		(207, "207 [Unknown]")
		(214, "214 [Unknown]")
		(222, "222 [Unknown]")
		(231, "231 [Unknown]")
		(238, "238 [Unknown Animation]")
		(241, "241 [Unknown]")
		(243, "243 [Unknown]")
		(249, "249 [Unknown]")
		(251, "251 [Unknown]")
		(253, "253 [Unknown: Floats in background]")
		(302, "302 [Unknown]")
		(304, "304 [Unknown]")
		(305, "305 [Unknown]")
		(445, "445 [Unknown]")
		(1000, "1000 [Unknown]")
		(1066, "1066 [Unknown]")
		(3001, "3001 [Unknown]")
	*/

	static std::map<int, std::string> fast_entities;
	static map_type entities = boost::assign::map_list_of
		//items
		(108, "Auto-dropping Rope")
		(107, "Auto-exploding Bomb")
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
		(526, "Kapala")
		(527, "Udjat Eye")
		(528, "Ankh")
		(529, "Hedjet")
		(530, "Scepter")
		(531, "Book of the Dead")
		(532, "Vlad's Cape")
		(533, "Vlad's Amulet")

		//enemies
		(223, "Olmec")
		(234, "Anubis II Spawner")
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
		(1049, "Black Knight")
		(1050, "Golden Monkey")
		(1058, "Turret")
		(1048, "Alien Queen")
		(1060, "Alien Queen Eye")
		(1065, "Little Alien (Horizontally Moving)")
		(1051, "Succubus")
		(1052, "Horse Head")
		(1053, "Ox Face")
		(1055, "King Yama Body")
		(1056, "King Yama Head")
		(1057, "King Yama Fist")
		(1063, "Rat Critter")
		(1061, "Spiderling Critter")
		(1059, "Blue Frog Critter")
		(1062, "Fish Critter")
		(1064, "Penguin Critter")
		(1067, "Locust Critter")
		(1068, "Maggot Critter")

		//holdable objects
		(112, "Rock")
		(114, "Skull")
		(120, "Boulder")
		(121, "Push Block")
		(122, "Arrow")
		(216, "Arrow Shaft")
		(164, "Snowball")
		(172, "Piranha Skeleton")
		(176, "Magma Cauldron")
		(210, "Mattock Head")
		(246, "Unlit Torch")
		(252, "Eggplant")

		//containers
		(100, "Chest")
		(101, "Crate")
		(113, "Pot")
		(119, "Chest (2)")
		(153, "Locked Chest (Udjat Eye)")
		(154, "Golden Key (Udjat Eye)")

		//valuables
		(102, "Single Gold Bar")
		(103, "Gold Pyramid")
		(104, "Emerald (Large)")
		(105, "Sapphire (Large)")
		(106, "Ruby (Large)")
		(118, "Gold Nugget (Large)")
		(124, "Gold Nugget (Small)")
		(240, "Zero-Value Gold Nugget")
		(125, "Emerald (Small)")
		(126, "Sapphire (Small)")
		(127, "Ruby (Small)")
		(173, "Diamond")
		(250, "Crowned Skull")
		(525, "Idol")

		//decorations / undamaging level entities
		(115, "Cobweb")
		(237, "Destroyed Cobweb")
		(116, "Sticky Honey")
		(162, "Lantern")
		(163, "Torch Light")
		(174, "Worm Uvula Entrance")
		(175, "Activated Worm")
		(177, "Wide Light Emitter")
		(188, "Roulette Wheel")
		(189, "Roulette Wheel Pin")
		(190, "Roulette Barricade")
		(227, "Mothership Lights")
		(244, "Haunted Castle Banner")
		(245, "Mounted Lightable Torch Holder")
		(4, "Ladder")
		(5, "Ladder Junction")
		(7, "Dirt Patch")
		(2, "Locked Tutorial Background")
		(12, "Background Blue Dirt Wall")
		(13, "Small Gold Crust Foreground")
		(14, "Large Gold Crust Foreground")
		(24, "Tree Branch")
		(33, "Small Tree Branch")
		(26, "Bedrock Edge Patch")
		(30, "Jungle Dirt Patch")
		(31, "Temple Block Patch")
		(3004, "Cash Register")

		//level obstacles
		(1, "Spikes")
		(197, "Alien Forcefield Ground Laser")
		(9, "Arrow Trap")
		(22, "Tiki Trap")
		(45, "Crush Trap 'Thwomp'")

		//fluids
		(27, "Water Fluid")
		(36, "Lava Fluid")
		(91, "Acid Worm Fluid")

		//platform objects
		(161, "Blue Falling Platform")
		(168, "Ending Cutscene Timed Erupting Floor")
		(169, "Ending Cutscene Timed Erupting Lava")
		(178, "Spike Ball (Detached)")
		(182, "Worm Breakable Meat")
		(183, "Cracking Ice Platform")
		(215, "Vertically-Moving Mothership Platform")
		(235, "TNT")
		(15, "Idol Altar Half")
		(23, "Tree Trunk")
		(25, "Bedrock Block")
		(32, "Invisible Solid Block")
		(35, "Sacrifice Altar Half")

		//sub-entity components
		(117, "Bullet")
		(142, "Giant Spider Cobweb Projectile")
		(156, "Used Parachute")
		(158, "Static Swing Attack Projectile")
		(159, "Psychic Attack Bubbling")
		(160, "UFO Projectile")
		(165, "Vomit Fly")
		(167, "Crush Animation")
		(171, "Co-op Player Flag")
		(179, "Breaking Chain Projectile")
		(184, "Leaf")
		(192, "Snail Bubble")
		(193, "Cobra Venom Projectile")
		(194, "Falling Icicle Projectile")
		(195, "Broken Ice Projectiles")
		(196, "Splashing Water Projectile")
		(203, "Freeze Ray Projectile")
		(213, "Turret Projectile")
		(217, "Olmec Enemy Spawn Projectile")
		(225, "Kill Target")
		(226, "Activated Kill Target Laser")
		(247, "Purple Target")
		(218, "Splashing Water")
		(228, "Broken Web Pouch")
		(236, "Spinner Spider Thread")

		//animations
		(37, "Exploding Dirt")
		(221, "Smoke Poof")
		(232, "Breaking Animation")
		(233, "Magma Flame Animation")
		(242, "Ending Cutscene Exploding Firework")
		(301, "Single-Tile Exploding Animation")
		(303, "Bounce Trap Animation")
		(306, "Lit Torch Animation")
		(307, "Extinguished Torch Animation")

		//event triggers
		(166, "Fast Level Start Event Trigger")
		(170, "5-Second Win Scheduler")
		(229, "Ankh Respawn Timed Event Trigger")
		(230, "Ankh Respawn Animation")
		(10, "Grey Background Trigger")

		//special
		(3, "Invisible Static Level Exit")
		(220, "Ball & Chain (without Chain)")
		(211, "Character Coffin")
		(212, "Character Ghost")
		(219, "Co-Op Player Item Bag")
		(180, "Tutorial Journal")
		(181, "Journal Page")
		(187, "Decoy Chest")
		(224, "Ending Cutscene Camel")
		(239, "Decoy Yang")
		(248, "Unopenable Mystery Box")
		(2001, "Background Dark Square Shadow")
		(3002, "Green-Glowing Ground Attachment")

		//unknown
		(41, "41")
		(42, "42")
		(43, "43")
		(44, "44")
		(46, "46")
		(47, "47")
		(48, "48")
		(49, "49")
		(50, "50")
		(51, "51")
		(52, "52")
		(53, "53")
		(54, "54")
		(55, "55")
		(56, "56")
		(57, "57")
		(58, "58")
		(59, "59")
		(60, "60")
		(61, "61")
		(62, "62")
		(63, "63")
		(64, "64")
		(65, "65")
		(66, "66")
		(67, "67")
		(68, "68")
		(69, "69")
		(70, "70")
		(71, "71")
		(72, "72")
		(73, "73")
		(74, "74")
		(75, "75")
		(76, "76")
		(77, "77")
		(78, "78")
		(79, "79")
		(80, "80")
		(81, "81")
		(82, "82")
		(83, "83")
		(84, "84")
		(85, "85")
		(86, "86")
		(87, "87")
		(88, "88")
		(89, "89")
		(90, "90")
		(92, "92")
		(93, "93")
		(94, "94")
		(95, "95")
		(96, "96")
		(97, "97")
		(98, "98")
		(99, "99")

		(2001, "2001")
		(3000, "3000")
		(3005, "3005")
		(3002, "3002")
		(3003, "3003")
		
		(3001, "3001")

		(9094, "9094")
		(9095, "9095")
		(9096, "9096")
		(9097, "9097")
		(9098, "9098");

	static std::map<int, unsigned> entity_colors = boost::assign::map_list_of
		//items
		(108, 0x794a2500)
		(107, 0x514f4d00)
		(500, 0x8e4f1000)
		(501, 0x454c4200)
		(502, 0x254f1000)
		(503, 0x957a0e00)
		(504, 0x26471000)
		(505, 0x47312000)
		(506, 0x81312000)
		(507, 0x70414200)
		(508, 0xaaa6a100)
		(509, 0xEE0A0A00)
		(510, 0x47474400)
		(511, 0xB3623700)
		(512, 0xad7d7e00)
		(513, 0xc3767600)
		(514, 0x037BFE00)
		(515, 0x95412500)
		(516, 0xFDD22900)
		(517, 0x7635BA00)
		(518, 0x475F8900)
		(519, 0x02478900)
		(520, 0xA2795A00)
		(521, 0xDFA22100)
		(522, 0x0153CD00)
		(523, 0xC7030300)
		(524, 0xc9340900) 
		(526, 0xE9CF9C00)
		(527, 0xFCE60000)
		(528, 0xD4D13800)
		(529, 0xDBA82800)
		(530, 0xDF7D0000)
		(531, 0x8f2e0b00)
		(532, 0xCF1B0200)
		(533, 0xC4AD2600)

		//enemies
		(223, 0xEDB60600)
		(234, 0x50375300)
		(1001, 0x61992B00)
		(1002, 0x615A5900)
		(1003, 0x8E62CB00)
		(1004, 0xFFD4B500)
		(1005, 0xCC010100)
		(1006, 0xA2483F00)
		(1007, 0x008DAF00)
		(1008, 0x63A41A00)
		(1009, 0x4C88C500)
		(1010, 0x539B2800)
		(1011, 0x91565600)
		(1012, 0xC5C5C500)
		(1013, 0x705B5A00)
		(1014, 0xf4d17600)
		(1015, 0xF4A16500)
		(1016, 0xACCB2200)
		(1017, 0xFEFDFE00)
		(1018, 0xCC404500)
		(1019, 0xD146D800)
		(1020, 0x684f6800)
		(1021, 0xED6D1000)
		(1022, 0x7A4E2300)
		(1023, 0xF95A3D00)
		(1024, 0xF0E53400)
		(1025, 0x2451e000)
		(1026, 0xA5C51A00)
		(1027, 0x56535300)
		(1028, 0xBA080000)
		(1029, 0xEED23400)
		(1030, 0xF4151400)
		(1031, 0x1571d100)
		(1032, 0xF7DC1300)
		(1033, 0xECE56A00)
		(1034, 0xf1912400)
		(1035, 0xBFC51D00)
		(1036, 0x4DB5D400)
		(1037, 0x7632F900)
		(1038, 0x55DAE900)
		(1039, 0xBAE1FA00)
		(1040, 0x8257B700)
		(1041, 0x5DAF0100)
		(1042, 0xE5471500)
		(1043, 0xa89b4000)
		(1044, 0x65B00F00)
		(1045, 0x69ED4200)
		(1046, 0xB2B21A00)
		(1047, 0xD0A70F00)
		(1049, 0x72595900)
		(1050, 0xEEE10000)
		(1058, 0x82828200)
		(1048, 0xA9CC1B00)
		(1060, 0x812db400)
		(1065, 0x8fab1700)
		(1051, 0x48BAC800)
		(1052, 0xF3BB4700)
		(1053, 0x3862D300)
		(1055, 0x8A34C900)
		(1056, 0xBD5B4200)
		(1057, 0xbe312c00)
		(1063, 0x87838300)
		(1061, 0x6F675C00)
		(1059, 0x95d6f900)
		(1062, 0x58533B00)
		(1064, 0x3F658900)
		(1067, 0xC4B34600)
		(1068, 0xE3D0B700)

		//holdable objects
		(112, 0x80808000)
		(114, 0xb0b0b000)
		(120, 0x8F765300)
		(121, 0x9f926f00)
		(122, 0xAFA99B00)
		(216, 0xa4582d00)
		(164, 0x9e9eff00)
		(172, 0xc3949400)
		(176, 0x92544200)
		(210, 0x87878700)
		(246, 0xA4694600)
		(252, 0x6C28BD00)

		//containers
		(100, 0xB13B1200) 
		(101, 0xAD8A3D00)
		(113, 0xD6BBA500)
		(119, 0x8f4b3400)
		(153, 0xa8581100)
		(154, 0xFFF82000)

		//valuables
		(102, 0xC9C70000)
		(103, 0xfffd2400)
		(104, 0xC7F86400)
		(105, 0x78D0F800)
		(106, 0xE06D8500)
		(118, 0xCBA74200)
		(124, 0xB47D2C00)
		(240, 0xaa7d0d00)
		(125, 0x38831B00)
		(126, 0x276ba500)
		(127, 0xC4182700)
		(173, 0xFDFDFD00)
		(250, 0xF3F61500)
		(525, 0xFFFF4E00)

		//decorations / undamaging level entities
		(115, 0xC6D1D500)
		(237, 0x6c899300)
		(116, 0xDFB63000)
		(162, 0xFFF92500)
		(163, 0xFF794A00)
		(174, 0xFF895400)
		(175, 0xff575400)
		(188, 0x7E8B2200)
		(189, 0xBA030000)
		(190, 0x7A3A1400)
		(227, 0x0B79FF00)
		(244, 0x4a29d100)
		(245, 0x855A4700)

		//level obstacles
		(197, 0xFF13FE00)

		//platform objects
		(161, 0x667EE200)
		(168, 0xa3675a00)
		(169, 0xFE5A0E00)
		(178, 0x908F8D00)
		(182, 0xF8906900)
		(183, 0xAAF5FF00)
		(215, 0x65419000) 
		(235, 0xc20e0500)

		//specialized entities
		(1, 0xFFFFFF00)
		(2, 0x728ea600)
		(4, 0xAF5F3C00)
		(5, 0x94503300)
		(7, 0xbc722f00)
		(9, 0x81746800)
		(11, 0xFFFFFF00)
		(12, 0x536e8300)
		(13, 0xc6983400)
		(14, 0xd2ab5500)
		(15, 0xa8988500)
		(22, 0x9c5a4700)
		(23, 0xc1792f00)
		(24, 0xbd762e00)
		(25, 0x826C5E00)
		(26, 0xAA897100)
		(27, 0xAAAAFF00)
		(30, 0x81950000)
		(31, 0x96785f00)
		(33, 0x7b4d1e00)
		(35, 0xd62e2400)
		(36, 0xfe4b1a00)
		(37, 0xA6550D00)
		(40, 0x99856600)
		(45, 0x8b898900)
		(91, 0x6cab5900)
		(3004, 0x8f3c0000);

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

		int raw = raw_entity(entity);
		auto namep = fast_entities.find(raw);
		if(namep == fast_entities.end()) {
			return std::to_string(raw) + std::string(" [Unknown]");
		}

		return namep->second;
	}

	unsigned GetColor(int entity) {
		entity = raw_entity(entity);

		unsigned color = entity_colors[entity];
		if(color)
			return color;
		else
			return (entity*0x14362218) & 0xFFFFFF00;
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


	///////////
	// searching
	///////////

	std::vector<std::string> Search(const std::string& query) {
		std::vector<std::string> all;
		for(const map_type::value_type& v : entities) {
			all.push_back(v.second);
		}
		return TileUtil::Search(all, query);
	}
}