#pragma once

#include <vector>
#include <map>
#include "tile_chunk.h"
#include "entity_spawn_builder.h"

class ChunkTimeline {
private:
	typedef std::map<std::string, std::string> state;
	typedef std::vector<EntitySpawnBuilder::EntitySpawn> entity_state;
	typedef MessageGrid msg_state;

private:
	int p;
	
	std::shared_ptr<EntitySpawnBuilder> esb;
	std::vector<state> states;
	std::vector<entity_state> entity_states;
	std::vector<msg_state> message_states;
	std::map<std::string, SingleChunk*> chunks;
	MessageGrid* msg_grid;

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

	void apply_state(state& state, entity_state& es, MessageGrid& grid) {
		apply_state(state);

		if(esb) {
			esb->clear();
			for(const EntitySpawnBuilder::EntitySpawn& e : es) {
				esb->add(e.x_pos(), e.y_pos(), e.entity);
			}

			*msg_grid = grid;
		}
	}

public:
	ChunkTimeline(std::vector<Chunk*> chunks, std::shared_ptr<EntitySpawnBuilder> esb, MessageGrid& msg_grid) : 
		esb(esb), 
		p(0),
		msg_grid(&msg_grid)
	{
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
			message_states.push_back(*msg_grid);
		}
		
		p++;
		states.erase(states.begin()+p, states.end());
		
		if(esb) {
			entity_states.erase(entity_states.begin()+p, entity_states.end());
			message_states.erase(message_states.begin()+p, message_states.end());
		}
	}

	void rewind() {
		if(p > 0) {
			if(p == states.size()) {
				push_state();
				p--;
			}

			p--;

			if(esb)
				apply_state(states[p], entity_states[p], message_states[p]);
			else
				apply_state(states[p]);
		}
	}

	void forward() {
		if(p < (int)states.size() - 1) {
			p++;

			if(esb)
				apply_state(states[p], entity_states[p], message_states[p]);
			else
				apply_state(states[p]);
		}
	}
};