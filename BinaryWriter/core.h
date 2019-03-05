#pragma once

#include "FileWriter.h"
#include "IDBRecord.h"
#include "IDBType.h"
#include "ConfigEntry.h"
#include <string>
#include <vector>
#include <map>

namespace KDB::Binary {
	class Core final
	{
	private:
		FileWriter* m_configFile;
		FileWriter* m_typesFile;
		FileWriter* m_ptrFile;
		FileWriter* m_diaryFile;
		FileWriter* m_tempFile;

		std::vector<FileWriter> m_storageFiles;
		std::vector<FileWriter> m_indexFiles;
		
	public:
		Core(const std::string& definitionFilePath);
		virtual ~Core();

		//no copy or move as this is supposed to be a singleton (per DB)
		Core(const Core& other) = delete;
		Core& operator=(const Core& rhs) = delete;
		Core(Core&& other) = delete;
		Core& operator=(Core&& rhs) = delete;

		//temporanea; nella versione finale si leggono tutti i tipi e si interroga la lista
		std::unique_ptr<KDB::Contracts::IDBRecord> getType(long long offset);

		void addType(const KDB::Contracts::IDBType& type);

		std::unique_ptr<KDB::Contracts::IDBRecord> getConfigEntry(long long offset);
		void addConfigEntry(const KDB::Primitives::ConfigEntry& entry);
	};
}