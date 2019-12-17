//problemi di overflow tra int e size_t
#pragma warning( disable : 26451)

#include "FileWriter.h"
#include "ConfigSettings.h"
#include "Field.h"
#include "Type.h"
#include "ConfigEntry.h"
#include "Pointer.h"
#include "BlockDefinition.h"
#include "PartitionDefinition.h"
#include "Object.h"
#include "shortcuts.h"

namespace KDB::Binary
{
	using namespace KDB::Primitives;
	using namespace Contracts;

	FileWriter::FileWriter(KDB::Primitives::ConfigSettings* settings, std::string_view filename, bool isVolatile)
		: m_settings(settings), m_fileName(filename)
	{
		//we open the file in the ctor and it will stay open for the lifetime of the class for performance reasons;
		//volatile files are cleared before opening
		if (isVolatile)
			m_stream.open(m_fileName.c_str(), ios::binary | ios::in | ios::out | ios::trunc);
		else
			m_stream.open(m_fileName.c_str(), ios::binary | ios::in | ios::out);
	}

	FileWriter::~FileWriter()
	{
		m_stream.close();
	}

	void swapFileWriters(FileWriter& lhs, FileWriter& rhs) noexcept
	{
		std::swap(lhs.m_fileName, rhs.m_fileName);
		std::swap(lhs.m_stream, rhs.m_stream);
		std::swap(lhs.m_settings, rhs.m_settings);
	}

	FileWriter::FileWriter(FileWriter&& other) noexcept
		: FileWriter()
	{
		swapFileWriters(*this, other);
	}

	FileWriter& FileWriter::operator=(FileWriter&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		FileWriter temp(std::move(rhs));
		swapFileWriters(temp, *this);
		return *this;
	}

	bool FileWriter::writeRecordAfterLast(const IDBRecord& record)
	{
		//for some reason seekp() alone does not work as expected
		m_stream.seekg(0, std::ios_base::end);
		m_stream.seekp(m_stream.tellg());

		auto data = record.getData();
		return writeRecordAtCurrPosition(data, 0);
	}

	bool FileWriter::writeRecordAtCurrPosition(const std::vector<char>& data, int spaceAfter)
	{
		m_stream.write(data.data(), data.size());

		//if we are writing in the middle of the file, we need to signal how much empty space is left after the record
		if (spaceAfter >= 5)
		{
			vector<char> emptyAfter;
			emptyAfter.push_back(DELETED_ENTRY);
			Utilities::push_int(emptyAfter, spaceAfter);
			m_stream.write(emptyAfter.data(), emptyAfter.size());
		}
		else if (spaceAfter > 0)
		{
			//spaces smaller than 5 bytes are handled with padding as they would not be able to contain their own size
			vector<char> padding(spaceAfter, PADDING);
			m_stream.write(padding.data(), spaceAfter);
		}

		return true;
	}

	unsigned long long FileWriter::writeRecordAfterOffset(const Contracts::IDBRecord& record, unsigned long long offset, unsigned long long limit)
	{
		m_stream.seekg(offset);

		//we need to materialize the record here in order to know whether it will fit in a given space or not
		auto data = record.getData();
		auto size = data.size();
		int headerSize = sizeof(int) + 1;

		int spare = -1;
		int next;
		while ((next = m_stream.peek()) != EOF)
		{
			int spaceSize;

			//scan for a free location (indicated by 0xFF as the record type id)
			if (next != RecordType::DELETED_ENTRY) 
			{
				skipRecord();
				continue;
			}

			//check that the free space is large enough for the record; if not, we keep looking
			m_stream.ignore(1); //ignore the empty record marker
			Utilities::read_int(m_stream, &spaceSize);
			spare = spaceSize - size;
			if (spare >= 0)
				break;
			m_stream.ignore(spaceSize - headerSize); //we already consumed 5 bytes for the record type and size
		}

		//go back to the beginning of the record
		m_stream.seekg(-headerSize, std::ios::cur);

		auto currOffset = m_stream.tellg();
		//if we reach/pass the end of the partition or of the file, there is no space to write the record
		if (spare < 0 || currOffset >= limit)
		{
			throw std::runtime_error("Could not write record due to full partition - scanned from " + std::to_string(offset) + " to " + std::to_string(limit) + ".");
		}

		//align writing position to current reading position (which is now in the correct place)
		m_stream.seekp(currOffset);
		writeRecordAtCurrPosition(data, spare);

		//the actual offset at which the record has been written is returned to allow a pointer to this location to be created
		return currOffset;
	}

