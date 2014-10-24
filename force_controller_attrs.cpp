#include "force_controller_attrs.h"

unsigned type_ctrl_offset() {
	return 0x18;
}

void force_controller_type(std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp, int pid, int cont) 
{
	std::shared_ptr<Spelunky> spel = dp->spel;

	Address ctrl = gh->ctrl_object();
	spel->write_mem(ctrl + gh->ctrl_size()*pid + type_ctrl_offset(), &cont, sizeof(int));
}

int controller_type(std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp, int pid) {
	std::shared_ptr<Spelunky> spel = dp->spel;

	int type = -1;

	Address ctrl = gh->ctrl_object();
	spel->read_mem(ctrl + gh->ctrl_size()*pid + type_ctrl_offset(), &type, sizeof(int));

	return type;
}