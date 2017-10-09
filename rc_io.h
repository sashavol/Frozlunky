#pragma once

#include "rc.h"
#include "spelunky.h"
#include "derandom.h"

class RCLoad : public RemoteCallConstructor {
private:
	bool is_valid;
	Address fn_load;
	std::shared_ptr<DerandomizePatch> dp;

public:
	RCLoad(std::shared_ptr<DerandomizePatch> dp);

	virtual std::shared_ptr<RCData> make() override;
};

class RCSave : public RemoteCallConstructor {
private:
	bool is_valid;
	Address fn_save;
	std::shared_ptr<DerandomizePatch> dp;

public:
	RCSave(std::shared_ptr<DerandomizePatch> dp);
	
	virtual std::shared_ptr<RCData> make() override;
};

class RCReset : public RemoteCallConstructor {
private:
	bool is_valid;
	Address fn_reset;
	std::shared_ptr<DerandomizePatch> dp;

public:
	RCReset(std::shared_ptr<DerandomizePatch> dp);

	virtual std::shared_ptr<RCData> make() override;
};