#pragma once
#include <vector>
#include "Field.h"

/*
 * Defines the interface for types defined by the user in the database.
 */
namespace KDB::Contracts
{
	class IDBType: public IDBRecord
	{
	public:
		virtual std::string_view getName() const = 0;
		virtual const KDB::Primitives::Field& getField(int) const = 0;
	};
}
