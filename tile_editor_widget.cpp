#include "tile_editor_widget.h"
#include "tile_description.h"

//OPT arrow traps?

//TODO checkpoint system
//TODO level range selection

//TODO documentation
//TODO fix scrollbars not reaching end on higher resizes

#define TILE_FONT_SIZE 11


bool EditorWidget::allow_input() {
	if(!key_press)
		return true;

	return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - *key_press).count() > 20;
}

void EditorWidget::status(unsigned state) {
	if(status_cb)
		status_cb(state);
}

std::shared_ptr<EntitySpawnBuilder> EditorWidget::get_entity_builder() {
	return esb;
}

std::vector<Chunk*> EditorWidget::get_chunks() {
	return chunks;
}

ChunkCursor& EditorWidget::get_cursor() {
	return cursor;
}

TilePicker& EditorWidget::get_picker() {
	return picker;
}

void EditorWidget::status_callback(std::function<void(unsigned)> cb) {
	this->status_cb = cb;
}

void EditorWidget::clear_state() {
	timeline = ChunkTimeline(chunks, esb);
}

void EditorWidget::clear_chunk(Chunk* cnk) {
	if(read_only)
		return;

	if(cnk->type() == ChunkType::Single) {
		SingleChunk* sc = static_cast<SingleChunk*>(cnk);
		std::string d = sc->get_data();
		std::fill(d.begin(), d.end(), '0');
		sc->set_data(d);
	}
	else {
		for(Chunk* c : static_cast<GroupChunk*>(cnk)->get_chunks()) {
			clear_chunk(c);
		}
	}
}

void EditorWidget::clear_chunks() {
	if(read_only)
		return;

	timeline.push_state();

	for(Chunk* c : chunks) {
		clear_chunk(c);
	}

	if(esb) {
		esb->clear();
	}

	update_hint_bar();
	status(STATE_CHUNK_WRITE);
}

void EditorWidget::shift_env_left(int u, bool box_change) {
	if(!box_change)
		cursor.try_dx(-u);
	else
		cursor.try_dex(-u);
		
	last_dir = Direction::LEFT;
}

void EditorWidget::shift_env_right(int u, bool box_change) {
	if(!box_change)
		cursor.try_dx(u);
	else
		cursor.try_dex(u);
		
	last_dir = Direction::RIGHT;
}

void EditorWidget::shift_env_up(int u, bool box_change) {
	if(!box_change)
		cursor.try_dy(-u);
	else
		cursor.try_dey(-u);

	last_dir = Direction::UP;
}

void EditorWidget::shift_env_down(int u, bool box_change) {
	if(!box_change)
		cursor.try_dy(u);
	else
		cursor.try_dey(u);

	last_dir = Direction::DOWN;
}

void EditorWidget::shift_env_last(int u) {
	switch(last_dir){
	case Direction::UP:
		shift_env_up(u);
		break;
	case Direction::DOWN:
		shift_env_down(u);
		break;
	case Direction::LEFT:
		shift_env_left(u);
		break;
	default:
	case Direction::RIGHT:
		shift_env_right(u);
		break;
	}
}

void EditorWidget::shift_picker_cursor(int dx, int dy) {
	picker.try_move(dx, dy);
	if(picker.tile()) {
		hint_bar->set_tile(picker.tile(), arm, nullptr);
		hint_bar->redraw();
	}
	else if(picker.entity()) {
		hint_bar->set_entity(picker.entity(), nullptr);
		hint_bar->redraw();
	}
}

int EditorWidget::mouse_event_id() {
	switch(Fl::event_state()) {
	case FL_BUTTON1:
		return 0;
	case FL_BUTTON2:
		return 1;
	case FL_BUTTON3:
		return 2;
	default:
		return 0;
	}
}

