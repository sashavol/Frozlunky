#pragma once

#include <functional>
#include <vector>

template <typename FnType>
struct CFnGen {
public:
	typedef std::function<FnType>* heap_fn;

private:
	std::vector<std::function<FnType>*> heap_fns;

public:
	CFnGen() {}
	
	std::function<FnType>* wrap(std::function<FnType> fn) {
		std::function<FnType>* ret(new std::function<FnType>(fn));
		heap_fns.push_back(ret);
		return ret;
	}

	~CFnGen() {
		for(std::function<FnType>* fn : heap_fns) {
			delete fn;
		}
		
		heap_fns.clear();
	}

	//copy
	CFnGen(const CFnGen& o) {
		for(std::function<FnType>* fn : o.heap_fns) {
			heap_fns.push_back(new std::function<FnType>(fn));
		}
	}

	//copy assign
	CFnGen& operator=(const CFnGen& o) {
		for(std::function<FnType>* fn : heap_fns) {
			delete fn;
		}
		heap_fns.clear();

		for(std::function<FnType>* fn : o.heap_fns) {
			heap_fns.push_back(new std::function<FnType>(fn));
		}
	}

	//move
	CFnGen(CFnGen&& o) {
		heap_fns = o.heap_fns;
		o.heap_fns.clear();
	}

	//move assign
	CFnGen& operator=(CFnGen&& o) {
		for(std::function<FnType>* fn : heap_fns) {
			delete fn;
		}

		heap_fns = o.heap_fns;
		o.heap_fns.clear();
	}
};