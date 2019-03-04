#include "FileWriter.h"
#include "Field.h"
#include "Type.h"
#include "shortcuts.h"

namespace KDB::Binary
{
	using namespace KDB::Primitives;

	extern std::unique_ptr<Type> KDB::Primitives::buildType(std::fstream& stream);
	using namespace Contracts;

	FileWriter::FileWriter(std::string_view filename)
		: m_fileName(filename)
	{
		//we open the file in the ctor and it will stay open for the lifetime of the class for performance reasons
		m_stream.open(m_fileName.c_str(), ios::binary | ios::in | ios::out | ios::app);
	}

	FileWriter::~FileWriter()
	{
		m_stream.close();
	}

	void swapFileWriters(FileWriter& lhs, FileWriter& rhs) noexcept
	{
		std::swap(lhs.m_fileName, rhs.m_fileName);
		std::swap(lhs.m_stream, rhs.m_stream);
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
		m_stream.write(record.getData().data(), record.getSize());
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
			//TODO: other types of record
		}
	}
}