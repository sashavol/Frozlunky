#include "patches.h"
#include "game_hooks.h"
#include "rc.h"
#include "remote_call_patch.h"

struct RCRoundedMessage : public RemoteCallConstructor {
public:
	Address msg_addr;

private:
	std::shared_ptr<GameHooks> gh;
	bool is_valid;
	Address fn_rounded_msg;
	BYTE game_this_offs;

public:
	RCRoundedMessage(std::shared_ptr<GameHooks> gh);

	bool valid();
	void set_message(Address msg);
	virtual std::shared_ptr<RCData> make() override; 
};

struct MessageDisplayer {
private:
	std::shared_ptr<RCRoundedMessage> msg_constructor;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<Spelunky> spel;

public:
	MessageDisplayer(std::shared_ptr<GameHooks> gh);

	void display(const std::string& msg);
	bool valid();
};