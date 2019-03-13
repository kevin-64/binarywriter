#include "BlockDefinition.h"
#include "PartitionDefinition.h"
#include "Utilities.h"
#include "shortcuts.h"
#include "RecordType.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	const char END_OF_BLOCK = 0xBF;

	void swapBlockDefinitions(BlockDefinition& lhs, BlockDefinition& rhs) noexcept
	{
		std::swap(lhs.m_blockId, rhs.m_blockId);
		std::swap(lhs.m_mainTypeId, rhs.m_mainTypeId);
		std::swap(lhs.m_partitions, rhs.m_partitions);
		std::swap(lhs.m_size, rhs.m_size);
	}

	BlockDefinition::BlockDefinition(Guid blockId, Guid typeId, std::vector<PartitionDefinition*>&& partitions)
		: m_blockId(std::move(blockId)), m_mainTypeId(std::move(typeId)), m_partitions(std::move(partitions)), m_size(0)
	{
		m_size = getSize();
	}

	BlockDefinition::~BlockDefinition()
	{
		for (auto it = begin(m_partitions); it != end(m_partitions); it++)
		{
			delete *it;
		}
	}

	BlockDefinition::BlockDefinition(BlockDefinition&& other) noexcept
		: BlockDefinition::BlockDefinition()
	{
		swapBlockDefinitions(*this, other);
	}

	BlockDefinition& BlockDefinition::operator=(BlockDefinition&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		BlockDefinition blockDef(std::move(rhs));
		swapBlockDefinitions(*this, blockDef);
		return *this;
	}

	/* Format of a "BlockDefinition" record:
	 * 0:	     BlockDefinition record identifier (0xB0)
	 * 1-16:	 Block ID (BID) for this block
	 * 17-32:	 Type ID (TID) for the main user-defined type contained in this block
	 * 33-...	 Partition definition(s), at least 1
	 */
	 //Return by-value is efficient as the variable is local thanks to copy elision/RVO
	std::vector<char> BlockDefinition::getData() const
	{
		std::vector<char> data;

//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::BLOCK_DEFINITION);

		auto blockId = m_blockId.serialize();
		Utilities::push_vector(data, blockId);

		auto typeId = m_mainTypeId.serialize();
		Utilities::push_vector(data, typeId);

		//write each partition as a separate record
		for (auto p : m_partitions)
		{
			auto part = p->getData();
			Utilities::push_vector(data, part);
		}

		data.push_back(END_OF_BLOCK);
		return data;
	}

	int BlockDefinition::getSize() const
	{
		//size is cached for performance
		if (this->m_size != 0)
			return this->m_size;

		auto size = 34; //record identifier, block ID and type ID, plus an "end of block" identifier
		size += (this->m_partitions.size() * this->m_partitions.at(0)->getSize()); //partition def. size is fixed

		return size;
	}

	Guid BlockDefinition::getTypeId() const
	{
		return this->m_mainTypeId;
	}

	std::unique_ptr<BlockDefinition> buildBlockDefinition(std::fstream& stream)
	{
		GUID guid;
		Utilities::read_GUID(stream, &guid);
		Guid blockId(std::move(guid));

		Utilities::read_GUID(stream, &guid);
		Guid typeId(std::move(guid));
		
		std::vector<PartitionDefinition*> partitions;

		//read the partitions as separate records
		while (true)
		{
			char next;
			stream.read(&next, 1);

			if (next == END_OF_BLOCK)
				break;
				
			auto part = buildPartitionDefinition(stream);
			partitions.push_back(part.release());
		}

		return std::make_unique<BlockDefinition>(BlockDefinition(std::move(blockId), std::move(typeId), std::move(partitions)));
	}
}