#pragma once

#include <vector>
#include <map>
#include "tile_chunk.h"

class ChunkTimeline {
private:
	typedef std::map<std::string, std::string> state;

private:
	int p;
	
	std::vector<state> states;
	std::map<std::string, SingleChunk*> chunks;

private:
	void load_chunk(Chunk* c) {
		if(c->type() == ChunkType::Single)
			chunks[c->get_name()] = static_cast<SingleChunk*>(c);
		else {
			for(Chunk* c : static_cast<GroupChunk*>(c)->get_chunks()) {
				load_chunk(c);
			}
		}
	}

private:
	void apply_state(state& state) {
		for(auto&& u : state) {
			chunks[u.first]->set_data(u.second);
		}	
	}

public:
	ChunkTimeline(std::vector<Chunk*> chunks) : p(0) {
		for(Chunk* c : chunks) {
			load_chunk(c);
		}
	}
	
	void push_state() {
		state st;
		for(auto&& cnk : chunks) {
			st[cnk.second->get_name()] = cnk.second->get_data();
		}
		states.push_back(st);
		p++;
		states.erase(states.begin()+p, states.end());
	}

	void rewind() {
		if(p > 0) {
			p--;
			apply_state(states[p]);
		}
	}

	void forward() {
		if(p < (int)states.size() - 1) {
			p++;
			apply_state(states[p]);
		}
	}
};