#pragma once

#include "FileWriter.h"
#include "IDBRecord.h"
#include <string>
#include <vector>

namespace KDB::Binary {
	class Core final
	{
	private:
		std::vector<FileWriter> m_fileWriters;
	public:
		Core(const std::string& definitionFilePath);
		~Core() = default;

		//no copy or move as this is supposed to be a singleton (per DB)
		Core(const Core& other) = delete;
		Core& operator=(const Core& rhs) = delete;
		Core(Core&& other) = delete;
		Core& operator=(Core&& rhs) = delete;

		//temporanea; nella versione finale si leggono tutti i tipi e si interroga la lista
		std::unique_ptr<KDB::Contracts::IDBRecord> getType(long long offset);
	};
}