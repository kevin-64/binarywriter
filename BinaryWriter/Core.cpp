#include "core.h"
#include "shortcuts.h"
#include <istream>
#include <fstream>

using ios = std::ios;

namespace KDB::Binary
{

	#pragma region IDs

	//IDs used to identify types of files to read
	const std::string KDBCONF_ID = "kdbconfg";
	const std::string TYPEDEF_ID = "ktypedef";
	const std::string STORAGE_ID = "ks";
	const std::string PTRTABL_ID = "kptrtblf";
	const std::string INDEXES_ID = "ki";
	const std::string DIARYOO_ID = "kdbdoops";
	const std::string VOLATIL_ID = "kdbvolat";

	#pragma endregion

	Core::Core(const std::string& definitionFilePath)
		: m_configFile(nullptr), m_typesFile(nullptr), m_ptrFile(nullptr), m_diaryFile(nullptr), m_tempFile(nullptr)
	{
		std::ifstream definitionFile(definitionFilePath);

		unsigned char initStatus = 0x80; //just the MSB is set here, as there are 7 bits in our check mask
		std::string current;

		while (!definitionFile.eof())
		{
			std::getline(definitionFile, current);

			auto actual = current + ".edb";						//full path with extension
			auto key = current.substr(current.length() - 8, 8);	//just the filename, used to ID the type
			auto start = key.substr(0, 2);						//just the starting 2 chars, used for multi-file IDs

			if (start == STORAGE_ID)
			{
				initStatus |= 1;
				m_storageFiles.emplace_back(&m_settings, actual);
			}
			else if (start == INDEXES_ID)
			{
				initStatus |= (1 << 1);
				m_indexFiles.emplace_back(&m_settings, actual);
			}
			else if (key == TYPEDEF_ID)
			{
				initStatus |= (1 << 2);
				m_typesFile = new FileWriter(&m_settings, actual);
			}
			else if (key == KDBCONF_ID)
			{
				initStatus |= (1 << 3);
				m_configFile = new FileWriter(&m_settings, actual);

				//we read the static part of the database configuration
				readConfiguration();
			}
			else if (key == PTRTABL_ID)
			{
				initStatus |= (1 << 4);
				m_ptrFile = new FileWriter(&m_settings, actual);
			}
			else if (key == DIARYOO_ID)
			{
				initStatus |= (1 << 5);
				m_diaryFile = new FileWriter(&m_settings, actual);
			}
			else if (key == VOLATIL_ID)
			{
				initStatus |= (1 << 6);
				m_tempFile = new FileWriter(&m_settings, actual);
			}
			else 
			{
				throw new std::runtime_error("Unrecognized file type: " + key);
			}
		}

		//if there are missing file categories the initialization is not complete; we return the value of the
		//bitmask to allow debugging the missing file(s)

		//TODO: currently disabled during development
		/*if (initStatus != 0xFF)
		{
			throw new std::runtime_error("Error during initialization: status " + std::to_string(initStatus));
		}*/
	}

	Core::~Core() 
	{
		//ifs are required in case of error during initialization
		if (m_configFile != nullptr)
			delete m_configFile;

		if (m_typesFile != nullptr)
			delete m_typesFile;

		if (m_ptrFile != nullptr)
			delete m_ptrFile;

		if (m_diaryFile != nullptr)
			delete m_diaryFile;

		if (m_tempFile != nullptr)
			delete m_tempFile;
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getType(long long offset)
	{
		return m_typesFile->readRecord(offset);
	}

	void Core::addType(const KDB::Contracts::IDBType& type) 
	{
		m_typesFile->writeRecord(type);
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getConfigEntry(long long offset)
	{
		return m_configFile->readRecord(offset);
	}

	void Core::addConfigEntry(const KDB::Primitives::ConfigEntry& entry)
	{
		m_configFile->writeRecord(entry);
	}

	void Core::addPointer(const KDB::Primitives::Pointer& ptr)
	{
		m_ptrFile->writeRecord(ptr);
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getPointer(long long offset)
	{
		return m_ptrFile->readRecord(offset);
	}
	
	void Core::readConfiguration()
	{
		auto offset = 0;

		auto record = m_configFile->readRecord(offset);
		auto cfgEntry = dynamic_cast<KDB::Primitives::ConfigEntry*>(record.get());
		m_settings.PointerFormat.AddressSize = cfgEntry->getIntValue();
		offset += cfgEntry->getSize();

		record = m_configFile->readRecord(offset);
		cfgEntry = dynamic_cast<KDB::Primitives::ConfigEntry*>(record.get());
		m_settings.PointerFormat.BlockIdSize = cfgEntry->getIntValue();
		offset += cfgEntry->getSize();

		record = m_configFile->readRecord(offset);
		cfgEntry = dynamic_cast<KDB::Primitives::ConfigEntry*>(record.get());
		m_settings.PointerFormat.OffsetSize = cfgEntry->getIntValue();
	}
}