#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "FieldType.h"

namespace KDB::Primitives
{
	class Field
	{
	private:
		std::string m_name;
		FieldType m_type;

	public:
		Field(const std::string& name, FieldType type);
		Field(std::string&& name, FieldType type);
		virtual ~Field() = default;

		friend void swap(Field& lhs, Field& rhs) noexcept;

		Field(Field&& other) noexcept;
		Field& operator=(Field&& rhs) noexcept;

		Field(const Field&) = delete;
		Field& operator=(const Field&) = delete;

		std::pair<std::string_view, int> getFieldSignature() const;
	};
}