void EditorWidget::cursor_move(int rx, int ry, bool drag) {
	if(rx < 0 || ry < 0 || rx >=  x()+w() || ry >= y()+h())
		return;

	Chunk* c = find_chunk(rx, ry);

	if(c != nullptr) {
		auto ccpos = chunkcoord_pos(rx, ry);

		if(!drag) {
			move_drag_start = std::make_pair(ccpos.first, ccpos.second);

			cursor.s(ccpos.first, ccpos.second);
			cursor.e(ccpos.first, ccpos.second);
		}
		else if(move_drag_start.first > -1) {
			int ex = ccpos.first, ey = ccpos.second;
			int sx = move_drag_start.first, sy = move_drag_start.second;

			cursor.s(sx, sy);
			cursor.e(ex, ey);
		}

		update_hint_bar();
	}
	else {
		move_drag_start = std::make_pair(-1, -1);
		
		int entity = picker.entity(rx, ry);
		if(entity != 0) {
			hint_bar->set_entity(entity, nullptr);
			picker.entity_select(entity);
		}
		else {
			char ptile = picker.tile(rx, ry);
			if(ptile != 0) {
				hint_bar->set_tile(ptile, arm, nullptr);
				picker.select(ptile);
			}
		}
	}

	parent()->redraw();
}

void EditorWidget::cursor_fill(int x, int y) {
	char tile = picker.tile();
	int entity = picker.entity();

	if(x >= 0 && y >= 0 && (tile || entity)) {
		cursor.s(x, y);
		cursor.e(x, y);
		timeline.push_state();
		
		if(tile)
			cursor.fill(tile);
		else
			cursor.entity_fill(entity);

		status(STATE_CHUNK_PASTE);
		parent()->redraw();
	}
}

static cursor_store clipboard;

