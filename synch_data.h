#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <Windows.h>

namespace MP {
	class SynchData {
	public:
		typedef std::pair<BYTE*, unsigned> sized_array;
		typedef std::map<std::string, sized_array> synch_data;

	private:
		synch_data data;

		//no copy
		SynchData(SynchData& other) {}

	public:
		SynchData();
		~SynchData();

		void set(const std::string& key, const sized_array& seg);
		sized_array get(const std::string& key);

		void dump(std::vector<char>& out);

		static std::shared_ptr<SynchData> Parse(const char* data);
	};
}