	std::unique_ptr<IDBRecord> FileWriter::readRecord(long long offset) {
		return this->readRecord(offset, nullptr);
	}

	std::unique_ptr<IDBRecord> FileWriter::readRecord(long long offset, KDB::Primitives::Type* objectType)
	{
		m_stream.seekg(offset);

		char recordType;
		m_stream.read(&recordType, 1);

		auto rType = reinterpret_cast<unsigned char*>(&recordType);

		switch (*rType)
		{
			case RecordType::TYPE_DEFINITION:
				return buildType(m_stream);
			case RecordType::CONFIG_RECORD:
				return buildConfigEntry(m_stream);
			case RecordType::OWNING_POINTER_RECORD:
			case RecordType::SHARED_POINTER_RECORD:
			case RecordType::REFERENCE_POINTER_RECORD:
			case RecordType::HOLD_POINTER_RECORD:
				return buildPointer(m_stream, m_settings->PointerFormat, (Contracts::PointerType)(*rType));
			case RecordType::BLOCK_DEFINITION:
				return buildBlockDefinition(m_stream);
			case RecordType::BLOCK_PARTITION:
				return buildPartitionDefinition(m_stream);
			case RecordType::MAIN_RECORD:
				return buildObject(m_stream, objectType);
			//TODO: altri tipi di record
		}
	}

	bool FileWriter::deleteRecord(long long offset)
	{
		m_stream.seekg(offset);

		char recordType;
		m_stream.read(&recordType, 1);

		auto rType = reinterpret_cast<unsigned char*>(&recordType);

		switch (*rType)
		{
		case RecordType::TYPE_DEFINITION:
			//return deleteType(m_stream);
			break;
		case RecordType::CONFIG_RECORD:
			//return deleteConfigEntry(m_stream);
			break;
		case RecordType::OWNING_POINTER_RECORD:
		case RecordType::SHARED_POINTER_RECORD:
		case RecordType::REFERENCE_POINTER_RECORD:
		case RecordType::HOLD_POINTER_RECORD:
			//return deletePointer(m_stream, m_settings->PointerFormat);
			break;
		case RecordType::BLOCK_DEFINITION:
			//return deleteBlockDefinition(m_stream);
			break;
		case RecordType::BLOCK_PARTITION:
			//return deletePartitionDefinition(m_stream);
			break;
		case RecordType::MAIN_RECORD:
			return deleteObject(m_stream);
			//TODO: altri tipi di record
		}

	}

	void FileWriter::skipRecord()
	{
		char recordType;
		m_stream.read(&recordType, 1);

		auto type = reinterpret_cast<unsigned char*>(&recordType);

		//the amount of bytes to skip depends on the type of record (and, for some types,
		//on the content of the record itself)
		switch (*type)
		{
			case RecordType::PADDING:
				m_stream.ignore(1);
				break;
			case RecordType::TYPE_DEFINITION:
				skipType(m_stream);
				break;
			case RecordType::CONFIG_RECORD:
				skipConfigEntry(m_stream);
				break;
			case RecordType::OWNING_POINTER_RECORD:
			case RecordType::SHARED_POINTER_RECORD:
			case RecordType::REFERENCE_POINTER_RECORD:
			case RecordType::HOLD_POINTER_RECORD:
				skipPointer(m_stream, m_settings->PointerFormat);
				break;
			case RecordType::BLOCK_DEFINITION:
				skipBlockDefinition(m_stream);
				break;
			case RecordType::BLOCK_PARTITION:
				skipPartitionDefinition(m_stream);
				break;
			case RecordType::MAIN_RECORD:
				skipObject(m_stream);
				break;
			//TODO: altri tipi di record
		}
	}