int EditorWidget::handle_key(int key) {
	cursor_finish_move();

	switch(key) {
	case 65289: //tab
		if(shift_down)
			status(STATE_REQ_TAB_REVERSE);
		else
			status(STATE_REQ_TAB);

		return 1;

	case 65307: //esc: reset cursor size
		if(!shift_down) {
			int sx = cursor.rsx(), sy = cursor.rsy();
			cursor.s(sx, sy);
			cursor.e(sx, sy);
			parent()->redraw();
		}
		else {
			int ex = cursor.rex(), ey = cursor.rey();
			cursor.s(ex, ey);
			cursor.e(ex, ey);
			parent()->redraw();
		}
		return 1;

	case 65361: //left
		if(alt_down) {
			shift_picker_cursor(-1, 0);
			parent()->redraw();
			return 1;
		}
		shift_env_left(ctrl_down ? 2 : 1, shift_down);
		parent()->redraw();
		return 1;

	case 65362: //up
		if(alt_down) {
			shift_picker_cursor(0, -1);
			parent()->redraw();
			return 1;
		}
		shift_env_up(ctrl_down ? 2 : 1, shift_down);
		parent()->redraw();
		return 1;

	case 65363: //right
		if(alt_down) {
			shift_picker_cursor(1, 0);
			parent()->redraw();
			return 1;
		}
		shift_env_right(ctrl_down ? 2 : 1, shift_down);
		parent()->redraw();
		return 1;

	case 65364: //down
		if(alt_down) {
			shift_picker_cursor(0, 1);
			parent()->redraw();
			return 1;
		}
		shift_env_down(ctrl_down ? 2 : 1, shift_down);
		parent()->redraw();
		return 1;

	case 32:    //space
		if(cursor.in_bounds()) {
			if(ctrl_down) {
				int entity = cursor.entity_get();
				char tile = cursor.get();
				if(entity)
					picker.entity_select(entity);
				else if(tile)
					picker.select(tile);
				parent()->redraw();
			}
			else {
				timeline.push_state();
				cursor.put('0');
				shift_env_right(1);
				parent()->redraw();
				update_hint_bar();
				status(STATE_CHUNK_WRITE);
			}
		}
		return 1;

	case 65288: //backspace
		if(cursor.in_bounds()) {
			timeline.push_state();
			cursor.put('0');
			if(!ctrl_down)
				shift_env_left(1);
			parent()->redraw();
			update_hint_bar();
			status(STATE_CHUNK_WRITE);
		}
		return 1;

	case 65535: //delete
		if(cursor.in_bounds()) {
			timeline.push_state();
			cursor.put('0');
			parent()->redraw();
			update_hint_bar();
			status(STATE_CHUNK_WRITE);
		}
		return 1;

	case 65505: //shift
		shift_down = true;
		return 1;

	case 65507: //ctrl
	case 65508:
		ctrl_down = true;
		return 1;

	case 65513: //alt
	case 65514:
		alt_down = true;
		return 1;

	case '`': //`: place down currently picked tile
		if(!shift_down) {
			if(alt_down) { //do nothing if alt is down for intuition purposes (alt + tile -> picker, alt + picker.tile() -> picker results in no operation)
				return 1;
			}
			
			timeline.push_state();

			if(picker.tile())
				cursor.put(picker.tile());
			else if(picker.entity())
				cursor.entity_put(picker.entity());
			else
				cursor.put('0');
			
			shift_env_right(1);
			status(STATE_CHUNK_WRITE);
			update_hint_bar();

			parent()->redraw();
			return 1;
		}

	case 0xFFBE: //F1
		if(ctrl_down) {
			status(STATE_REQ_RESIZE_1);
			return 1;
		}
	
	case 0xFFBF: //F2
		if(ctrl_down) {
			status(STATE_REQ_RESIZE_1_5);
			return 1;
		}
	
	case 0xFFC0: //F3
		if(ctrl_down) {
			status(STATE_REQ_RESIZE_2);
			return 1;
		}

	case 0xFFC1: //F4
		if(ctrl_down) {
			status(STATE_REQ_RESIZE_3);
			return 1;
		}

	case 0xFFC2: //F5
		if(ctrl_down) {
			status(STATE_REQ_RESIZE_4);
			return 1;
		}

	case 102: //ctrl+f: fill at cursor
		if(ctrl_down) {
			cursor_fill(cursor.rsx(), cursor.rsy());
			status(STATE_CHUNK_WRITE);
			update_hint_bar();
			return 1;
		}

	case 97: //ctrl+a: select all
		if(ctrl_down) {
			cursor.s(0, 0);
			cursor.e(cursor.cc_width() - 1, cursor.cc_height() - 1);
			parent()->redraw();
			return 1;
		}

	case 115: //ctrl+s: save + apply
		if(ctrl_down) {
			tp->apply_chunks();
			
			if(shift_down)
				status(STATE_REQ_SAVE_AS);
			else
				status(STATE_REQ_SAVE);

			return 1;
		}

	case 122: //ctrl+z: undo
		if(ctrl_down) {
			if(!read_only) {
				timeline.rewind();
				parent()->redraw();
				status(STATE_CHUNK_PASTE);
				update_hint_bar();
			}
			return 1;
		}
					
	case 121: //ctrl+y: redo
		if(ctrl_down) {
			if(!read_only) {
				timeline.forward();
				parent()->redraw();
				status(STATE_CHUNK_WRITE);
				update_hint_bar();
			}
			return 1;
		}

	case 99: //ctrl+c: copy
		if(ctrl_down) {
			if(cursor.in_bounds()) {
				clipboard = cursor.encode();
				status(STATE_CHUNK_COPY);
			}
			return 1;
		}
			
	case 120: //ctrl+x: cut
		if(ctrl_down) {
			timeline.push_state();
			clipboard = cursor.encode();
			cursor.put('0');
			status(STATE_CHUNK_WRITE);
			update_hint_bar();
			parent()->redraw();
			return 1;
		}

	case 118: //ctrl+v: paste
		if(ctrl_down) {
			if(cursor.in_bounds() && !clipboard.empty()) {
				timeline.push_state();
				cursor.decode(clipboard);
				status(STATE_CHUNK_PASTE);
				update_hint_bar();
				parent()->redraw();
			}
			return 1;
		}

	case 'o': //ctrl+o: open level pack
		if(ctrl_down) {
			status(STATE_REQ_OPEN);
			return 1;
		}

	case 'r': //ctrl+shift+r: open resource editor
		if(ctrl_down && shift_down) {
			status(STATE_REQ_RESOURCE_EDITOR);
			return 1;
		}

	case 'p': //ctrl+p: pick entity, ctrl+shift+p: open project settings
		if(ctrl_down) {
			if(shift_down)
				status(STATE_REQ_LEVEL_SETTINGS);
			else
				status(STATE_REQ_ENTITY_PICKER);
			return 1;
		}

	case 'd': //ctrl+d: swap to default viewer
		if(ctrl_down) {
			status(STATE_REQ_DEFAULT_SWAP);
			return 1;
		}

	case 'n': //ctrl+n: clear level, ctrl+shift+n: new level pack
		if(ctrl_down) {
			if(shift_down) {
				status(STATE_REQ_NEW_FILE);
			}
			else if(!read_only) {
				clear_chunks();
				parent()->redraw();
			}
			return 1;
		}

	case 'e': //ctrl+e: force level toggle
		if(ctrl_down) {
			status(STATE_REQ_TOGGLE_FORCE_LEVEL);
			return 1;
		}

	default: //*: place down typed character as tile, alt+*: pick typed character tile
		{
			char tile = Fl::event_text()[0];
			if(tp->valid_tile(tile)) {
				//do not put down tile if alt. Alt+Tile = pick tile only
				if(!alt_down) {
					if(cursor.in_bounds()) {
						timeline.push_state();
						cursor.put(tile);
						shift_env_right(1);
						status(STATE_CHUNK_WRITE);
					}
				}
				else {
					picker.select(tile);
					hint_bar->set_tile(tile, arm, nullptr);
				}

				parent()->redraw();
			}
			return 1;
		}
	}

	throw std::invalid_argument("No input triggered.");
}

