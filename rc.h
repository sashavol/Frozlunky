#pragma once

#include "winheaders.h"
#include "spelunky.h"

#include <memory>

/*
	Abstract definition of a constructor for synchronous remote calls via RemoteCallPatch
*/
class RemoteCallConstructor {
public:
	struct RCData {
		Address fn;
		const BYTE* ctx;
		unsigned ctx_len;

		RCData(Address fn, const BYTE* ctx, unsigned ctx_len) :
			fn(fn), ctx(ctx), ctx_len(ctx_len) {}

		~RCData() {
			delete[] ctx;
			ctx = nullptr;
		}
	};

	virtual std::shared_ptr<RCData> make() = 0;
};