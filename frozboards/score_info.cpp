#include "score_info.h"

#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace Frozboards {
	std::string TrackedValues::encode() {
		std::stringstream ss;
		ss << "{" << std::setbase(10) << gold << "/" << health << "/" << bombs << "/" << ropes;
		ss << std::setbase(10) << "/" << level_time << "/" << total_time << "/" << level << "}";
		return ss.str();
	}

	std::string EncodeMulti(TrackedValuesSet& values) 
	{
		std::stringstream ss;
		ss << "[";
		for(TrackedValues& tv : values) {
			ss << tv.encode();
			ss << ",";
		}
		
		std::string result = ss.str();
		if(result.size() == 1) {
			return "[]";
		}
		else {
			return result.substr(0, result.size()-1) + "]";
		}
	}
}