void EditorWidget::cursor_finish_move() {
	move_drag_start = std::pair<int, int>(-1, -1);
}

void EditorWidget::cursor_build(int rx, int ry, bool drag, char default_tile) {
	char tile = default_tile ? default_tile : picker.tile();
	int entity = default_tile ? 0 : picker.entity();

	if(tile == 0 && entity == 0)
		return;

	auto affect = [=](int x, int y) {
		auto cc = chunkcoord_pos(x, y);
		cursor.s(cc.first, cc.second);
		cursor.e(min(cursor.cc_width()-1, cc.first+build_dim.first-1), min(cursor.cc_height()-1, cc.second+build_dim.second-1));

		if(entity)
			cursor.entity_put(entity);
		else if(tile)
			cursor.put(tile);
		
		status(STATE_CHUNK_WRITE);
	};

	if(drag) {
		double x = last_build.first, y = last_build.second;
		int to_x = rx, to_y = ry;

		double vx = to_x - x, vy = to_y - y;
		double len = sqrt(vx*vx + vy*vy);
		if(len >= 1) {
			vx /= len; vy /= len;

			while(abs(to_x - x) > 0.1 || abs(to_y - y) > 0.1) {
				affect((int)x, (int)y);
				
				vx = to_x - x, vy = to_y - y;
				len = sqrt(vx*vx + vy*vy);
				if(len > 0.9) {
					vx /= len;
					vy /= len;
				}

				x += vx;
				y += vy;
			}
		}
	}
	else {
		build_dim = std::make_pair(cursor.rex() - cursor.rsx() + 1, cursor.rey() - cursor.rsy() + 1);
		affect(rx, ry);
	}

	last_build = std::make_pair(rx, ry);
	parent()->redraw();
}

