#pragma once

#include <vector>
#include "IDBRecord.h"

/*
 * Defines the interface for opaque pointers that refer to records in the database.
 */
namespace KDB::Contracts
{
	class IDBPointer: public IDBRecord
	{
	public:
		virtual unsigned long long getAddress() const = 0;
	};
}