#pragma once
#include "IDBRecord.h"
#include "IDBPointer.h"
#include "Utilities.h"
#include "PointerFormat.h"
#include <memory>

namespace KDB::Primitives
{
	/*
	* Describes a pointer to a storage location in the database.
	*/
	class Pointer : public Contracts::IDBPointer
	{
	private:
		Pointer() = default;
		PointerFormat m_format;
		unsigned long long m_address;
		Contracts::PointerType m_ptrType;
		Guid m_blockId;
		unsigned long long m_offset;
		int m_size;
		bool m_complete;

	public:
		Pointer(PointerFormat fmt, unsigned long long address, Guid blockId, unsigned long long offset, Contracts::PointerType ptrType);
		Pointer(PointerFormat fmt, unsigned long long address);
		virtual ~Pointer() = default;

		friend void swapPointer(Pointer& lhs, Pointer& rhs) noexcept;

		//move semantics are supported
		Pointer(Pointer&& other) noexcept;
		Pointer& operator=(Pointer&& rhs) noexcept;

		//no copy is supported
		Pointer(const Pointer&) = delete;
		Pointer& operator=(const Pointer&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		virtual unsigned long long getAddress() const override;
		virtual Contracts::PointerType getPointerType() const override;

		Guid getBlockId() const;
		unsigned long long getOffset() const;
		bool isComplete() const;

		friend std::unique_ptr<Pointer> buildPointer(std::fstream& stream, const PointerFormat& ptrFormat, Contracts::PointerType ptrType);
		friend void skipPointer(std::fstream& stream, const PointerFormat& format);
	};

	std::unique_ptr<Pointer> buildPointer(std::fstream& stream, const PointerFormat& ptrFormat, Contracts::PointerType ptrType);
	void skipPointer(std::fstream& stream, const PointerFormat& format);
}