int EditorWidget::handle(int evt) {
	switch(evt) {
	case FL_FOCUS:
		return 1;
	case FL_UNFOCUS:
		/*cursor.sx = -1;
		cursor.sy = -1;
		cursor.ex = -1;
		cursor.ey = -1;
		parent()->redraw();*/
		return 1;

	case FL_PUSH: //mouse pressed
		this->take_focus();
		mouse_down[mouse_event_id()] = true;
			
		if(Fl::event_state() & FL_BUTTON1)
			cursor_move(Fl::event_x(), Fl::event_y(), false);
		else if(Fl::event_state() & FL_BUTTON3) {
			auto cpos = chunkcoord_pos(Fl::event_x(), Fl::event_y());
			if(cpos.first >= 0 && cpos.second >= 0) {
				timeline.push_state();
				cursor_build(Fl::event_x(), Fl::event_y(), false);
			}
		}
		else if(Fl::event_state() & FL_BUTTON2) {
			auto cpos = chunkcoord_pos(Fl::event_x(), Fl::event_y());
			if(cpos.first >= 0 && cpos.second >= 0) {
				timeline.push_state();
				cursor_build(Fl::event_x(), Fl::event_y(), false, '0');
			}
		}

		return 1;

	case FL_RELEASE: //mouse btn released
		mouse_down[mouse_event_id()] = false;
		if(Fl::event_state() & FL_BUTTON1) {
			cursor_finish_move();
		}
		return 1;

	case FL_DRAG:
		if(Fl::event_state() & FL_BUTTON1)
			cursor_move(Fl::event_x(), Fl::event_y(), true);
		else if(Fl::event_state() & FL_BUTTON3) {
			auto cpos = chunkcoord_pos(Fl::event_x(), Fl::event_y());
			if(cpos.first >= 0 && cpos.second >= 0) {
				cursor_build(Fl::event_x(), Fl::event_y(), true);
			}
		}
		else if(Fl::event_state() & FL_BUTTON2) {
			auto cpos = chunkcoord_pos(Fl::event_x(), Fl::event_y());
			if(cpos.first >= 0 && cpos.second >= 0) {
				cursor_build(Fl::event_x(), Fl::event_y(), true, '0');
			}
		}

		return 1;

	case 0xC: //key typed
		{
			if(Fl::focus() != this)
				return 0;

			int key = Fl::event_key();

			if(!allow_input())
				break;

			//key_pressed only if not flag key
			if(key != 65505 && key != 65507 && key != 65508 && key != 65513 && key != 65514) { 
				key_press = std::make_shared<clock::time_point>(clock::now());
			}

			//handle_key throws invalid argument if key operation does nothing.
			try {
				return handle_key(key);
			}
			catch(std::invalid_argument&) {}
		}
		//key pressed
		return 1;

	case FL_KEYUP:
		key_press = nullptr;

		switch(Fl::event_key()) {
		case 65505:
			shift_down = false;
			return 1;
		case 65507:
		case 65508:
			ctrl_down = false;
			return 1;
		case 65513:
		case 65514:
			alt_down = false;
			return 1;
		}
		//key released
		return 1;
	}

	return Fl_Widget::handle(evt);
}


template <typename Numeric_>
static Numeric_ clamp_(Numeric_ s, Numeric_ e, Numeric_ v) {
	return min(e, max(s, v));
}

void EditorWidget::update_hint_bar() {
	auto pair = render_pos(cursor.rsx(), cursor.rsy());

	char tile = cursor.get();
	int entity = cursor.entity_get();
	
	if(entity)
		hint_bar->set_entity(entity, find_chunk(pair.first, pair.second));
	else
		hint_bar->set_tile(tile, arm, find_chunk(pair.first, pair.second));
	
	hint_bar->redraw();
}

EditorWidget::~EditorWidget() {}

#define PICKER_X_ES_OFFS 23
#define PICKER_WIDTH 45
#define PICKER_XU 15
#define PICKER_YU 15

