#include "shortcuts.h"
#include "Utilities.h"

namespace KDB::Utilities
{
	void push_int(vector<char>& vec, int value)
	{
		vec.push_back((char)value & 0x000000FF);
		vec.push_back((char)value & 0x0000FF00);
		vec.push_back((char)value & 0x00FF0000);
		vec.push_back((char)value & 0xFF000000);
	}
}
