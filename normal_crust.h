#pragma once

#include "patches.h"
#include "second_rand.h"

class NormalCrustPatch : public Patch {
private:
	std::shared_ptr<SecondaryRandomPatch> srp;
	bool is_valid;

	virtual bool _perform() override {
		
	}
	
	virtual bool _undo() override {
		
	}

public:
	NormalCrustPatch(std::shared_ptr<SecondaryRandomPatch> srp) : Patch(srp->spel), srp(srp), is_valid(true)
	{
		if(!srp->is_active()) {
			srp->perform();
		}

		//TODO!
	}

	virtual bool valid() {
		return is_valid;
	}
};