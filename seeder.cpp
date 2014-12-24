#include "seeder.h"

void Seeder::blank_seed() {
	if(!dp->is_active()) {
		dp->perform();
	}

	const int size = RAND_ARR_SIZE/sizeof(int);
	Address bytes = dp->random_bytes_address();
	int bytes_data[size] = {};

	for(int i = 0; i < size; i++) {
		bytes_data[i] = 1 + (i % 8)*30;
	}

	dp->spel->write_mem(bytes, bytes_data, size*sizeof(int));
}

void Seeder::crazy_seed(int crazy, const std::string& seed, int crazyval) 
{
	if(!dp->is_active()) {
		dp->perform();
	}

	const int size = RAND_ARR_SIZE/sizeof(int);
	Address bytes = dp->random_bytes_address();
	int bytes_data[size] = {};

	srand(std::hash<std::string>()(seed));

	for(int i = 0; i < size; i++) {
		if(rand() % crazy == 0) {
			bytes_data[i] = crazyval;
		}
		else
			bytes_data[i] = rand();
	}

	dp->spel->write_mem(bytes, bytes_data, size*sizeof(int));
}

bool Seeder::is_locked() {
	return locked;
}

std::string Seeder::get_seed() {
	return *current_str_seed;
}

bool Seeder::lock() {
	if(locked)
		return false;

	locked = true;
	return true;
}

void Seeder::unlock() {
	locked = false;
}

void Seeder::on_seed_change(seed_change_cb cb) {
	seed_change_cbs.push_back(cb);
}

void Seeder::push_seed() {
	stack.push_back(*current_str_seed);
}

void Seeder::pop_seed() {
	seed(stack.back());
	stack.pop_back();
}

void Seeder::seed(Seed seed) 
{
	if(locked) {
		return;
	}

	if(!dp->is_active()) {
		dp->perform();
	}

	current_seed = seed;

	Address bytes = dp->random_bytes_address();

	const size_t size = RAND_ARR_SIZE/sizeof(int);
	uint32_t bytes_data[size];

	std::uniform_int_distribution<uint32_t> distrib(0, 0xffffffff);
	std::mt19937_64 engine;
	engine.seed(seed);

	for(int i = 0; i < size; i++) {
		bytes_data[i] = distrib(engine);
	}

	dp->spel->write_mem(bytes, bytes_data, size*sizeof(int));
}

void Seeder::seed(const std::string& seed_str) 
{
	if(locked) {
		return;
	}

	mut_seed.lock();

	for(auto cb = seed_change_cbs.begin(); cb != seed_change_cbs.end();) 
	{
		if(!((*cb)(seed_str))) {
			cb = seed_change_cbs.erase(cb);
		}
		else {
			cb++;
		}
	}

	delete current_str_seed;
	current_str_seed = new std::string(seed_str);
		
	{
		std::string special_check = seed_str;
		boost::algorithm::to_lower(special_check);
		boost::algorithm::trim(special_check);

		if(special_check == "~") {
			blank_seed();
			mut_seed.unlock();
			return;
		}
		else if(special_check == "mediumlunky") {
			crazy_seed(12, "mediumlunky");
			mut_seed.unlock();
			return;
		}
		else if(boost::algorithm::starts_with(special_check, "mediumlunky:")) {
			std::string seed = special_check.substr(12);
			crazy_seed(12, seed);
			mut_seed.unlock();
			return;
		}
		else if(special_check == "hardlunky") {
			crazy_seed(8, "hardlunky");
			mut_seed.unlock();
			return;
		}
		else if(boost::algorithm::starts_with(special_check, "hardlunky:")) {
			std::string seed = special_check.substr(10);
			crazy_seed(8, seed);
			mut_seed.unlock();
			return;
		}
		else if(special_check == "insanelunky") {
			crazy_seed(5, "insanelunky");
			mut_seed.unlock();
			return;
		}
		else if(boost::algorithm::starts_with(special_check, "insanelunky:")) {
			std::string seed = special_check.substr(12);
			crazy_seed(5, seed);
			mut_seed.unlock();
			return;
		}
		else if(special_check == "sashavol") {
			crazy_seed(2, "sashavol");
			mut_seed.unlock();
			return;
		}
		else if(boost::algorithm::starts_with(special_check, "sashavol:")) {
			std::string seed = special_check.substr(9);
			crazy_seed(2, seed);
			mut_seed.unlock();
			return;
		}
	}
		
	Seed the_seed = std::hash<std::string>()(seed_str);
	seed(std::hash<std::string>()(seed_str));

	mut_seed.unlock();
}