EditorWidget::EditorWidget(AreaRenderMode arm, 
						   std::shared_ptr<StaticChunkPatch> tp, 
						   std::shared_ptr<EntitySpawnBuilder> esb,
						   int x, int y, int w, int h, 
						   Fl_Scrollbar* scrollbar, 
						   TileEditingHintbar* hint_bar,
						   std::vector<Chunk*> chunks, 
						   bool extended_mode, bool read_only) : 
	Fl_Widget(x,y,w,h,""),
	read_only(read_only),
	chunks(chunks),
	esb(esb),
	sidebar_scrollbar(scrollbar),
	hint_bar(hint_bar),
	cnk_render_w(130),
	cnk_render_h(104),
	xu(0),
	yu(0),
	last_w(0),
	last_h(0),
	ctrl_down(false),
	shift_down(false),
	alt_down(false),
	tp(tp),
	extended_mode(extended_mode),
	last_dir(Direction::UP),
	timeline(chunks, esb),
	move_drag_start(-1, -1),
	arm(arm),
	picker(this, arm, tp->valid_tiles(), scrollbar->x() + PICKER_X_ES_OFFS, y, PICKER_WIDTH, h, PICKER_XU, PICKER_YU),
	cursor(chunks, esb, extended_mode ? 2 : 4, read_only)
{
	std::fill(mouse_down, mouse_down+sizeof(mouse_down), false);

	if(chunks.empty()) {
		this->deactivate();
		return;
	}

	ensure_size();

	//default picker to air
	picker.select('0');

	int vtiles = (get_chunk_render_pos(*chunks.rbegin()).second + yu*CHUNK_HEIGHT - this->y())/yu;
	int screen_vtiles = this->h() / yu;

	if(screen_vtiles >= vtiles) {
		sidebar_scrollbar->deactivate();
		sidebar_scrollbar->slider_size(1.0);
		use_scrollbar = false;
	}
	else { 
		use_scrollbar = true;
		sidebar_scrollbar->activate();
		sidebar_scrollbar->slider_size(this->h() / (double)(get_chunk_render_pos(*chunks.rbegin()).second + yu*CHUNK_HEIGHT - this->y()));
		sidebar_scrollbar->type(FL_VERTICAL);
		sidebar_scrollbar->bounds(0, vtiles - CHUNK_HEIGHT);
		sidebar_scrollbar->step(1);

		sidebar_scrollbar->callback([](Fl_Widget* wid) {
			wid->redraw();
		});
	}

	//adjust scrollbar if cursor off screen
	cursor.pos_callback([=](int dsx, int dsy, int dex, int dey) {
		update_hint_bar();
		
		auto affect = [=](int amt) {
			int t = sidebar_scrollbar->value() + amt;
			sidebar_scrollbar->value(clamp_((int)sidebar_scrollbar->minimum(), (int)sidebar_scrollbar->maximum(), t));
		};

		if(render_pos(cursor.rex(), cursor.rey()).second >= this->y() + cnk_render_h*4) {
			std::pair<int, int> bottom = chunkcoord_pos(this->x(), this->y() + this->h() - 1);
			affect(cursor.rey() - bottom.second + CHUNK_HEIGHT);
		}
		else if(render_pos(cursor.rsx(), cursor.rsy()).second < this->y()) {
			std::pair<int, int> zero = chunkcoord_pos(this->x(), this->y());
			affect(cursor.rsy() - zero.second + 1 - CHUNK_HEIGHT);
		}
	});

	update_hint_bar();
}

Chunk* EditorWidget::find_chunk(int rx, int ry) {
	int w = this->w();
	int x = this->x(), y = this->y();
	int hc = 0;

	ry += sidebar_scrollbar->value()*yu;

	for(Chunk* c : chunks) {
		if((rx >= x && rx < x + cnk_render_w)
		&& (ry >= y && ry < y + cnk_render_h))
		{
			return c;
		}

		hc++;
		x += cnk_render_w;
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h;
			hc = 0;
		}
	}

	return nullptr;
}

