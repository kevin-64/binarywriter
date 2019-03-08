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
		std::vector<PartitionDefinition*> m_partitions;	
		int m_size;

	public:
		BlockDefinition(Guid blockId, Guid typeId, std::vector<PartitionDefinition*>&& partitions);
		virtual ~BlockDefinition() = default;

		friend void swapBlockDefinitions(BlockDefinition& lhs, BlockDefinition& rhs) noexcept;

		//move semantics are supported
		BlockDefinition(BlockDefinition&& other) noexcept;
		BlockDefinition& operator=(BlockDefinition&& rhs) noexcept;

		//no copy is supported
		BlockDefinition(const BlockDefinition&) = delete;
		BlockDefinition& operator=(const BlockDefinition&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		friend std::unique_ptr<BlockDefinition> buildBlockDefinition(std::fstream& stream);
	};

	std::unique_ptr<BlockDefinition> buildBlockDefinition(std::fstream& stream);
}