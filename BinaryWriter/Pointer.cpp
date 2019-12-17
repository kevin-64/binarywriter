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
		std::swap(lhs.m_ptrType, rhs.m_ptrType);
		std::swap(lhs.m_complete, rhs.m_complete);
	}

	//Complete constructor: used when the pointer is created and needs to be stored
	Pointer::Pointer(PointerFormat format, int64 address, Guid blockId, int64 offset, Contracts::PointerType ptrType)
		: m_size(0), m_format(format), m_address(address), m_blockId(blockId), m_offset(offset), m_ptrType(ptrType), 
		  m_complete(true)
	{
		this->m_size = getSize();
	}

	//Incomplete constructor: used when the pointer needs to be passed to clients
	Pointer::Pointer(PointerFormat format, int64 address)
		: m_size(0), m_format(format), m_address(address), m_blockId(GuidEmpty()), m_offset(0), 
		  m_ptrType(Contracts::PointerType::Unspecified), m_complete(false)
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
		data.push_back((unsigned char)(this->m_ptrType));
		
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

	Contracts::PointerType Pointer::getPointerType() const
	{
		return this->m_ptrType;
	}

	Guid Pointer::getBlockId() const
	{
		if (m_complete)
			return this->m_blockId;
		throw new std::runtime_error("Attempted to dereference incomplete pointer.");
	}

	unsigned long long Pointer::getOffset() const
	{
		if (m_complete)
			return this->m_offset;
		throw new std::runtime_error("Attempted to dereference incomplete pointer.");
	}

	bool Pointer::isComplete() const
	{
		return m_complete;
	}

	std::unique_ptr<Pointer> buildPointer(std::fstream& stream, const PointerFormat& format, Contracts::PointerType ptrType)
	{
		unsigned long long address;
		GUID blockId;
		unsigned long long offset;
		
		Utilities::read_varint(stream, &address, format.AddressSize);
		Utilities::read_GUID(stream, &blockId);
		Utilities::read_varint(stream, &offset,  format.OffsetSize);

		return std::make_unique<Pointer>(Pointer(PointerFormat(format), address, Guid(std::move(blockId)), offset, ptrType));
	}

	void skipPointer(std::fstream& stream, const PointerFormat& format)
	{
		//16 is the size of a GUID
		unsigned long long total = 16ULL + (unsigned long long)(format.AddressSize) + (unsigned long long)(format.OffsetSize);
		stream.ignore(total);
	}
}