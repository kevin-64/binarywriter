//disabilitiamo gli warning per il troncamento e gli overflow
#pragma warning( disable : 4305 4309)
#pragma warning( disable: 26451)

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

		data.push_back(RecordType::TYPE_DEFINITION);

		auto serGuid = m_typeId.serialize();
		Utilities::push_vector(data, serGuid);
		Utilities::push_char(data, static_cast<char>(this->m_name.size()));

		Utilities::push_string(data, this->m_name);
		
		Utilities::push_char(data, static_cast<char>(this->m_fields.size()));
		for (auto&& f : this->m_fields)
		{
			auto& fieldName = f.getFieldName();
			auto fieldType = f.getFieldType();

			Utilities::push_char(data, static_cast<char>(fieldName.length()));
			Utilities::push_stringview(data, fieldName);

			Utilities::push_char(data, static_cast<char>(fieldType));
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
			auto fieldName = f.getFieldName();
			size += 1 + fieldName.length();				//length and characters of the field name
			size++;										//type ID of the field
		}

		return size;
	}

	std::string_view Type::getName() const
	{
		return this->m_name;
	}

	const Guid& Type::getTypeId() const
	{
		return this->m_typeId;
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

	bool deleteType(std::fstream& stream)
	{
		unsigned long long recordStart = stream.tellg();
		recordStart--;

		//skip the type id (the record type was already consumed by the caller)
		stream.ignore(16);
		auto totalSize = 1 + 16;

		//get and skip the name size
		char nameSize;
		stream.read(&nameSize, 1);
		stream.ignore(nameSize);
		totalSize += nameSize + 1;

		char totFields = 0;
		char fieldIndex;
		while ((fieldIndex = stream.peek()) != EOF)
		{
			//if we reach another entry or an empty record we are finished
			if ((fieldIndex == DELETED_ENTRY) || (totFields > 0 && fieldIndex == TYPE_DEFINITION))
				break;

			stream.ignore(1); //field index
			stream.read(&nameSize, 1); //field name length
			stream.ignore(nameSize + 1); //field name and field type identifier
			totalSize += nameSize + 2;

			totFields++;
		}

		//we go back to the beginning for the overwrite
		stream.seekp((unsigned long long)recordStart);

		//we overwrite with empty space, indicating how much is available at the beginning (after the record type)
		vector<char> emptySpace(totalSize, DELETED_ENTRY);
		Utilities::writeover_int(emptySpace, 1, totalSize);
		stream.write(emptySpace.data(), emptySpace.size());

		return true;
	}
}