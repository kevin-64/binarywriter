#pragma once

#include <map>
#include <utility>
#include "IDBRecord.h"
#include "Type.h"

namespace KDB::Primitives
{
	const unsigned char EMPTY_FIELD_LIST_MARKER = 0xFD;
	const unsigned char TYPE_ENTRY_MARKER = 0xFE;

	const unsigned char LONG_RECORD_MARKER = 0xE0;
	const unsigned char EMBEDDED_RECORD_MARKER = 0xFD;
	const unsigned char REMOTE_RECORD_MARKER = 0xFE;
	const unsigned char EMPTY_SINGLE_FIELD_MARKER = 0xFF;

	class Object : public Contracts::IDBRecord
	{
	private:
		Object() = default;
		const Type* m_type;
		std::map<std::string, void*> m_attributes;
		int m_size;

		int writeFieldData(std::vector<char>& data, void* fieldData, FieldType fieldType) const;
	public:
		Object(const Type* type, std::map<std::string, void*>&& attributes);
		virtual ~Object() = default;

		friend void swapObjects(Object& lhs, Object& rhs) noexcept;

		//move semantics are supported
		Object(Object&& other) noexcept;
		Object& operator=(Object&& rhs) noexcept;

		//No copy is allowed
		Object(const Object& other) = delete;
		Object& operator=(const Object& rhs) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;
		
		const Guid& getTypeId() const;

		friend std::unique_ptr<Object> buildObject(std::fstream& stream, const KDB::Primitives::Type* type);
		friend void skipObject(std::fstream& stream);
		friend bool deleteObject(std::fstream& stream);
	};

	std::unique_ptr<Object> buildObject(std::fstream& stream, const KDB::Primitives::Type* type);
	void skipObject(std::fstream& stream);
	bool deleteObject(std::fstream& stream);
}