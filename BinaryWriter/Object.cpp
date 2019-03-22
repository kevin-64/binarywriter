#include "Object.h"
#include "shortcuts.h"
#include "Utilities.h"

namespace KDB::Primitives
{
	void swapObjects(Object& lhs, Object& rhs) noexcept
	{
		std::swap(lhs.m_type, rhs.m_type);
		std::swap(lhs.m_attributes, rhs.m_attributes);
		std::swap(lhs.m_size, rhs.m_size);
	}

	Object::Object(const Type* type, const map<string, void*>* attributes)
		: m_type(type), m_attributes(attributes), m_size(0)
	{
		m_size = getSize();
	}

	Object::Object(Object&& other) noexcept
		: Object::Object()
	{
		swapObjects(*this, other);
	}

	Object& Object::operator=(Object&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		Object obj(std::move(rhs));
		swapObjects(*this, obj);
		return *this;
	}

	/* Format of an "Object" record:
	 * 0:	     Object record identifier (0x02)
	 * 1-8:      Record size (RS)
	 *	         For each field:
	 *			 0:        Field index (0x00-0xEF), empty field list (0xFD) or type entry (0xFE)
	 *
	 *			 I)		For single empty fields:
	 *			 1		   Empty field marker (0xFF)
	 *			 
	 *			 II)	For remote fields:
	 *			 1		   Remote field marker (0xFE)
	 *			 2-PS+1	   Pointer to actual field location
	 *
	 *			 III)	For embedded record fields:
	 *			 1		   Embedded field marker (0xFD)
	 *			 2		   Embedded record identifier (0x03)
	 *			 3-L+2	   Record data (similar to a normal object record, but RS is 2 bytes long)
	 *
	 *			 IV)	For regular fields:
	 *			 1		   Field record size (FRS) (0x01-0xDF)
	 *			 2-FRS+1   Field data
	 *
	 *			 V)		For long fields:
	 *			 1		   Long field marker (0xE0)
	 *			 2-5	   Field record size (FRS)
	 *
	 *			 VI)	For empty field lists:
	 *			 1		   First empty field index (inclusive)
	 *			 2		   Last empty field index (inclusive)
	 *
	 *			 VII)	For type entry fields:
	 *		     1-16	   Type ID		   
	 */
	vector<char> Object::getData() const
	{
		vector<char> data;
		int size = 1 + sizeof(int); //record ID + size of the size itself

//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::MAIN_RECORD);

		//the actual record size is not known yet; we will amend it at the end of the record processing
		Utilities::push_int(data, /*recordSize =*/ 0); 

		char beginEmpty = -1;
		char endEmpty = -1;

		for (char i = 0; i < m_type->getFieldCount(); i++)
		{
			auto& fieldDef = m_type->getField(i);
			auto fieldSig = fieldDef.getFieldSignature();
			const std::string& fieldName = fieldSig.first;

			//if a field is present, we immediately write it; if it is not, we keep track of it so that
			//multiple missing fields can be written in a single empty list
			auto fieldIt = m_attributes->find(fieldDef.getFieldSignature().first);
			if (m_attributes->end() != fieldIt)
			{
				//when a field is present, we have to write any missing fields found before it
				if (beginEmpty != -1)
				{
					if (beginEmpty != endEmpty)
					{
						//empty field list
						Utilities::push_char(data, EMPTY_FIELD_LIST_MARKER);
						Utilities::push_char(data, beginEmpty);
						Utilities::push_char(data, endEmpty);
						size += 3;
					}
					else
					{
						//single empty field
						Utilities::push_char(data, beginEmpty);
						Utilities::push_char(data, EMPTY_FIELD_LIST_MARKER);
						size += 2;
					}

					beginEmpty = endEmpty = -1;
				}

				Utilities::push_char(data, i); //current field index
				auto fieldSize = writeFieldData(data, fieldIt->second, static_cast<FieldType>(fieldSig.second));
				size += 1 + fieldSize;
			}
			else
			{
				if (beginEmpty == -1) beginEmpty = i;
				endEmpty = i;
			}
		}

		//the actual record size is written over the incorrect value previously used as a placeholder
		Utilities::writeover_int(data, 1, size);
		return data;
	}

	int Object::getSize() const
	{
		return 0; //the size can only be dynamically determined by getData() and is not known here
	}

	std::unique_ptr<Object> buildObject(std::fstream& stream)
	{
		//TODO
		return nullptr;
	}

	//utility function to write individual fields to the data vector
	int Object::writeFieldData(std::vector<char> data, void* fieldData, FieldType fieldType) const
	{
		int size;
		switch (fieldType)
		{
			case FieldType::Boolean:
				data.push_back(sizeof(bool));
				data.push_back(*reinterpret_cast<bool*>(fieldData));
				return 1 + sizeof(bool);
			case FieldType::Integer:
				data.push_back(sizeof(int));
				Utilities::push_int(data, *reinterpret_cast<int*>(fieldData));
				return 1 + sizeof(int);
			case FieldType::String:
				Utilities::push_char(data, LONG_RECORD_MARKER);
				size = *reinterpret_cast<int*>(fieldData); //each string first contains its size
				Utilities::push_int(data, size);
				Utilities::push_memory(data, size, sizeof(int) + reinterpret_cast<char*>(fieldData));
				return 1 + sizeof(int) + size;
			case FieldType::UUID:
				data.push_back(16);
				Utilities::push_vector(data, reinterpret_cast<Guid*>(fieldData)->serialize());
				return 17;
		}

		throw std::runtime_error("Type {" + std::to_string((int)fieldType) + "} cannot be resolved.");
	}
}