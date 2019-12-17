#pragma once

#include <vector>
#include "IDBRecord.h"

/*
 * Defines the interface for opaque pointers that refer to records in the database.
 */
namespace KDB::Contracts
{
	enum class PointerType
	{
		//Pointer objects created by the user have their type resolved only when they are dereferenced
		Unspecified = 0xFF,
		//Main pointer to the object; allows read and write access
		Owning = 0xA0,
		//Allows read-only access to multiple clients at once; it does not stop modification or deletion by the owner
		Shared = 0xA1,
		//Allows read-only access to multiple clients and guarantees the existence of the object as long as it is held
		Reference = 0xA2,
		//Allows read-only access to multiple clients and guarantees immutability of the record as long as it is held
		Hold = 0xA3
	};

	class IDBPointer: public IDBRecord
	{
	public:
		virtual unsigned long long getAddress() const = 0;
		virtual PointerType getPointerType() const = 0;
	};
}