std::pair<int, int> EditorWidget::chunkcoord_pos(Chunk* cnk) {
	int xc = 0, yc = 0;
	int hc = 0;

	for(Chunk* c : chunks) {
		if(c == cnk)
			return std::pair<int, int>(xc, yc);

		hc++;
		xc += c->get_width();
		if(hc == 4 || (extended_mode && hc == 2)) {
			yc += c->get_height();
			xc = 0;
			hc = 0;
		}
	}

	return std::pair<int, int>(-1, -1);
}

std::pair<int, int> EditorWidget::chunkcoord_pos(int rx, int ry) {
	Chunk* base = find_chunk(rx, ry);
	if(base) {
		auto rpos = get_chunk_render_pos(base);
		auto cpos = chunkcoord_pos(base);
		return std::pair<int, int>((rx - rpos.first)/xu + cpos.first, (ry - rpos.second)/yu + cpos.second);
	}
	else {
		DBG_EXPR(std::cout << "[EditorWidget] Warning: Failed to find chunk for r(" << std::setbase(10) << rx << ", " << ry << std::setbase(16) << ")" << std::endl);
	}
	return std::pair<int, int>(-1, -1);
}

std::pair<int, int> EditorWidget::render_pos(int tx, int ty) {
	int w = this->w();
	int x = this->x(), y = this->y();
	int xc = 0, yc = 0;
	int hc = 0;

	y -= sidebar_scrollbar->value()*yu;
	
	for(Chunk* c : chunks) {
		if((tx >= xc && tx < xc + c->get_width())
		&& (ty >= yc && ty < yc + c->get_height()))
		{
			return std::pair<int, int>((tx - xc)*xu + x, (ty - yc)*yu + y);
		}

		hc++;
		x += cnk_render_w;
		xc += c->get_width();
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h;
			yc += c->get_height();
			xc = 0;
			hc = 0;
		}
	}

	return std::pair<int, int>(-1, -1);
}

std::pair<int, int> EditorWidget::get_chunk_render_pos(Chunk* cnk) {
	int w = this->w();
	int x = this->x();
	int y = this->y();

	int hc = 0;

	for(Chunk* c : chunks) {
		if(c == cnk)
			return std::pair<int, int>(x, y - sidebar_scrollbar->value()*yu);

		hc++;
		x += cnk_render_w;
		if(hc == 4 || (extended_mode && hc == 2)) { //TODO this is not very accurate, fix this check.
			x = this->x();
			y += cnk_render_h;
			hc = 0;
		}
	}

	throw std::runtime_error("Chunk from non-native editor.");
}



void EditorWidget::render_chunk(Chunk* cnk, int px, int py, int maxw, int maxh) {
	int ch = cnk->get_height(), cw = cnk->get_width();
	
	int xu = maxw / cw, yu = maxh / ch;
	int ymin = this->y(), ymax = this->y() + cnk_render_h*4 - 1;

	//maps chunk coordinates to world coordinates
	auto map = [&](int cx, int cy) -> std::pair<int, int> {
		return std::pair<int, int>(px + cx*maxw / cw, py + cy*maxh / ch); 
	};

	
	//chunk bg + bounds
	auto fs = map(0, 0), fx = map(cw, ch);
	fx.first -= 1; fx.second -= 1;

	fs.second = clamp_(ymin, ymax, fs.second);
	fx.second = clamp_(ymin, ymax, fx.second);

	auto render_bounds = [=]() {
		fl_color(Fl_Color(0x1A1A1A00));
		fl_line(fs.first, fs.second, fs.first, fx.second);
		fl_line(fs.first, fs.second, fx.first, fs.second);
	};
	fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, fs.first, fs.second, fx.first - fs.first + 1, fx.second - fs.second + 1, Fl_Color(0));

	//render bounds behind scene if not in read-only mode
	if(!read_only) {
		render_bounds();
	}
	
	fl_font(FL_SCREEN, TILE_FONT_SIZE);
	for(int cy = 0; cy < ch; cy++) {
		for(int cx = 0; cx < cw; cx++) {
			char tile = cnk->tile(cx, cy);
			//'0' = air
			if(tile != '0') { 
				auto dp = map(cx, cy);
				if(dp.second >= ymin && dp.second+yu < ymax) {
					draw_tile(tile, dp.first, dp.second, xu, yu, arm);
				}
				else if(dp.second + yu >= ymin || dp.second < ymax) {
					std::pair<int,int> start = dp, end = std::make_pair(dp.first+xu-1, dp.second+yu-1);
					start.second = clamp_(ymin, ymax, dp.second);
					end.second = clamp_(ymin, ymax, end.second);
					if(end.second - start.second > 5) {
						draw_tile(tile, start.first, start.second, end.first - start.first + 1, end.second - start.second + 1); 
					}
				}
			}
		}
	}

	//render bounds in front of scene for chunk clarity in read-only mode
	if(read_only) {
		render_bounds();
	}

	picker.draw();
}

