#include "PartitionDefinition.h"
#include "RecordType.h"
#include "shortcuts.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	void swapPartitionDefinition(PartitionDefinition& lhs, PartitionDefinition& rhs) noexcept
	{
		std::swap(lhs.m_fileId, rhs.m_fileId);
		std::swap(lhs.m_partitionId, rhs.m_partitionId);
		std::swap(lhs.m_partitionSize, rhs.m_partitionSize);
		std::swap(lhs.m_ptrAdjOffset, rhs.m_ptrAdjOffset);
		std::swap(lhs.m_size, rhs.m_size);
	}

	PartitionDefinition::PartitionDefinition(char pid, unsigned long long psize, int fileId, unsigned long long poa)
		: m_partitionId(pid), m_partitionSize(psize), m_fileId(fileId), m_ptrAdjOffset(poa), m_size(0)
	{
		m_size = getSize();
	}

	PartitionDefinition::PartitionDefinition(PartitionDefinition&& other) noexcept
		: PartitionDefinition::PartitionDefinition()
	{
		swapPartitionDefinition(*this, other);
	}

	PartitionDefinition& PartitionDefinition::operator=(PartitionDefinition&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		PartitionDefinition partDef(std::move(rhs));
		swapPartitionDefinition(*this, partDef);
		return *this;
	}

	/* Format of a "PartitionDefinition" record:
	 * 0:	     PartitionDefinition record identifier (0xB1)
	 * 1:		 Partition ID (PID) for this partition within its block
	 * 2-9:		 Size of the entire partition in its file (in bytes)
	 * 10-13:	 File ID (FID) of the storage file (ks...) that contains this partition
	 * 14-21:	 Pointer Offset Adjustment (POA) for this partition; avoids the need to alter pointers when
	 *			 a block is partitioned
	 */
	 //Return by-value is efficient as the variable is local thanks to copy elision/RVO
	std::vector<char> PartitionDefinition::getData() const
	{
		std::vector<char> data;

//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::BLOCK_PARTITION);
		Utilities::push_char(data, m_partitionId);
		Utilities::push_int64(data, m_partitionSize);
		Utilities::push_int(data, m_fileId);
		Utilities::push_int64(data, m_ptrAdjOffset);

		return data;
	}

	int PartitionDefinition::getSize() const
	{
		return PARTITION_RECORD_SIZE; //fixed as there are no variable-length fields
	}

	unsigned long long PartitionDefinition::getPartitionSize() const
	{
		return this->m_partitionSize;
	}

	std::pair<int, unsigned long long> PartitionDefinition::getPartitionCoordinates() const
	{
		//TODO: #fulprt handle full partitions to avoid overwrites
		return std::make_pair(this->m_fileId - 1, this->m_ptrAdjOffset);
	}

	std::unique_ptr<PartitionDefinition> buildPartitionDefinition(std::fstream& stream)
	{
		char partitionId;
		Utilities::read_char(stream, &partitionId);

		int64 partitionSize;
		Utilities::read_int64(stream, &partitionSize);

		int fileId;
		Utilities::read_int(stream, &fileId);

		int64 ptrAdjOffset;
		Utilities::read_int64(stream, &ptrAdjOffset);

		return std::make_unique<PartitionDefinition>(PartitionDefinition(partitionId, partitionSize, fileId, ptrAdjOffset));
	}

	void skipPartitionDefinition(std::fstream& stream)
	{
		stream.ignore(PARTITION_RECORD_SIZE - 1);
	}
}