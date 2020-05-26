//problemi di overflow tra int e size_t
#pragma warning( disable : 26451)

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
	const std::string BLOCDEF_ID = "kblocksf";
	const std::string STORAGE_ID = "ks";
	const std::string PTRTABL_ID = "kptrtblf";
	const std::string INDEXES_ID = "ki";
	const std::string DIARYOO_ID = "kdbdoops";
	const std::string VOLATIL_ID = "kdbvolat";

	#pragma endregion

	Core::Core(const std::string& definitionFilePath)
		: m_settings(), m_blocksFile(nullptr),
		  m_configFile(nullptr), m_typesFile(nullptr), m_ptrFile(nullptr), m_diaryFile(nullptr), m_tempFile(nullptr)
	{
		std::ifstream definitionFile(definitionFilePath);

		unsigned char initStatus = 0;
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
				m_storageFiles.emplace_back(&m_settings, actual, false);
			}
			else if (start == INDEXES_ID)
			{
				initStatus |= (1 << 1);
				m_indexFiles.emplace_back(&m_settings, actual, false);
			}
			else if (key == TYPEDEF_ID)
			{
				initStatus |= (1 << 2);
				m_typesFile = new FileWriter(&m_settings, actual, false);
			}
			else if (key == KDBCONF_ID)
			{
				initStatus |= (1 << 3);
				m_configFile = new FileWriter(&m_settings, actual, false);

				//we read the static part of the database configuration
				readConfiguration();
			}
			else if (key == PTRTABL_ID)
			{
				initStatus |= (1 << 4);
				m_ptrFile = new FileWriter(&m_settings, actual, false);
			}
			else if (key == BLOCDEF_ID)
			{
				initStatus |= (1 << 5);
				m_blocksFile = new FileWriter(&m_settings, actual, false);
			}
			else if (key == DIARYOO_ID)
			{
				initStatus |= (1 << 6);
				m_diaryFile = new FileWriter(&m_settings, actual, false);
			}
			else if (key == VOLATIL_ID)
			{
				initStatus |= (1 << 7);
				m_tempFile = new FileWriter(&m_settings, actual, true);
			}
			else 
			{
				throw std::runtime_error("Unrecognized file type: " + key);
			}
		}

		//if there are missing file categories the initialization is not complete; we return the value of the
		//bitmask to allow debugging the missing file(s)

		//TODO: currently disabled during development
		/*if (initStatus != 0xFF)
		{
			throw std::runtime_error("Error during initialization: status " + std::to_string(initStatus));
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
		m_typesFile->writeRecordAfterLast(type);
	}

	bool Core::deleteType(const Guid& guid)
	{
		auto block = seekBlock(guid);
		//TODO #fulprt: gestire le altre partizioni; tutte devono essere vuote per poter eliminare il tipo
		auto partInfo = block->getPartitionForWrite();
		auto partFileAndAdj = partInfo.second->getPartitionCoordinates();
		auto partSize = partInfo.second->getPartitionSize();
		auto offset = partInfo.first + partFileAndAdj.second;

		if (m_storageFiles.at(partFileAndAdj.first).anyRecords(offset, offset + partSize))
			throw std::runtime_error("Cannot delete type {" + guid.toString() + "} as there are records for it.");

		unsigned long long deletePos;
		std::tie(deletePos, std::ignore) = m_typesFile->scanForTypeDefinition(guid);
		m_typesFile->deleteRecord(deletePos);
		return true;
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getConfigEntry(long long offset)
	{
		return m_configFile->readRecord(offset);
	}

	void Core::addConfigEntry(const KDB::Primitives::ConfigEntry& entry)
	{
		m_configFile->writeRecordAfterLast(entry);
	}

	std::pair<std::pair<int, unsigned long long>, KDB::Primitives::Type*> Core::findRecord(const KDB::Contracts::IDBPointer& ptr,
																						   bool& isOwner)
	{
		auto realPtr = dynamic_cast<const KDB::Primitives::Pointer*>(&ptr);
		Guid blockId;
		unsigned long long startOffset;

		if (realPtr->isComplete()) {
			startOffset = realPtr->getOffset();
			blockId = realPtr->getBlockId();
			isOwner = (realPtr->getPointerType() == Contracts::PointerType::Owning);
		} else {
			//if the pointer is incomplete (client-side), its value needs to be fetched from the pointers file
			auto ptrDef = m_ptrFile->scanForPointer(realPtr->getAddress(), false);
			if (nullptr == ptrDef) //it might be a volatile pointer
				ptrDef = m_tempFile->scanTempForPointer(realPtr->getAddress(), true);
			auto realPtrDef = dynamic_cast<KDB::Primitives::Pointer*>(ptrDef.get());
			startOffset = realPtrDef->getOffset();
			blockId = realPtrDef->getBlockId();
			isOwner = (realPtrDef->getPointerType() == Contracts::PointerType::Owning);
		}

		auto upBlock = m_blocksFile->scanForBlockId(blockId);
		auto block = upBlock.get();
		auto fileAndOffset = block->getOffsetForRecord(startOffset);

		auto offsetAndType = m_typesFile->scanForTypeDefinition(block->getTypeId());
		auto realType = dynamic_cast<KDB::Primitives::Type*>(offsetAndType.second.release());

		return std::make_pair(fileAndOffset, realType);
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getRecord(const KDB::Contracts::IDBPointer& ptr)
	{
		bool isOwner;
		auto recInfo = findRecord(ptr, isOwner);
		auto fileAndOffset = recInfo.first;
		auto type = recInfo.second;
		return m_storageFiles.at(fileAndOffset.first).readRecord(fileAndOffset.second, type);
	}

	std::unique_ptr<KDB::Contracts::IDBPointer> Core::getShared(const KDB::Contracts::IDBPointer& owningPtr)
	{
		Guid blockId;
		unsigned long long offset;
		bool isOwner;
		
		auto realPtr = dynamic_cast<const Primitives::Pointer*>(&owningPtr);

		if (realPtr->isComplete())
		{
			blockId = realPtr->getBlockId();
			offset = realPtr->getOffset();
			isOwner = (realPtr->getPointerType() == Contracts::PointerType::Owning);
		}
		else
		{
			auto uCompletePtr = m_ptrFile->scanForPointer(realPtr->getAddress(), true);
			auto completePtr = uCompletePtr.get();
			auto realCompletePtr = dynamic_cast<Primitives::Pointer*>(completePtr);
			blockId = realCompletePtr->getBlockId();
			offset = realCompletePtr->getOffset();
			isOwner = (realCompletePtr->getPointerType() == Contracts::PointerType::Owning);
		}

		if (!isOwner)
			throw std::runtime_error("Cannot obtain shared pointer through non-owning pointer.");
		
		//a new shared pointer is generated for the new record
		auto address = createAddress();
		auto ptr = KDB::Primitives::Pointer(m_settings.PointerFormat, address, blockId, offset, Contracts::PointerType::Shared);
		
		//shared pointers are volatile by default unless explicitly persisted
		addTemp(ptr);
		return std::make_unique<KDB::Primitives::Pointer>(std::move(ptr));
	}

	std::unique_ptr<KDB::Contracts::IDBPointer> Core::addRecord(const KDB::Primitives::Object& object)
	{
		auto typeId = object.getTypeId();
		auto block = seekBlock(typeId);

		auto part = block->getPartitionForWrite();
		auto coord = (part.second)->getPartitionCoordinates();

		auto size = (part.second)->getPartitionSize();
		auto offset = part.first + coord.second;

		auto limit = offset + size - 1;
		auto writeOffset = m_storageFiles.at(coord.first).writeRecordAfterOffset(object, offset, limit);
		
		//the owning pointer is generated for the new record, to be stored in the pointer table
		auto address = createAddress();
		auto ptr = KDB::Primitives::Pointer(m_settings.PointerFormat, address, block->getBlockId(), 
										    writeOffset - offset, Contracts::PointerType::Owning);
		addPointer(ptr);
		return std::make_unique<KDB::Primitives::Pointer>(std::move(ptr));
	}

	bool Core::deleteRecord(const KDB::Contracts::IDBPointer& owningPtr)
	{
		bool isOwner;
		auto fileAndOffset = findRecord(owningPtr, isOwner).first;

		if (!isOwner)
			throw std::runtime_error("Cannot delete record through non-owning pointer.");

		return m_storageFiles.at(fileAndOffset.first).deleteRecord(fileAndOffset.second);
	}

	void Core::addPointer(const KDB::Primitives::Pointer& ptr)
	{
		m_ptrFile->writeRecordAfterLast(ptr);
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getPointer(long long offset)
	{
		return m_ptrFile->readRecord(offset);
	}

	void Core::addBlock(const KDB::Primitives::BlockDefinition& block)
	{
		//info about the first partition for the block is extracted in order to create it as requested
		auto blockOffsetAndPartition = block.getPartitionForWrite();
		auto fileAndAdjustment = (blockOffsetAndPartition.second)->getPartitionCoordinates();

		auto size = (blockOffsetAndPartition.second)->getPartitionSize();
		auto offset = blockOffsetAndPartition.first + fileAndAdjustment.second;
		
		//the partition needs to be allocated before use
		m_storageFiles.at(fileAndAdjustment.first).allocatePartition(offset, size);

		m_blocksFile->writeRecordAfterLast(block);
	}

	std::unique_ptr<KDB::Contracts::IDBRecord> Core::getBlock(long long offset)
	{
		return m_blocksFile->readRecord(offset);
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
		m_settings.PointerFormat.OffsetSize = cfgEntry->getIntValue();
	}

	unsigned long long Core::createAddress()
	{
		const int maxTries = 100;
		int tries = 0;
		unsigned long long next;
		do {
			next = m_rng();
			if (nullptr == m_ptrFile->scanForPointer(next, false))
				return next;
		} while (++tries < maxTries);

		throw std::runtime_error("Fatal error: could not obtain an unused address.");
	}

	void Core::addTemp(const KDB::Contracts::IDBRecord& record)
	{
		m_tempFile->writeRecordAfterLast(record);
	}

	std::unique_ptr<KDB::Primitives::BlockDefinition> Core::seekBlock(const Guid& typeId)
	{
		return m_blocksFile->scanForBlockType(typeId);
	}

	std::unique_ptr<KDB::Contracts::IDBType> Core::seekType(const std::string& typeName)
	{
		return m_typesFile->scanForTypeDefinition(typeName);
	}
}