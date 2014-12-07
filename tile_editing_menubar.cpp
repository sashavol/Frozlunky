#include "tile_editing_menubar.h"
#include <map>
#include <vector>
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
	("File", map_list_of
		("New File", tg_('n', true, true, false, "Ctrl+N"))
		("Open File", tg_('o', true, false, false, "Ctrl+O"))
		("Save + Apply", tg_('s', true, false, false, "Ctrl+S"))
		("Switch Editors", tg_(65289, false, false, false, "(Shift+) Tab"))
		("Swap to Default Editor", tg_('d', true, false, false, "Ctrl+D"))
	)
	("Edit", map_list_of
		("Undo", tg_('z', true, false, false, "Ctrl+Z"))
		("Redo", tg_('y', true, false, false, "Ctrl+Y"))
		("Cut", tg_('x', true, false, false, "Ctrl+X"))
		("Copy", tg_('c', true, false, false, "Ctrl+C"))
		("Paste", tg_('v', true, false, false, "Ctrl+V"))
	)
	("Level", map_list_of
		("Randomize Level Seed", tg_('r', true, false, false, "Ctrl+R"))
		("Clear Level", tg_('q', true, true, false, "Ctrl+Shift+Q"))
	)
	("Cursor", map_list_of
		("Select Entire Level", tg_('a', true, false, false, "Ctrl+A"))
		("Reset Cursor Size", tg_(65307, false, false, false, "Esc"))
		("Flood Fill", tg_(102, true, false, false, "Ctrl+F"))
		("Left", tg_(65361, false, false, false, "(Ctrl+) Left"))
		("Right", tg_(65362, false, false, false, "(Ctrl+) Right"))
		("Up", tg_(65363, false, false, false, "(Ctrl+) Up"))
		("Down", tg_(65364, false, false, false, "(Ctrl+) Down"))
		("Clear Tile", tg_(65535, false, false, false, "Delete"))
		("Clear Tile + Right", tg_(32, false, false, false, "Space"))
		("Clear Tile + Left", tg_(65364, false, false, false, "Backspace"))
	);

TileEditingMenuBar::KeyTrigger::KeyTrigger(int key, bool shift, bool ctrl, bool alt, const std::string& hotkey_text) :
	key(key),
	shift(shift),
	ctrl(ctrl),
	alt(alt),
	hotkey_text(hotkey_text)
{}

TileEditingMenuBar::TileEditingMenuBar(int x, int y, int w, int h, key_handler handler) : Fl_Menu_Bar(x, y, w, h) {
	//TODO
}