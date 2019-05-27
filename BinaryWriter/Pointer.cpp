#include "Pointer.h"
#include "shortcuts.h"
#include "Utilities.h"
#include "RecordType.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	void swapPointer(Pointer& lhs, Pointer& rhs) noexcept
	{
		std::swap(lhs.m_format, rhs.m_format);
		std::swap(lhs.m_address, rhs.m_address);
		std::swap(lhs.m_blockId, rhs.m_blockId);
		std::swap(lhs.m_offset, rhs.m_offset);
	}

	Pointer::Pointer()
	{
	}

	Pointer::Pointer(PointerFormat format, int64 address, Guid blockId, int64 offset)
		: m_size(0), m_format(format), m_address(address), m_blockId(blockId), m_offset(offset)
	{
		this->m_size = getSize();
	}

	Pointer::Pointer(Pointer&& other) noexcept
		: Pointer::Pointer()
	{
		swapPointer(*this, other);
	}

	Pointer& Pointer::operator=(Pointer&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		Pointer ptr(std::move(*this));
		swapPointer(*this, ptr);
		return *this;
	}

	/* Format of a "Pointer" record:
	 * 0:				 Pointer record identifier (0xA0)
	 * 1-AS:			 Address number, used in the database to refer to this pointer
	 * AS+1-BS+AS:		 Block ID, used to identify to which block this pointer refers
	 * BS+AS+1-BS+AS+OS  Offset within the block, used to calculate the final location
	 * NB: AS, BS and OS are defined in the database configuration and can vary; 
	 * their range is 2-sizeof(unsigned long long) and in any case must be a multiple of 2.
	 */
	 //Return by-value is efficient as the variable is local thanks to copy elision/RVO
	std::vector<char> Pointer::getData() const
	{
		std::vector<char> data;

		auto serGuid = m_blockId.serialize();

//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::POINTER_RECORD);
		
		//at the moment only sizes 2, 4 or 8 are supported:
		Utilities::push_varint(data, this->m_address, this->m_format.AddressSize);
		Utilities::push_vector(data, serGuid);
		Utilities::push_varint(data, this->m_offset,  this->m_format.OffsetSize);

		return data;
	}

	int Pointer::getSize() const
	{
		//we cache the size for performance
		if (this->m_size != 0)
			return this->m_size;

		auto size = 17;	//record type + block ID size (16 for a GUID)
		size += this->m_format.AddressSize;
		size += this->m_format.OffsetSize;

		return size;
	}

	unsigned long long Pointer::getAddress() const
	{
		return this->m_address;
	}

	Guid Pointer::getBlockId() const
	{
		return this->m_blockId;
	}

	unsigned long long Pointer::getOffset() const
	{
		return this->m_offset;
	}

	std::unique_ptr<Pointer> buildPointer(std::fstream& stream, const PointerFormat& format)
	{
		unsigned long long address;
		GUID blockId;
		unsigned long long offset;
		
		Utilities::read_varint(stream, &address, format.AddressSize);
		Utilities::read_GUID(stream, &blockId);
		Utilities::read_varint(stream, &offset,  format.OffsetSize);

		return std::make_unique<Pointer>(Pointer(PointerFormat(format), address, Guid(std::move(blockId)), offset));
	}

	void skipPointer(std::fstream& stream, const PointerFormat& format)
	{
		//16 is the size of a GUID
		unsigned long long total = format.AddressSize + 16 + format.OffsetSize;
		stream.ignore(total);
	}
}