#include "known_entities.h"
#include <boost/assign.hpp>
#include <map>
#include <boost/algorithm/string.hpp>

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
		(531, "Hell Bible")
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
		(122, "Arrow (with tip)")
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
		(245, "Mounted Lightable Torch")

		//level obstacles
		(197, "Alien Forcefield Ground Laser")

		//platform objects
		(161, "Blue Falling Platform")
		(168, "Ending Cutscene Timed Erupting Floor")
		(169, "Ending Cutscene Timed Erupting Lava")
		(178, "Spike Ball (Detached)")
		(182, "Worm Breakable Meat")
		(183, "Cracking Ice Platform")
		(215, "Vertically-Moving Mothership Platform")
		(235, "TNT")

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
		(196, "Water Splash Projectile")
		(203, "Freeze Ray Projectile")
		(213, "Turret Projectile")
		(217, "Olmec Enemy Spawn Projectile")
		(225, "Kill Target")
		(226, "Activated Kill Target Laser")
		(247, "Purple Target")
		(218, "Water Splash")
		(228, "Broken Web Pouch")
		(236, "Spinner Spider Thread")

		//animations
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

		//special
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


	///////////
	// searching
	///////////

	static double score_max_consec(const std::string* s1p, const std::string* s2p) {
		if(s2p->size() < s1p->size())
			std::swap(s1p, s2p);

		const std::string& s1 = *s1p;
		const std::string& s2 = *s2p;
		
		int max_cons = 0;

		for(int p2 = 0, m2 = s2.size(); p2 < m2; ++p2) {
			int cons = 0;
			for(int p1 = 0, m1 = s1.size(); p1 < m1 && p1+p2 < m2; ++p1) {
				if(s1[p1] == s2[p2+p1]) {
					cons++;
				}
			}	
			max_cons = std::max(max_cons, cons);
		}

		return max_cons / (double)s1.size();
	}
	
	static double score_charcomp(const std::string& s1, const std::string& s2) {
		int same = 0;
		for(char c1 : s1) {
			for(char c2 : s2) {
				if(c1 == c2)
					same++;
			}
		}
		return same / (double)(s1.size()*s2.size());
	}

	static double score_base_equality(const std::string* s1, const std::string* s2) {
		if(s1->size() > s2->size())
			std::swap(s1, s2);

		const std::string& a = *s1;
		const std::string& b = *s2;

		int count = 0;
		for(int p1 = 0, m1 = a.size(); p1 < m1; ++p1) {
			if(a[p1] == b[p1])
				count++;
		}

		return (double)count / (double)a.size();
	}

	static void string_normalize(std::string& str) {
		trim(str);
		to_upper(str);
		replace_all(str, " ", "");
	}

	std::vector<std::string> Search(const std::string& query) {
		if(query.empty())
			return std::vector<std::string>();

		auto split_pred = boost::is_any_of(" ");
		
		std::string comp_query = query;
		string_normalize(comp_query);

		std::map<std::string, double> scores;
		std::vector<std::string> out;
		
		for(const map_type::value_type& et : entities) {
			std::string comp = et.second;
			string_normalize(comp);
			
			if(comp == comp_query)
				scores[et.second] = 10000;
			else
				scores[et.second] = (score_base_equality(&comp, &comp_query) + score_max_consec(&comp, &comp_query) + score_charcomp(comp, comp_query));
			
			out.push_back(et.second);
		}

		std::sort(out.begin(), out.end(), [&](const std::string& a, const std::string& b) {
			return scores[a] > scores[b];
		});

		return out;
	}
}