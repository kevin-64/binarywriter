#include "Field.h"
#include "shortcuts.h"

using signPair = std::pair<std::string_view, int>;

namespace KDB::Primitives
{
	void swap(Field& lhs, Field& rhs) noexcept
	{
		std::swap(lhs.m_name, rhs.m_name);
		std::swap(lhs.m_type, rhs.m_type);
	}

	Field::Field(const string& name, FieldType type)
		: m_name(name), m_type(type)
	{
	}

	Field::Field(string&& name, FieldType type)
		: m_name(std::move(name)), m_type(type)
	{
	}

	Field::Field(Field&& other) noexcept
	{
		swap(*this, other);
	}

	Field& Field::operator=(Field&& rhs) noexcept
	{
		if (this == &rhs)
			return *this;

		Field f(std::move(rhs));
		swap(*this, f);
		return *this;
	}

	signPair Field::getFieldSignature() const
	{
		return std::make_pair(std::string_view(this->m_name), static_cast<char>(this->m_type));
	}
}