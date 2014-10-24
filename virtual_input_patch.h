#include "patches.h"
#include "derandom.h"

#include "input_recv_patch.h"

#include <memory>

/*
	Patches the game to only use virtual inputs. 
	Game cycles are halted if there are no inputs remaining in the input buffer spelunky-side.

	Data is maintained in a large cyclic buffer with infinite indices, real-index := idx % buffer-size
*/
class VirtualInputPatch : public Patch {
private:
	int buf_pos;

public:
	VirtualInputPatch(std::shared_ptr<DerandomizePatch> dp) : Patch(dp->spel), buf_pos(0) {
		//TODO
	}

	virtual bool valid() override;
	void push_inputs(const std::vector<InputFrame>& ifs);

private:
	virtual bool _perform() override;
	virtual bool _undo() override;
};