	void FileWriter::allocatePartition(unsigned long long offset, unsigned long long size)
	{
		m_stream.seekp(offset);

		char rType = DELETED_ENTRY;
		m_stream.write(&rType, 1);

		//at the beginning we need to indicate the size of the available space
		vector<char> partSizeMarker;
		Utilities::push_int(partSizeMarker, (int)size);
		m_stream.write(partSizeMarker.data(), sizeof(int));

		//the entire partition is blanked with 0xFF
		auto headerSize = 1 + sizeof(int);
		vector<char> fillVec(size - headerSize, 0xFF);
		m_stream.write(fillVec.data(), size - headerSize);
	}

	std::unique_ptr<BlockDefinition> FileWriter::scanForBlockType(Guid typeId)
	{
		char recordType;
		m_stream.seekg(0);

		while (EOF != m_stream.peek())
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::BLOCK_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning blocks.");

			auto bd = buildBlockDefinition(m_stream);
			if (bd->getTypeId() == typeId)
				return bd;
		}

		auto id = typeId.toString();
		throw std::runtime_error("No blocks have been allocated for type {" + id + "}");
	}

	std::unique_ptr<Contracts::IDBType> FileWriter::scanForTypeDefinition(const std::string& typeName)
	{
		char recordType;
		m_stream.seekg(0);

		while (EOF != m_stream.peek())
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::TYPE_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning types.");

			auto t = buildType(m_stream);
			if (t->getName() == typeName)
				return t;
		}

		throw std::runtime_error("Type '" + typeName + "' has not been recognized.");
	}

	std::unique_ptr<Contracts::IDBType> FileWriter::scanForTypeDefinition(Guid typeId) 
	{
		char recordType;
		m_stream.seekg(0);

		while (EOF != m_stream.peek())
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::TYPE_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning types.");

			auto t = buildType(m_stream);
			if (t->getTypeId() == typeId)
				return t;
		}

		throw std::runtime_error("Type {" + typeId.toString() + "} has not been recognized.");
	}

	std::unique_ptr<BlockDefinition> FileWriter::scanForBlockId(Guid blockId)
	{
		char recordType;
		m_stream.seekg(0);

		while (EOF != m_stream.peek())
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::BLOCK_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning blocks.");

			auto b = buildBlockDefinition(m_stream);
			if (b->getBlockId() == blockId)
				return b;
		}

		throw std::runtime_error("Block {" + blockId.toString() + "} has not been recognized.");
	}

	std::unique_ptr<Contracts::IDBPointer> FileWriter::scanForPointer(unsigned long long address, bool throwIfNoMatch)
	{
		char recordType;
		m_stream.seekg(0);

		while (EOF != m_stream.peek())
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			
			//all pointer entries start with 0xA (0xA0, A1, ...) so we bitmask to find invalid records
			if ((RecordType::OWNING_POINTER_RECORD & (*type)) != RecordType::OWNING_POINTER_RECORD)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning pointers.");

			auto p = buildPointer(m_stream, m_settings->PointerFormat, (Contracts::PointerType)(*type));
			if (p->getAddress() == address)
				return p;
		}

		if (throwIfNoMatch)
			throw std::runtime_error("Pointer " + std::to_string(address) + " has not been recognized.");
		return nullptr;
	}

	std::unique_ptr<Contracts::IDBPointer> FileWriter::scanTempForPointer(unsigned long long address, bool throwIfNoMatch)
	{
		char recordType;
		m_stream.seekg(0);

		while ((recordType = m_stream.peek()) != EOF)
		{
			//all pointer entries start with 0xA (0xA0, A1, ...) so we bitmask to isolate valid records
			if ((RecordType::OWNING_POINTER_RECORD & recordType) != RecordType::OWNING_POINTER_RECORD)
			{
				skipRecord();
				continue;
			}
			else
				m_stream.ignore(1);

			auto p = buildPointer(m_stream, m_settings->PointerFormat, (Contracts::PointerType)(recordType));
			if (p->getAddress() == address)
				return p;
		}

		if (throwIfNoMatch)
			throw std::runtime_error("Pointer " + std::to_string(address) + " has not been recognized.");
		return nullptr;
	}
}