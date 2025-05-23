#pragma once

#include "IDBRecord.h"
#include "ConfigSettings.h"
#include "RecordType.h"
#include "BlockDefinition.h"
#include "Pointer.h"
#include "Type.h"
#include "IDBType.h"
#include "IDBPointer.h"
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>

namespace KDB::Binary
{
	class FileWriter
	{
	private:
		KDB::Primitives::ConfigSettings* m_settings;
		std::string m_fileName;
		std::fstream m_stream;
		FileWriter() = default;

		void skipRecord();
		bool writeRecordAtCurrPosition(const std::vector<char>& data, int spaceAfter);
	public:
		FileWriter(KDB::Primitives::ConfigSettings* settings, std::string_view filename, bool isVolatile);
		virtual ~FileWriter();

		FileWriter(const FileWriter&) = delete;
		FileWriter& operator=(const FileWriter&) = delete;

		friend void swapFileWriters(FileWriter& lhs, FileWriter& rhs) noexcept;

		FileWriter(FileWriter&&) noexcept;
		FileWriter& operator=(FileWriter&&) noexcept;

		bool writeRecordAfterLast(const Contracts::IDBRecord& record);
		unsigned long long writeRecordAfterOffset(const Contracts::IDBRecord& record, unsigned long long offset, unsigned long long limit);

		bool anyRecords(unsigned long long startOffset, unsigned long long limit);
		bool anyReferences(const KDB::Primitives::Pointer& ptr);
		std::unique_ptr<Contracts::IDBRecord> readRecord(unsigned long long offset);
		std::unique_ptr<Contracts::IDBRecord> readRecord(unsigned long long offset, const KDB::Primitives::Type* objectType);

		bool deleteRecord(unsigned long long offset);

		void allocatePartition(unsigned long long offset, unsigned long long size);

		std::unique_ptr<Primitives::BlockDefinition> scanForBlockType(Guid typeId);
		std::unique_ptr<Contracts::IDBType> scanForTypeDefinition(const std::string& typeName, bool throwIfNoMatch);
		std::pair<unsigned long long, std::unique_ptr<Primitives::Type>> scanForTypeDefinition(const Guid& typeId);
		std::unique_ptr<Primitives::BlockDefinition> scanForBlockId(Guid blockId);
		std::unique_ptr<Contracts::IDBPointer> scanForPointer(unsigned long long address, bool throwIfNoMatch);
		std::unique_ptr<Contracts::IDBPointer> scanTempForPointer(unsigned long long address, bool throwIfNoMatch);
	};
}