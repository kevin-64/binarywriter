#pragma once

#include "FileWriter.h"
#include "IDBRecord.h"
#include "IDBType.h"
#include "ConfigEntry.h"
#include "Pointer.h"
#include "PointerFormat.h"
#include "BlockDefinition.h"
#include <string>
#include <vector>
#include <map>

namespace KDB::Binary {
	class Core final
	{
	private:
		KDB::Primitives::ConfigSettings m_settings;

		FileWriter* m_configFile;
		FileWriter* m_typesFile;
		FileWriter* m_ptrFile;
		FileWriter* m_blocksFile;
		FileWriter* m_diaryFile;
		FileWriter* m_tempFile;

		std::vector<FileWriter> m_storageFiles;
		std::vector<FileWriter> m_indexFiles;

		void readConfiguration();
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

		std::unique_ptr<KDB::Contracts::IDBRecord> getPointer(long long offset);
		void addPointer(const KDB::Primitives::Pointer& ptr);

		std::unique_ptr<KDB::Contracts::IDBRecord> getBlock(long long offset);
		void addBlock(const KDB::Primitives::BlockDefinition& block);

		//temporanea: nella versione finale il chiamante non ha necessità di conoscere il blocco
		std::unique_ptr<KDB::Primitives::BlockDefinition> seekBlock(Guid typeId);
	};
}