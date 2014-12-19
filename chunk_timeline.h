#pragma once

#include <vector>
#include <map>
#include "tile_chunk.h"
#include "entity_spawn_builder.h"

class ChunkTimeline {
private:
	typedef std::map<std::string, std::string> state;
	typedef std::vector<EntitySpawnBuilder::EntitySpawn> entity_state;

private:
	int p;
	
	std::shared_ptr<EntitySpawnBuilder> esb;
	std::vector<state> states;
	std::vector<entity_state> entity_states;
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
	void apply_state(state& state, entity_state& es) {
		for(auto&& u : state) {
			chunks[u.first]->set_data(u.second);
		}

		esb->clear();
		for(const EntitySpawnBuilder::EntitySpawn& e : es) {
			esb->add(e.x, e.y, e.entity);
		}
	}

public:
	ChunkTimeline(std::vector<Chunk*> chunks, std::shared_ptr<EntitySpawnBuilder> esb) : esb(esb), p(0) {
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
		
		if(esb) {
			entity_state est;
			for(auto&& es : *esb) {
				est.push_back(es.second);
			}
			entity_states.push_back(est);
		}
		
		p++;
		states.erase(states.begin()+p, states.end());
		entity_states.erase(entity_states.begin()+p, entity_states.end());
	}

	void rewind() {
		if(p > 0) {
			p--;
			apply_state(states[p], entity_states[p]);
		}
	}

	void forward() {
		if(p < (int)states.size() - 1) {
			p++;
			apply_state(states[p], entity_states[p]);
		}
	}
};