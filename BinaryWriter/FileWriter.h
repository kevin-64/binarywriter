#pragma once

#include "IDBRecord.h"
#include "ConfigSettings.h"
#include "RecordType.h"
#include "BlockDefinition.h"
#include "Type.h"
#include "IDBType.h"
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
	public:
		FileWriter(KDB::Primitives::ConfigSettings* settings, std::string_view filename);
		virtual ~FileWriter();

		FileWriter(const FileWriter&) = delete;
		FileWriter& operator=(const FileWriter&) = delete;

		friend void swapFileWriters(FileWriter& lhs, FileWriter& rhs) noexcept;

		FileWriter(FileWriter&&) noexcept;
		FileWriter& operator=(FileWriter&&) noexcept;

		bool writeRecord(const Contracts::IDBRecord& record);
		unsigned long long writeRecordAfterOffset(const Contracts::IDBRecord& record, unsigned long long offset, unsigned long long limit);
		std::unique_ptr<Contracts::IDBRecord> readRecord(long long offset);
		std::unique_ptr<Contracts::IDBRecord> readRecord(long long offset, KDB::Primitives::Type* objectType);

		void allocatePartition(unsigned long long offset, unsigned long long size);

		std::unique_ptr<Primitives::BlockDefinition> scanForBlockType(Guid typeId);
		std::unique_ptr<Contracts::IDBType> scanForTypeDefinition(const std::string& typeName);
		std::unique_ptr<Contracts::IDBType> scanForTypeDefinition(Guid typeId);
	};
}