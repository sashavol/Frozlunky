#include "seed_synchronizer.h"

namespace MP {
	SeedSynchronizer::SeedSynchronizer(std::shared_ptr<Seeder> seeder) : Synchronizer("seed"), seeder(seeder), last_hash(0) {}

	std::shared_ptr<SynchData> SeedSynchronizer::retrieve() {
		std::shared_ptr<SynchData> ret = std::make_shared<SynchData>();

		std::string seed = seeder->get_seed();
		seed_hash current = std::hash<std::string>()(seed);
		if(last_hash != current) {
			ret->set("s", SynchData::sized_array((BYTE*)seed.c_str(), seed.size()+1));
			last_hash = current;
		}

		return ret;
	}

	void SeedSynchronizer::synchronize(std::shared_ptr<SynchData> data) {
		SynchData::sized_array arr = data->get("s");

		if(arr.first != nullptr) {
			std::string seed_synch((char*)arr.first);
			seeder->seed(seed_synch);
			last_hash = std::hash<std::string>()(seed_synch);
		}
	}
}
