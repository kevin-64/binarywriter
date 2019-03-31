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

	//extern std::unique_ptr<Type> KDB::Primitives::buildType(std::fstream& stream);
	using namespace Contracts;

	FileWriter::FileWriter(KDB::Primitives::ConfigSettings* settings, std::string_view filename)
		: m_settings(settings), m_fileName(filename)
	{
		//we open the file in the ctor and it will stay open for the lifetime of the class for performance reasons
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

	bool FileWriter::writeRecord(const IDBRecord& record)
	{
		auto data = record.getData();
		m_stream.write(data.data(), data.size());
		return true;
	}

	bool FileWriter::writeRecordAfterOffset(const Contracts::IDBRecord& record, unsigned long long offset, unsigned long long limit)
	{
		m_stream.seekp(offset);
		
		//scan for a free location (indicated by 0xFF as the record type id)
		while (m_stream.peek() != RecordType::DELETED_ENTRY)
		{
			skipRecord();
		}

		//??? Why the hell is this necessary??? Aren't read/write operations sync'd in iostream??? And yet...
		m_stream.seekp(m_stream.tellg());

		//TODO: check whether there actually is enough space to write the whole record, not just if we are at the end
		//if we reach/pass the end, there is no space to write the record
		if (m_stream.tellp() >= limit)
		{
			throw std::runtime_error("Could not write record due to full partition - scanned from " + std::to_string(offset) + " to " + std::to_string(limit) + ".");
		}

		writeRecord(record);

		return true;
	}

	std::unique_ptr<IDBRecord> FileWriter::readRecord(long long offset) 
	{
		m_stream.seekg(offset);

		char recordType;
		m_stream.read(&recordType, 1);

		auto type = reinterpret_cast<unsigned char*>(&recordType);

		switch (*type)
		{
			case RecordType::TYPE_DEFINITION:
				return buildType(m_stream);
			case RecordType::CONFIG_RECORD:
				return buildConfigEntry(m_stream);
			case RecordType::POINTER_RECORD:
				return buildPointer(m_stream, m_settings->PointerFormat);
			case RecordType::BLOCK_DEFINITION:
				return buildBlockDefinition(m_stream);
			case RecordType::BLOCK_PARTITION:
				return buildPartitionDefinition(m_stream);
			case RecordType::MAIN_RECORD:
				return buildObject(m_stream);
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
			case RecordType::TYPE_DEFINITION:
				skipType(m_stream);
			case RecordType::CONFIG_RECORD:
				skipConfigEntry(m_stream);
			case RecordType::POINTER_RECORD:
				skipPointer(m_stream, m_settings->PointerFormat);
			case RecordType::BLOCK_DEFINITION:
				skipBlockDefinition(m_stream);
			case RecordType::BLOCK_PARTITION:
				skipPartitionDefinition(m_stream);
			case RecordType::MAIN_RECORD:
				skipObject(m_stream);
			//TODO: altri tipi di record
		}
	}

	void FileWriter::allocatePartition(unsigned long long offset, unsigned long long size)
	{
		m_stream.seekp(offset);
		
		vector<char> fillVector{ (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF };
		int64 count = 0;

		//the entire partition is blanked with 0xFF, 8 bytes at a time
		do
		{
			m_stream.write(fillVector.data(), fillVector.size());
			count += sizeof(int64);
		} while (count < size);
	}

	std::unique_ptr<BlockDefinition> FileWriter::scanForBlockType(Guid typeId)
	{
		char recordType;
		m_stream.seekg(0);

		do 
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::BLOCK_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning blocks.");

			auto bd = buildBlockDefinition(m_stream);
			if (bd->getTypeId() == typeId)
				return bd;
		} while (m_stream.peek() != EOF);

		auto id = typeId.toString();
		throw std::runtime_error("No blocks have been allocated for type {" + id + "}");
	}

	std::unique_ptr<Contracts::IDBType> FileWriter::scanForTypeDefinition(const std::string& typeName)
	{
		char recordType;
		m_stream.seekg(0);

		do 
		{
			m_stream.read(&recordType, 1);
			auto type = reinterpret_cast<unsigned char*>(&recordType);
			if (RecordType::TYPE_DEFINITION != *type)
				throw std::runtime_error("Internal error: invalid record type " + std::to_string(*type) + " while scanning types.");

			auto t = buildType(m_stream);
			if (t->getName() == typeName)
				return t;
		} while (m_stream.peek() != EOF);

		throw std::runtime_error("Type {" + typeName + "} has not been recognized.");
	}
}