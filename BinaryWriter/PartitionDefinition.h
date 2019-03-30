#pragma once
#include "IDBRecord.h"
#include "Utilities.h"
#include "guid.h"
#include <memory>

namespace KDB::Primitives
{
	const int PARTITION_RECORD_SIZE = 22;

	/*
	* Describes the definition of a partition of a storage block of the database.
	*/
	class PartitionDefinition : public Contracts::IDBRecord
	{
	private:
		PartitionDefinition() = default;
		char m_partitionId;
		unsigned long long m_partitionSize;
		int m_fileId;
		unsigned long long m_ptrAdjOffset;
		int m_size;

	public:
		PartitionDefinition(char pid, unsigned long long psize, int fileId, unsigned long long poa);
		virtual ~PartitionDefinition() = default;

		friend void swapPartitionDefinition(PartitionDefinition& lhs, PartitionDefinition& rhs) noexcept;

		//move semantics are supported
		PartitionDefinition(PartitionDefinition&& other) noexcept;
		PartitionDefinition& operator=(PartitionDefinition&& rhs) noexcept;

		//no copy is supported
		PartitionDefinition(const PartitionDefinition&) = delete;
		PartitionDefinition& operator=(const PartitionDefinition&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		//returns the file id and the adjustment offset for this partition
		std::pair<int, unsigned long long> getPartitionCoordinates() const;
		unsigned long long getPartitionSize() const;

		friend std::unique_ptr<PartitionDefinition> buildPartitionDefinition(std::fstream& stream);
		friend void skipPartitionDefinition(std::fstream& stream);
	};

	std::unique_ptr<PartitionDefinition> buildPartitionDefinition(std::fstream& stream);
	void skipPartitionDefinition(std::fstream& stream);
}