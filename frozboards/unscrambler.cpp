#include "unscrambler.h"
#include <sstream>
#include <map>
#include <mutex>

namespace Frozboards {
	std::map<int, Unscrambler::membuf> strs;
	std::mutex strs_mutex;
	
	Unscrambler::Unscrambler(const membuf& scrambled) : scrambled(scrambled) {
		strs_mutex.lock();
		for(int i = 0; i <= (int)strs.size(); i++) {
			if(strs.find(i) == strs.end()) {
				id = i;
				break;
			}
		}
		
		strs[id] = scrambled;
		strs_mutex.unlock();
	}

	Unscrambler::~Unscrambler() {
		strs_mutex.lock();
		auto iter = strs.find(id);
		if(iter != strs.end()) {
			strs.erase(iter);
		}
		strs_mutex.unlock();
	}

	void Unscrambler::cycle_unscramble() {
		membuf out;

		strs_mutex.lock();
		membuf& str = strs[id];
		strs_mutex.unlock();

		unsigned char* keys = str.data();
		size_t size = str.size();
		
		for(size_t i = 0; i < size; i++) {
			out.push_back((255 - keys[i]) ^ (i+1));
		}

		strs_mutex.lock();
		strs[id] = out;
		strs_mutex.unlock();
	}
	
	const Unscrambler::membuf& Unscrambler::data() {
		strs_mutex.lock();
		auto& ret = strs[id];
		strs_mutex.unlock();

		return ret;
	}
}