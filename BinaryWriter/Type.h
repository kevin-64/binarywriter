#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Field.h"
#include "IDBRecord.h"
#include "IDBType.h"
#include "RecordType.h"
#include "guid.h"

namespace KDB::Primitives 
{
	/*
	* Describes the specifics of an object type as defined by the database client.
	*/
	class Type : public Contracts::IDBType
	{
	private:
		Type() = default;
		std::string m_name;
		std::vector<Field> m_fields;
		Guid m_typeId;
		int m_size;

	public:
		Type(std::string name, std::vector<Field>&& fields, Guid&& guid);
		virtual ~Type() = default;

		friend void swapTypes(Type& lhs, Type& rhs) noexcept;

		//move semantics are supported
		Type(Type&& other) noexcept;
		Type& operator=(Type&& rhs) noexcept;

		//no copy is supported
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		virtual std::string_view getName() const override;
		virtual const Guid& getTypeId() const override;
		virtual const KDB::Primitives::Field& getField(int) const override;
		const char getFieldCount() const;

		friend std::unique_ptr<Type> buildType(std::fstream& stream);
		friend void skipType(std::fstream& stream);
		friend bool deleteType(std::fstream& stream);
	};

	std::unique_ptr<Type> buildType(std::fstream& stream);
	void skipType(std::fstream& stream);
	bool deleteType(std::fstream& stream);
}
