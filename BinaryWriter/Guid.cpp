#include "guid.h"
#include "shortcuts.h"
#include "Utilities.h"

#include <sstream>

void swapGuid(Guid& lhs, Guid& rhs) noexcept
{
	std::swap(lhs.m_guid, rhs.m_guid);
}

bool operator==(const Guid& lhs, const Guid& rhs)
{
	return lhs.m_guid == rhs.m_guid;
}

Guid::Guid()
{
	auto result = CoCreateGuid(&m_guid);
	if (S_OK != result)
		throw std::exception("Could not allocate GUID. Severe error.");
}

Guid::Guid(GUID&& from)
	: m_guid(std::move(from))
{
}

Guid::Guid(Guid&& other) noexcept
	: m_guid(std::move(other.m_guid))
{}

Guid& Guid::operator=(Guid&& rhs) noexcept
{
	if (&rhs == this)
		return *this;

	Guid g(std::move(rhs));
	swapGuid(*this, g);
	return *this;
}

Guid::Guid(const Guid& from)
	: m_guid(from.m_guid)
{
}

Guid& Guid::operator=(const Guid& rhs)
{
	if (&rhs == this)
		return *this;

	this->m_guid = rhs.m_guid;
	return *this;
}
std::string Guid::toString() const
{
	std::stringstream stream;
	stream << std::hex << m_guid.Data1    << "-"
		   << std::hex << m_guid.Data2    << "-"
		   << std::hex << m_guid.Data3    << "-"
		   << std::hex << +m_guid.Data4[0]
		   << std::hex << +m_guid.Data4[1] << "-"
		   << std::hex << +m_guid.Data4[2]
		   << std::hex << +m_guid.Data4[3]
		   << std::hex << +m_guid.Data4[4]
		   << std::hex << +m_guid.Data4[5]
		   << std::hex << +m_guid.Data4[6]
		   << std::hex << +m_guid.Data4[7];
		
	return stream.str();
}

vector<char> Guid::serialize() const
{
	vector<char> v;
	
	KDB::Utilities::push_ulong(v, m_guid.Data1);

	KDB::Utilities::push_ushort(v, m_guid.Data2);
	KDB::Utilities::push_ushort(v, m_guid.Data3);

	v.push_back(m_guid.Data4[0]);
	v.push_back(m_guid.Data4[1]);
	v.push_back(m_guid.Data4[2]);
	v.push_back(m_guid.Data4[3]);
	v.push_back(m_guid.Data4[4]);
	v.push_back(m_guid.Data4[5]);
	v.push_back(m_guid.Data4[6]);
	v.push_back(m_guid.Data4[7]);

	return v;
}

Guid GuidEmpty()
{
	Guid newGuid;
	newGuid.m_guid.Data1 = 0UL;
	newGuid.m_guid.Data2 = (unsigned short)0;
	newGuid.m_guid.Data3 = (unsigned short)0;

	newGuid.m_guid.Data4[0] = (unsigned char)0;
	newGuid.m_guid.Data4[1] = (unsigned char)0;
	newGuid.m_guid.Data4[2] = (unsigned char)0;
	newGuid.m_guid.Data4[3] = (unsigned char)0;
	newGuid.m_guid.Data4[4] = (unsigned char)0;
	newGuid.m_guid.Data4[5] = (unsigned char)0;
	newGuid.m_guid.Data4[6] = (unsigned char)0;
	newGuid.m_guid.Data4[7] = (unsigned char)0;

	return newGuid;
}

