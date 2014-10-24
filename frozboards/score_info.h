#pragma once

#include <vector>
#include <string>


namespace Frozboards {
	struct TrackedValues {
		unsigned gold;
		unsigned health;
		unsigned bombs;
		unsigned ropes;

		double level_time;
		double total_time;

		int level;

		std::string encode();
	};

	typedef std::vector<TrackedValues> TrackedValuesSet;

	std::string EncodeMulti(TrackedValuesSet& values);
}