void EditorWidget::render_cursor() {
	int sx = cursor.rsx(), sy = cursor.rsy(), ex = cursor.rex(), ey = cursor.rey();
	std::pair<int, int> s = render_pos(sx, sy), e = render_pos(ex, ey);
	
	fl_rect(s.first, s.second, e.first - s.first + xu, e.second - s.second + yu, 0xFF000000);
}

void EditorWidget::ensure_size() {
	if(last_w != w() || last_h != h()) {
		yu = h() / (4*CHUNK_HEIGHT);
		xu = yu;

		cnk_render_w = xu*CHUNK_WIDTH;
		cnk_render_h = yu*CHUNK_HEIGHT;

		int picker_w = (x()+w() - 5) - (sidebar_scrollbar->x() + sidebar_scrollbar->w() + 5) + 3;
		int picker_x = sidebar_scrollbar->x() + sidebar_scrollbar->w() + 5;
		picker.resize(picker_x, y(), picker_w, h(), picker_w/4, picker_w/4);
		
		last_w = w();
		last_h = h();
	}
}

void EditorWidget::draw() {
	ensure_size();
	
	int x = this->x(), y = this->y();
	int hc = 0;

	//render regular tiles
	for(Chunk* c : chunks) {
		int rx = x, ry = y - sidebar_scrollbar->value()*yu;

		if(ry >= this->y() - cnk_render_h && ry < this->h() + cnk_render_h) {
			render_chunk(c, rx, ry, cnk_render_w, cnk_render_h);
		}
			
		hc++;
		x += cnk_render_w;
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h;
			hc = 0;
		}
	}
	
	if(esb) {
		//TODO this rendering operation is very slow because of render_pos call, improve performance of either render_pos or this
		//render entity spawners
		for(auto&& es : *esb) {
			const EntitySpawnBuilder::EntitySpawn& eto = es.second;
			std::pair<int, int> unmapped = EntitySpawnLayer::unmap(eto.x, eto.y);
			auto rpos = render_pos(unmapped.first, unmapped.second);
			draw_entity(eto.entity, rpos.first, rpos.second, xu, yu);
		}
	}

	//render cursor
	if(cursor.in_bounds()) {
		render_cursor();
	}
}

////////

EditorScrollbar::EditorScrollbar(int x, int y, int w, int h) : Fl_Scrollbar(x,y,w,h), editor(nullptr) {}

void EditorScrollbar::set_parent_editor(EditorWidget* editor) {
	this->editor = editor;
}

int EditorScrollbar::handle(int evt) {
	if(evt == 5 || evt == 0x13 || evt == FL_PUSH || evt == FL_RELEASE) {
		auto par = this->window();
		if(par)
			par->redraw();
	}
	else if(evt == 0xC || evt == 0x9)
		return 0;
	else if(evt == FL_FOCUS)
		return 0;
		
	return Fl_Scrollbar::handle(evt);
}