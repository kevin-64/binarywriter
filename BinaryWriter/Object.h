#pragma once

#include <map>
#include <utility>
#include "IDBRecord.h"
#include "Type.h"

namespace KDB::Primitives
{
	class Object : public Contracts::IDBRecord
	{
	private:
		const Type& m_type;
		std::unique_ptr<std::map<std::string, void*>> m_attributes;
	public:
		Object(const Type& type, std::unique_ptr<std::map<std::string, void*>> attributes);
		virtual ~Object() = default;

		Object(Object&& other) = delete;
		Object& operator=(Object&& rhs) = delete;

		Object(const Object& other) = delete;
		Object& operator=(const Object& rhs) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;
	};
}