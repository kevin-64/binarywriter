#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Field.h"
#include "IDBRecord.h"
#include "guid.h"

namespace KDB::Primitives 
{
	/*
	* Describes the specifics of an object type as defined by the database client.
	*/
	class Type : public Contracts::IDBRecord
	{
	private:
		Type();
		std::string m_name;
		std::vector<Field> m_fields;
		Guid m_typeId;
		int m_size;

	public:
		Type(std::string name, std::vector<Field>&& fields, Guid&& guid);
		virtual ~Type() = default;

		friend void swapType(Type& lhs, Type& rhs) noexcept;

		//move semantics are supported
		Type(Type&& other) noexcept;
		Type& operator=(Type&& rhs) noexcept;

		//no copy is supported
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		friend std::unique_ptr<Type> buildType(std::fstream& stream);
	};

	std::unique_ptr<Type> buildType(std::fstream& stream);
}
