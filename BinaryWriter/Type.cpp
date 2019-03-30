#include "Type.h"
#include "Utilities.h"
#include "shortcuts.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	void swapTypes(Type& lhs, Type& rhs) noexcept
	{
		std::swap(lhs.m_name, rhs.m_name);
		std::swap(lhs.m_fields, rhs.m_fields);
		std::swap(lhs.m_typeId, rhs.m_typeId);
	}

	Type::Type()
	{
	}

	Type::Type(string name, vector<Field>&& fields, Guid&& guid)
		: m_size(0), m_name(std::move(name)), m_fields(std::move(fields)), m_typeId(std::move(guid))
	{
		m_size = getSize();
	}

	Type::Type(Type&& other) noexcept
		: Type::Type() 
	{
		swapTypes(*this, other);
	}

	Type& Type::operator=(Type&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		Type t(std::move(rhs));
		swapTypes(*this, t);
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

//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::TYPE_DEFINITION);

		auto serGuid = m_typeId.serialize();
		Utilities::push_vector(data, serGuid);
		Utilities::push_char(data, static_cast<char>(this->m_name.size()));

		Utilities::push_string(data, this->m_name);
		
		Utilities::push_char(data, static_cast<char>(this->m_fields.size()));
		for (auto&& f : this->m_fields)
		{
			auto signature = f.getFieldSignature();

			Utilities::push_char(data, static_cast<char>(signature.first.length()));
			Utilities::push_stringview(data, signature.first);

			Utilities::push_char(data, static_cast<char>(signature.second));
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

	const Guid& Type::getTypeId() const
	{
		return this->m_typeId;
	}

	std::string_view Type::getName() const
	{
		return this->m_name;
	}

	const KDB::Primitives::Field& Type::getField(int index) const
	{
		return this->m_fields[index];
	}

	const char Type::getFieldCount() const
	{
		return static_cast<char>(this->m_fields.size());
	}

	std::unique_ptr<Type> buildType(std::fstream& stream)
	{
		GUID guid;
		Utilities::read_GUID(stream, &guid);

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

	void skipType(std::fstream& stream)
	{
		//type records are highly variable so it is more efficient to just read them in order to advance the stream position
		buildType(stream);
	}
}