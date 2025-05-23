#pragma once
#include "IDBRecord.h"
#include "Utilities.h"
#include "PartitionDefinition.h"
#include "guid.h"
#include <memory>

namespace KDB::Primitives
{
	/*
	* Describes the definition of a storage block of the database.
	*/
	class BlockDefinition : public Contracts::IDBRecord
	{
	private:
		BlockDefinition() = default;
		Guid m_blockId;
		Guid m_mainTypeId;
		unsigned long long m_blockOffset;
		std::vector<PartitionDefinition*> m_partitions;	
		int m_size;

	public:
		BlockDefinition(Guid blockId, unsigned long long offset, Guid typeId, std::vector<PartitionDefinition*>&& partitions);
		virtual ~BlockDefinition();

		friend void swapBlockDefinitions(BlockDefinition& lhs, BlockDefinition& rhs) noexcept;

		//move semantics are supported
		BlockDefinition(BlockDefinition&& other) noexcept;
		BlockDefinition& operator=(BlockDefinition&& rhs) noexcept;

		//no copy is supported
		BlockDefinition(const BlockDefinition&) = delete;
		BlockDefinition& operator=(const BlockDefinition&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		Guid getTypeId() const;
		Guid getBlockId() const;

		//returns the block offset and an available partition for this block
		std::pair<unsigned long long, const PartitionDefinition*> getPartitionForWrite() const;

		//returns the file ID and the offset within that file at which the record with the given offset can be found
		std::pair<int, unsigned long long> getOffsetForRecord(unsigned long long recordOffset);

		friend std::unique_ptr<BlockDefinition> buildBlockDefinition(std::fstream& stream);
		friend void skipBlockDefinition(std::fstream& stream);
	};

	std::unique_ptr<BlockDefinition> buildBlockDefinition(std::fstream& stream);
	void skipBlockDefinition(std::fstream& stream);
}