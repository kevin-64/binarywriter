#pragma once
#include <vector>

/*
 * Defines the interface for classes that require the ability to be written to the database.
 */
namespace KDB::Contracts
{
	class IDBRecord
	{
	public:
		virtual std::vector<char> getData() const = 0;
		virtual int getSize() const = 0;
	};
}
