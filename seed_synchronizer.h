#pragma once

#include "synch.h"
#include "seeder.h"

namespace MP {
	class SeedSynchronizer : public Synchronizer {
		typedef unsigned seed_hash;

	private:
		std::shared_ptr<Seeder> seeder;
		seed_hash last_hash;

	public:
		SeedSynchronizer(std::shared_ptr<Seeder> seeder);

		virtual std::shared_ptr<SynchData> retrieve() override;
		virtual void synchronize(std::shared_ptr<SynchData> data) override;
	};
}