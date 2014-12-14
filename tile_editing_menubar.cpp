#include "tile_editing_menubar.h"
#include <unordered_map>
#include <vector>
#include <sstream>
#include <boost/assign.hpp>

using boost::assign::map_list_of;
using boost::assign::list_of;
typedef TileEditingMenuBar::KeyTrigger tg_;

/*
	File:
		'n' -> ctrl+shift+n -> new file
		'o' -> ctrl+o -> open file
		115 -> ctrl+s -> save + apply edited chunks
		65289 -> (shift+) tab -> switch editors
		'd' -> ctrl+d -> swap to and from default tab

	Edit:
		122 -> ctrl+z -> undo
		121 -> ctrl+y -> redo
		120 -> ctrl+x -> cut
		99 -> ctrl+c -> copy
		118 -> ctrl+v -> paste
	
	Level:
		'r' -> ctrl+r -> randomize level seed
		'q' -> ctrl+shift+q -> clear level
	
	Cursor:
		97 -> ctrl+a -> select entire level
		65307 -> esc -> reset cursor size to 1x1
		102 -> ctrl+f -> flood fill with picked tile, current tile under cursor as empty space
		65361 -> (ctrl+) left -> move cursor left
		65362 -> (ctrl+) up -> move cursor up
		65363 -> (ctrl+) right -> move cursor right
		65364 -> (ctrl+) left -> move cursor left
		65535 -> delete -> empty current tile
		32 -> space -> empty current tile + move cursor right
		65288 -> backspace -> empty current tile + move cursor left
*/



static std::map<std::string, std::map<std::string, TileEditingMenuBar::KeyTrigger>> menu_data = map_list_of
	("aFile", map_list_of
		("aNew Level Pack", tg_('n', true, true, false, "Ctrl+N"))
		("bOpen Level Pack", tg_('o', true, false, false, "Ctrl+O"))
		("cSave + Apply", tg_('s', true, false, false, "Ctrl+S"))
		("dSave as..", tg_('s', true, true, false, "Ctrl+Shift+S"))
		("eSwitch Editor", tg_(65289, false, false, false, "(Shift+) Tab"))
		("fSwap to Default Editor", tg_('d', true, false, false, "Ctrl+D"))
	)
	("bEdit", map_list_of
		("aUndo", tg_('z', true, false, false, "Ctrl+Z"))
		("bRedo", tg_('y', true, false, false, "Ctrl+Y"))
		("cCut", tg_('x', true, false, false, "Ctrl+X"))
		("dCopy", tg_('c', true, false, false, "Ctrl+C"))
		("ePaste", tg_('v', true, false, false, "Ctrl+V"))
	)
	("cLevel", map_list_of
		("aForce Level", tg_('e', true, false, false, "Ctrl+E"))
		("bRandomize Level Seed", tg_('r', true, false, false, "Ctrl+R"))
		("cClear Level", tg_('q', true, true, false, "Ctrl+Shift+Q"))
	)
	("dCursor", map_list_of
		("aSelect Entire Level", tg_('a', true, false, false, "Ctrl+A"))
		("bReset Cursor Size", tg_(65307, false, false, false, "(Shift+) Esc"))
		("cFlood Fill", tg_(102, true, false, false, "Ctrl+F"))
		("dLeft", tg_(65361, false, false, false, "(Ctrl+) Left"))
		("eRight", tg_(65362, false, false, false, "(Ctrl+) Right"))
		("fUp", tg_(65363, false, false, false, "(Ctrl+) Up"))
		("gDown", tg_(65364, false, false, false, "(Ctrl+) Down"))
		("hClear Tile", tg_(65535, false, false, false, "Delete"))
		("iClear Tile + Right", tg_(32, false, false, false, "Space"))
		("jClear Tile + Left", tg_(65288, false, false, false, "Backspace"))
	);

TileEditingMenuBar::KeyTrigger::KeyTrigger(int key, bool ctrl, bool shift, bool alt, const std::string& hotkey_text) :
	key(key),
	shift(shift),
	ctrl(ctrl),
	alt(alt),
	hotkey_text(hotkey_text)
{}

//std::function<void()>
static void heap_function_invoke_(Fl_Widget* widget, void* data) {
	typedef std::function<void(Fl_Widget*)> void_fn_type;
	(*static_cast<void_fn_type*>(data))(widget);
}

TileEditingMenuBar::~TileEditingMenuBar() {
	for(auto&& fn : heap_functions) {
		delete fn;
	}
	heap_functions.clear();
}

TileEditingMenuBar::TileEditingMenuBar(int x, int y, int w, int h, key_handler handler) : 
	Fl_Menu_Bar(x, y, w, h), 
	handler(handler) 
{	
	std::function<void(Fl_Widget*)>* las;
	for(auto&& menu : menu_data) {
		double max_w = 0;

		auto menu_name_build = [&](const std::pair<std::string, TileEditingMenuBar::KeyTrigger>& option) {
			std::ostringstream ss;
			ss << menu.first.substr(1) << "/" << option.first.substr(1) << "   ";
			
			double targ = max_w - fl_width(option.second.hotkey_text.c_str());
			while(fl_width(ss.str().c_str()) < targ) {
				ss << " ";
			}

			ss << option.second.hotkey_text;

			return ss.str();
		};

		max_w = fl_width(menu_name_build(*std::max_element(menu.second.begin(), menu.second.end(), 
			[=](const std::pair<const std::string, TileEditingMenuBar::KeyTrigger>& a, 
				const std::pair<const std::string, TileEditingMenuBar::KeyTrigger>& b) 
			{
				return menu_name_build(a).size() < menu_name_build(b).size();	
			})).c_str());

		for(auto&& option : menu.second) {
			this->add(
				menu_name_build(option).c_str(),
				0, 
				heap_function_invoke_, 
				las = new std::function<void(Fl_Widget*)>([=](Fl_Widget* widget) {
					this->handler(option.second);
				})
			);
			heap_functions.push_back(las);
		}
	}
}