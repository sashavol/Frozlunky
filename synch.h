#pragma once

#include "synch_data.h"
#include <string>

namespace MP {
	class Synchronizer {
	private:
		std::string id;
	
	public:
		Synchronizer(const std::string& identifier) : id(identifier) {}

		const std::string& identifier() {
			return id;
		}

		virtual std::shared_ptr<SynchData> retrieve() = 0;
		virtual void synchronize(std::shared_ptr<SynchData> data) = 0;
	};
}