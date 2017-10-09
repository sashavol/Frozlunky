#pragma once

#include <string>
#include <vector>

namespace Frozboards {
	class Unscrambler {
	public:
		typedef std::vector<unsigned char> membuf;

	private:
		membuf scrambled;
		unsigned id;

	public:
		Unscrambler(const membuf& scrambled);
		~Unscrambler();

		void cycle_unscramble();
		const membuf& data();
	};
}