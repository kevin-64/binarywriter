#include "Type.h"
#include "Utilities.h"
#include "shortcuts.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	void swapType(Type& lhs, Type& rhs) noexcept
	{
		std::swap(lhs.m_name, rhs.m_name);
		std::swap(lhs.m_fields, rhs.m_fields);
		std::swap(lhs.m_typeId, rhs.m_typeId);
	}

	Type::Type()
	{
	}

	Type::Type(string name, vector<Field>&& fields, Guid&& guid)
		: m_name(std::move(name)), m_fields(std::move(fields)), m_typeId(std::move(guid))
	{
		m_size = getSize();
	}

	Type::Type(Type&& other) noexcept
		: Type::Type() 
	{
		swapType(*this, other);
	}

	Type& Type::operator=(Type&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		Type t(std::move(rhs));
		swapType(*this, t);
		return *this;
	}

	/* Format of a "Type" record:
	 * 0:	     Type record identifier (0x01)
	 * 1-16:     GUID of the type
	 * 17:       Length of the type name (L)
	 * 18--L+18: Type name characters
	 * L+19:     Number of fields (N)
	 *	         For each field:
	 *			 0:        Length of the field name (F)
	 *			 1--F:     Field name characters
	 *			 F+1:      ID of the field type 
	 */
	vector<char> Type::getData() const
	{
		vector<char> data;
		data.push_back(RecordType::TYPE_DEFINITION);

		auto serGuid = m_typeId.serialize();
		std::copy(begin(serGuid), end(serGuid), std::back_inserter(data));
		data.push_back(static_cast<char>(this->m_name.size()));
		std::copy(begin(this->m_name), end(this->m_name), std::back_inserter(data));
		
		data.push_back(static_cast<char>(this->m_fields.size()));
		for (auto&& f : this->m_fields)
		{
			auto signature = f.getFieldSignature();

			data.push_back(static_cast<char>(signature.first.length()));
			std::copy(begin(signature.first), end(signature.first), std::back_inserter(data));

			data.push_back(signature.second);
		}

		return data;
	}

	int Type::getSize() const
	{
		//the size is cached for performance
		if (m_size != 0)
			return m_size;

		int size = 18;									//record ID, Type GUID (16) and size of the name
		size += this->m_name.size();					//characters of the name
		size++;											//number of fields
		for (auto&& f : this->m_fields)					
		{
			auto signature = f.getFieldSignature();
			size += 1 + signature.first.length();		//length and characters of the field name
			size++;										//type ID of the field
		}

		return size;
	}

	std::string_view Type::getName() const
	{
		return this->m_name;
	}

	const KDB::Primitives::Field& Type::getField(int index) const
	{
		return this->m_fields[index];
	}

	std::unique_ptr<Type> buildType(std::fstream& stream)
	{
		GUID guid;
		stream.read(&reinterpret_cast<char*>(&(guid.Data1))[0], 1);
		stream.read(&reinterpret_cast<char*>(&(guid.Data1))[1], 1);
		stream.read(&reinterpret_cast<char*>(&(guid.Data1))[2], 1);
		stream.read(&reinterpret_cast<char*>(&(guid.Data1))[3], 1);

		stream.read(&reinterpret_cast<char*>(&(guid.Data2))[0], 1);
		stream.read(&reinterpret_cast<char*>(&(guid.Data2))[1], 1);

		stream.read(&reinterpret_cast<char*>(&(guid.Data3))[0], 1);
		stream.read(&reinterpret_cast<char*>(&(guid.Data3))[1], 1);

		stream.read(reinterpret_cast<char*>(&(guid.Data4)), 8);

		Guid g(std::move(guid));

		char counter;
		stream.read(&counter, 1);

		vector<char> name(counter);
		stream.read(name.data(), counter);

		stream.read(&counter, 1);

		vector<Field> fields;
		char fieldCount = counter;
		for (char i = 0; i < fieldCount; i++)
		{
			stream.read(&counter, 1);
			vector<char> fName(counter);
			stream.read(fName.data(), counter);
			char fType;
			stream.read(&fType, 1);
			fields.emplace_back(std::string(fName.begin(), fName.end()), static_cast<FieldType>(fType));
		}

		return std::make_unique<Type>(Type(std::string(name.begin(), name.end()), std::move(fields), std::move(g)));
	}
}