#include "core.h"
#include <istream>
#include <fstream>

using ios = std::ios;

namespace KDB::Binary
{
	Core::Core(const std::string& definitionFilePath)
	{
		std::ifstream definitionFile(definitionFilePath);

		std::string current;
		while (!definitionFile.eof())
		{
			std::getline(definitionFile, current);
			m_fileWriters.emplace_back(current);
		}
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getType(long long offset)
	{
		return m_fileWriters.at(0).readRecord(offset);
	}
}