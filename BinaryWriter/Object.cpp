#include "Object.h"
#include "shortcuts.h"

namespace KDB::Primitives
{
	Object::Object(const Type& type, unique_ptr<map<string, void*>> attributes)
		: m_type(type), m_attributes(std::move(attributes))
	{
	}

	vector<char> Object::getData() const
	{
		//TODO
		return vector<char>();
	}

	int Object::getSize() const
	{
		//TODO
		return 0;
	}
}