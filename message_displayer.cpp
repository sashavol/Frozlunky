#include <locale>
#include <codecvt>

#include "gui.h"
#include "message_displayer.h"

//+2
static BYTE this_offs_find[] = {0x8B,0xCC,0xCC,0xCC,0xBA,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B};
static std::string this_offs_mask = "x...x....x....x.....x";

//+0
static BYTE rounded_msg_find[] = {0xA1,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0x33,0xCC,0x3B,0xCC,0x74,0xCC,0x83,0xCC,0xCC,0x74};
static std::string rounded_msg_mask = "x....x..x.x.x.x..x";

// AA (+1)  -> game_ptr
// BB (+7)  -> game_this_offs
// CC (+9) -> msg_addr
static BYTE msg_ctx_template[] = {
	0xA1, 0xAA,0xAA,0xAA,0xAA,
	0x8B, 0x48, 0xBB,
	0x68, 0xCC,0xCC,0xCC,0xCC
};

RCRoundedMessage::RCRoundedMessage(std::shared_ptr<GameHooks> gh) : 
	gh(gh),
	is_valid(true),
	fn_rounded_msg(0),
	game_this_offs(0)
{
	fn_rounded_msg = gh->spel->find_mem(rounded_msg_find, rounded_msg_mask);
	if(!fn_rounded_msg) {
		DBG_EXPR(std::cout << "[RCRoundedMessage] Failed to find fn_rounded_msg" << std::endl);
		is_valid = false;
		return;
	}

	DBG_EXPR(std::cout << "[RCRoundedMessage] fn_rounded_msg = " << fn_rounded_msg << std::endl);

	Address this_offs_cont = gh->spel->find_mem(this_offs_find, this_offs_mask);
	if(!this_offs_cont) {
		DBG_EXPR(std::cout << "[RCRoundedMessage] Failed to find this_offs_cont" << std::endl);
		is_valid = false;
		return;
	}
	gh->spel->read_mem(this_offs_cont+2, &game_this_offs, sizeof(BYTE));

	DBG_EXPR(std::cout << "[RCRoundedMessage] game_this_offs = " << (int)game_this_offs << std::endl);
}

void RCRoundedMessage::set_message(Address msg) {
	this->msg_addr = msg;
}

bool RCRoundedMessage::valid() {
	return is_valid;
}

std::shared_ptr<RemoteCallConstructor::RCData> RCRoundedMessage::make() {
	if(!valid()) {
		return nullptr;
	}

	BYTE* ctx = new BYTE[sizeof(msg_ctx_template)];
	std::memcpy(ctx, msg_ctx_template, sizeof(msg_ctx_template));

	Address game_ptr = gh->dp->game_ptr();
	std::memcpy(ctx + 1, &game_ptr, sizeof(Address));
	ctx[7] = game_this_offs;
	std::memcpy(ctx + 9, &msg_addr, sizeof(Address));
	
	return std::make_shared<RCData>(fn_rounded_msg, ctx, sizeof(msg_ctx_template));
}


////////////////
// MessageDisplayer
////////////////

MessageDisplayer::MessageDisplayer(std::shared_ptr<GameHooks> gh) : 
	gh(gh),
	spel(gh->spel)
{
	msg_constructor = std::make_shared<RCRoundedMessage>(gh);
	if(!msg_constructor->valid()) {
		DBG_EXPR(std::cout << "[MessageDisplayer] Warning: RCRoundedMessage is invalid." << std::endl);
	}
}

void MessageDisplayer::display(const std::string& msg) {
	if(!msg_constructor->valid() || msg.empty()) {
		return;
	}

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wmsg = converter.from_bytes(msg);

	Address alloc = spel->allocate(sizeof(wchar_t)*wmsg.size() + 1);
	spel->write_mem(alloc, wmsg.c_str(), sizeof(wchar_t)*wmsg.size()+1);
	msg_constructor->set_message(alloc);

	std::shared_ptr<RemoteCallConstructor::RCData> rc = msg_constructor->make();
	if(!rc) {
		DBG_EXPR(std::cout << "[MessageDisplayer] Failed to construct message RC." << std::endl);
		return;
	}

	if(!CurrentRCP()->enqueue_call(rc, [=]() {
		spel->free(alloc);
	}))
	{
		DBG_EXPR(std::cout << "[MessageDisplayer] Failed to enqueue call." << std::endl);
	}
}

bool MessageDisplayer::valid() {
	return msg_constructor->valid();
}