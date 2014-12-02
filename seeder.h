#pragma once

#include "patches.h"
#include "derandom.h"
//#include "second_rand.h"
#include <boost/algorithm/string.hpp>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <functional>
#include <mutex>

class Seeder {
public:
	typedef size_t Seed;
	typedef std::function<bool(const std::string&)> seed_change_cb;

private:
	std::shared_ptr<DerandomizePatch> dp;
	std::vector<std::string> stack;
	std::vector<seed_change_cb> seed_change_cbs;
	std::mutex mut_seed;

	Seed current_seed;
	std::string* current_str_seed;
	bool locked;


	void crazy_seed(int crazy, const std::string& seed, int crazyval=0);

public:
	~Seeder() {
		delete current_str_seed;
	}

	Seeder(std::shared_ptr<DerandomizePatch> dp) : dp(dp), locked(false), current_str_seed(new std::string("")) {}

	std::string get_seed();

	bool lock();
	void unlock();
	bool is_locked();

	void on_seed_change(seed_change_cb cb);

	void push_seed();
	void pop_seed();

	void seed(Seed seed);
	void seed(const std::string& seed_str);
};