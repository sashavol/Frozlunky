#include "synch_data.h"

namespace MP {
	SynchData::SynchData() {}
	SynchData::~SynchData() {
		for(std::pair<std::string, sized_array> pair : data) {
			delete pair.second.first;
		}
	}

	void SynchData::set(const std::string& key, const sized_array& seg) {
		auto existing = data.find(key);
		if(existing != data.end()) {
			delete existing->second.first;
		}

		data[key] = seg;
	}

	SynchData::sized_array SynchData::get(const std::string& key) {
		auto find = data.find(key);
		if(find == data.end()) {
			return sized_array(nullptr, 0);
		}
		else {
			return find->second;
		}
	}

	void SynchData::dump(std::vector<char>& out) {
		//key_len<4>|key<key_len>|data_len<4>|data<data_len>
		for(std::pair<std::string, sized_array> pair : data) {
			unsigned key_len = pair.first.size();
			std::copy(&key_len, &key_len+sizeof(unsigned), std::back_inserter(out));
			std::copy(pair.first.begin(), pair.first.end(), std::back_inserter(out));

			unsigned data_len = pair.second.second;
			std::copy(&data_len, &data_len+sizeof(unsigned), std::back_inserter(out));

			BYTE* data = pair.second.first;
			std::copy(data, data+data_len, std::back_inserter(out));
		}

		for(int i = 0; i < 4; i++)
			out.push_back(0);
	}

	std::shared_ptr<SynchData> SynchData::Parse(const char* data) {
		std::shared_ptr<SynchData> sd = std::make_shared<SynchData>();

		char keybuf[128];
		while(*(unsigned*)data != 0) {
			unsigned len = *(unsigned*)data;
			data += sizeof(unsigned);

			keybuf[len] = 0;
			std::memcpy(keybuf, data, len);
			data += len;

			len = *(unsigned*)data;
			data += sizeof(unsigned);

			BYTE* data_buf = new BYTE[len];
			std::memcpy(data_buf, data, len);
			data += len;

			sd->data[std::string(keybuf)] = std::pair<BYTE*, unsigned>(data_buf, len);
		}

		return sd;
	}
}