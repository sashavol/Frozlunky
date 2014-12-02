#include "tile_editor_widget.h"

bool EditorWidget::allow_input() {
	if(!key_press)
		return true;

	return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - *key_press).count() > 20;
}

void EditorWidget::status(unsigned state) {
	if(status_cb)
		status_cb(state);
}

std::vector<Chunk*> EditorWidget::get_chunks() {
	return chunks;
}

void EditorWidget::status_callback(std::function<void(unsigned)> cb) {
	this->status_cb = cb;
}


void EditorWidget::clear_chunk(Chunk* cnk) {
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
	for(Chunk* c : chunks) {
		clear_chunk(c);
	}
	timeline.push_state();
}

void EditorWidget::shift_env_left(int u) {
	cursor.try_dx(-u);
	last_dir = Direction::LEFT;
}

void EditorWidget::shift_env_right(int u) {
	cursor.try_dx(u);
	last_dir = Direction::RIGHT;
}

void EditorWidget::shift_env_up(int u) {
	cursor.try_dy(-u);
	last_dir = Direction::UP;
}

void EditorWidget::shift_env_down(int u) {
	cursor.try_dy(u);
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
			move_drag_start = std::pair<int, int>(ccpos.first, ccpos.second);
			cursor.sx = ccpos.first;
			cursor.sy = ccpos.second;
			cursor.ex = ccpos.first + 1;
			cursor.ey = ccpos.second + 1;
		}
		else if(move_drag_start.first > -1) {
			int ex = ccpos.first, ey = ccpos.second;
			int sx = move_drag_start.first, sy = move_drag_start.second;

			if(ex < sx)
				std::swap(sx, ex);
			if(ey < sy)
				std::swap(sy, ey);

			cursor.sx = sx;
			cursor.sy = sy;
			cursor.ex = ex + 1;
			cursor.ey = ey + 1;
		}
	}
	else {
		move_drag_start = std::pair<int, int>(-1, -1);
			
		char ptile = picker.tile(rx, ry);
		if(ptile != 0) {
			picker.select(ptile);
		}
	}

	parent()->redraw();
}

void EditorWidget::cursor_finish_move() {
	move_drag_start = std::pair<int, int>(-1, -1);
}

char EditorWidget::cursor_tile() {
	return picker.tile();
}

void EditorWidget::cursor_build(int rx, int ry, bool drag) {
	char tile = cursor_tile();
	if(tile == 0)
		return;

	auto affect = [=](int x, int y) {
		Chunk* c = find_chunk(x, y);
		if(c && tile != 0) {
			auto spos = get_chunk_render_pos(c);
			c->tile(c->get_width() * (x - spos.first) / cnk_render_w, c->get_height() * (y - spos.second) / cnk_render_h, tile);
		}
	};

	if(drag) {
		int from_x = last_build.first, from_y = last_build.second;
		int to_x = rx, to_y = ry;
			
		if(from_x > to_x)
			std::swap(from_x, to_x);
		if(from_y > to_y)
			std::swap(from_y, to_y);

		double vx = to_x - from_x, vy = to_y - from_y;
		double len = sqrt(vx*vx + vy*vy);
		if(len >= 1) {
			vx /= len; vy /= len;
			
			for(double x = from_x, y = from_y; x <= to_x && y <= to_y; x += vx, y += vy) {
				affect((int)x, (int)y);
			}
		}
	}
	else {
		affect(rx, ry);
	}

	last_build = std::pair<int, int>(rx, ry);
	parent()->redraw();
}


static cursor_store clipboard;

int EditorWidget::handle(int evt) {
	switch(evt) {
	case FL_PUSH: //mouse pressed
		mouse_down[mouse_event_id()] = true;
			
		if(Fl::event_state() & FL_BUTTON1)
			cursor_move(Fl::event_x(), Fl::event_y(), false);
		else if(Fl::event_state() & FL_BUTTON3)
			cursor_build(Fl::event_x(), Fl::event_y(), false);

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
		else if(Fl::event_state() & FL_BUTTON3)
			cursor_build(Fl::event_x(), Fl::event_y(), true);

		return 1;

	case 0xC: //key typed
		{
			int key = Fl::event_key();

			if(!allow_input())
				break;

			//key_pressed only if not flag key
			if(key != 65505 && key != 65507 && key != 65508 && key != 65513 && key != 65514) { 
				key_press = std::make_shared<clock::time_point>(clock::now());
			}

			cursor_finish_move();

			switch(key) {
			case 65361:
				shift_env_left(ctrl_down ? 2 : 1);
				parent()->redraw();
				return 1;

			case 65362: //up
				shift_env_up(ctrl_down ? 2 : 1);
				parent()->redraw();
				return 1;

			case 65363:
				shift_env_right(ctrl_down ? 2 : 1);
				parent()->redraw();
				return 1;

			case 65364: //down
				shift_env_down(ctrl_down ? 2 : 1);
				parent()->redraw();
				timeline.push_state();
				return 1;

			case 32:    //space
				if(cursor.in_bounds()) {
					cursor.put('0');
					if(!ctrl_down)
						shift_env_right(1);
					parent()->redraw();
					timeline.push_state();
				}
				return 1;

			case 65288: //backspace
				if(cursor.in_bounds()) {
					cursor.put('0');
					if(!ctrl_down)
						shift_env_left(1);
					parent()->redraw();
					timeline.push_state();
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

			case 65535: //delete
				if(cursor.in_bounds()) {
					cursor.put('0');
					parent()->redraw();
					timeline.push_state();
				}
				return 1;

			case 115: //s
				if(ctrl_down) {
					tp->apply_chunks();
					status(STATE_CHUNK_APPLY);
					return 1;
				}

			case 122: //z: undo
				this->take_focus();
				if(ctrl_down) {
					timeline.rewind();
					parent()->redraw();
					return 1;
				}
					
			case 121: //y: redo
				this->take_focus();
				if(ctrl_down) {
					timeline.forward();
					parent()->redraw();
					return 1;
				}

			case 99: //c
				this->take_focus();
				if(ctrl_down && cursor.in_bounds()) {
					clipboard = cursor.encode();
					status(STATE_CHUNK_COPY);
					return 1;
				}

			case 118: //v
				this->take_focus();
				if(ctrl_down && cursor.in_bounds() && !clipboard.empty()) {
					cursor.decode(clipboard);
					status(STATE_CHUNK_PASTE);
					parent()->redraw();
					timeline.push_state();
				}

			default:
				{
					this->take_focus();
					char tile = Fl::event_text()[0];
					if(cursor.in_bounds() && tp->valid_tile(tile)) {
						cursor.put(tile);
						picker.select(tile);
						if(alt_down)
							shift_env_last(1);
						else
							shift_env_right(1);
						status(STATE_CHUNK_WRITE);
						parent()->redraw();
					}
					timeline.push_state();
					return 1;
				}
			}
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

	case FL_FOCUS:
		return 1;
	case FL_UNFOCUS:
		return 1;
	}

	return Fl_Widget::handle(evt);
}


//we now make assumptions about the staticity of the chunks being passed.
void EditorWidget::compute_u() {
	Chunk* cnk = *chunks.begin();
	int ch = cnk->get_height(), cw = cnk->get_width();
	int maxw = cnk_render_w, maxh = cnk_render_h;
	fit_chunk_aspect(cnk, maxw, maxh);

	xu = maxw / cw;
	yu = maxh / ch;
}

EditorWidget::~EditorWidget() {}

EditorWidget::EditorWidget(std::shared_ptr<StaticChunkPatch> tp, int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks, bool extended_mode) : 
	Fl_Widget(x,y,w,h,""),
	chunks(chunks),
	sidebar_scrollbar(scrollbar),
	cnk_render_w(130),
	cnk_render_h(104),
	xu(0),
	yu(0),
	hv_gap(0),
	ctrl_down(false),
	shift_down(false),
	alt_down(false),
	tp(tp),
	extended_mode(extended_mode),
	last_dir(Direction::UP),
	timeline(chunks),
	move_drag_start(-1, -1),
	picker(tp->valid_tiles(), x + w - 87, y, 45, h, 15, 15),
	cursor(chunks, extended_mode ? 2 : 4)
{
	//allocate width for sidebar
	w -= 60;

	std::fill(mouse_down, mouse_down+sizeof(mouse_down), false);

	if(chunks.empty()) {
		this->deactivate();
		return;
	}

	compute_u();

	double ratio = this->h() / (double)(get_chunk_render_pos(*chunks.rbegin()).second - this->y());
	if(ratio >= 1.0) {
		sidebar_scrollbar->deactivate();
		sidebar_scrollbar->slider_size(1.0);
	}
	else { 
		sidebar_scrollbar->activate();
		sidebar_scrollbar->slider_size(ratio);
		sidebar_scrollbar->type(FL_VERTICAL);
		sidebar_scrollbar->bounds(0, get_chunk_render_pos(*chunks.rbegin()).second);
		sidebar_scrollbar->step(10);

		sidebar_scrollbar->callback([](Fl_Widget* wid) {
			wid->redraw();
		});
	}
}

Chunk* EditorWidget::find_chunk(int rx, int ry) {
	int w = this->w();
	int x = this->x(), y = this->y();
	int hc = 0;

	ry += sidebar_scrollbar->value();

	for(Chunk* c : chunks) {
		if((rx >= x && rx < x + cnk_render_w)
		&& (ry >= y && ry < y + cnk_render_h))
		{
			return c;
		}

		hc++;
		x += cnk_render_w + hv_gap;
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h + hv_gap;
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
	return std::pair<int, int>(-1, -1);
}

std::pair<int, int> EditorWidget::render_pos(int tx, int ty) {
	int w = this->w();
	int x = this->x(), y = this->y();
	int xc = 0, yc = 0;
	int hc = 0;

	y += sidebar_scrollbar->value();
	
	for(Chunk* c : chunks) {
		if((tx >= xc && tx < xc + c->get_width())
		&& (ty >= yc && ty < yc + c->get_height()))
		{
			return std::pair<int, int>((tx - xc)*xu + x, (ty - yc)*yu + y);
		}

		hc++;
		x += cnk_render_w + hv_gap;
		xc += c->get_width();
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h + hv_gap;
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
			return std::pair<int, int>(x, y - sidebar_scrollbar->value());

		hc++;
		x += cnk_render_w + hv_gap;
		if(hc == 4 || (extended_mode && hc == 2)) { //TODO this is not very accurate, fix this check.
			x = this->x();
			y += cnk_render_h + hv_gap;
			hc = 0;
		}
	}

	throw std::runtime_error("Chunk from non-native editor.");
}

void EditorWidget::fit_chunk_aspect(Chunk* cnk, int& maxw, int& maxh) {
	int ch = cnk->get_height(), cw = cnk->get_width();
	if(cw < ch) {
		if(maxw < maxh)
			maxh = ch * maxw / cw;
		else
			maxw = cw * maxh / ch;
	}
	else {
		if(maxw < maxh)
			maxw = cw * maxh / ch;
		else
			maxh = ch * maxw / cw;
	}
}

void EditorWidget::render_chunk(Chunk* cnk, int px, int py, int maxw, int maxh) {
	int ch = cnk->get_height(), cw = cnk->get_width();
	fit_chunk_aspect(cnk, maxw, maxh);

	int xu = maxw / cw, yu = maxh / ch;

	//maps chunk coordinates to world coordinates
	auto map = [&](int cx, int cy) -> std::pair<int, int> {
		return std::pair<int, int>(px + cx*maxw / cw, py + cy*maxh / ch); 
	};

	fl_font(FL_SCREEN, min(xu, yu)-2);
	
	for(int cy = 0; cy < ch; cy++) {
		for(int cx = 0; cx < cw; cx++) {
			char tile = cnk->tile(cx, cy);
			auto dp = map(cx, cy);
			draw_tile(tile, dp.first, dp.second, xu, yu);			
		}
	}

	picker.draw();
}

void EditorWidget::render_cursor() {
	std::pair<int, int> s = render_pos(cursor.sx, cursor.sy), e = render_pos(cursor.ex-1, cursor.ey-1);
	e.first += xu; e.second += yu;

	fl_rect(s.first, s.second, e.first - s.first, e.second - s.second, 0xFF000000);
}

void EditorWidget::draw() {
	int x = this->x(), y = this->y();
	int hc = 0;

	for(Chunk* c : chunks) {
		int rx = x, ry = y - sidebar_scrollbar->value();

		if(!(ry < -cnk_render_h || ry > this->h() + cnk_render_h)) {
			render_chunk(c, rx, ry, cnk_render_w, cnk_render_h);
		}
			
		hc++;
		x += cnk_render_w + hv_gap;
		if(hc == 4 || (extended_mode && hc == 2)) {
			x = this->x();
			y += cnk_render_h + hv_gap;
			hc = 0;
		}

		//active chunk
		if(cursor.in_bounds()) {
			render_cursor();
		}
	}
}

////////

EditorScrollbar::EditorScrollbar(int x, int y, int w, int h) : Fl_Scrollbar(x,y,w,h), editor(nullptr) {}

void EditorScrollbar::set_parent_editor(EditorWidget* editor) {
	this->editor = editor;
}

int EditorScrollbar::handle(int evt) {
	if(evt == 5 || evt == 0x13) {
		auto par = parent();
		if(par)
			par->redraw();
	}
	else if(evt == 0xC || evt == 0x9)
		return 0;
	else if(evt == FL_FOCUS)
		return 0;
		
	return Fl_Scrollbar::handle(evt);
}