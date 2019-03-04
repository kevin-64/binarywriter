#include "guid.h"
#include "shortcuts.h"

#include <sstream>

void swapGuid(Guid& lhs, Guid& rhs) noexcept
{
	std::swap(lhs.m_guid, rhs.m_guid);
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

unique_ptr<string> Guid::toString() const
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
		
	return std::make_unique<string>(stream.str());
}

vector<char> Guid::serialize() const
{
	vector<char> v;
	v.push_back(static_cast<char>(m_guid.Data1 & 0x000000FF));
	v.push_back(static_cast<char>((m_guid.Data1 & 0x0000FF00) >> 8));
	v.push_back(static_cast<char>((m_guid.Data1 & 0x00FF0000) >> 16));
	v.push_back(static_cast<char>((m_guid.Data1 & 0xFF000000) >> 24));

	v.push_back(static_cast<char>(m_guid.Data2 & 0x00FF));
	v.push_back(static_cast<char>((m_guid.Data2 & 0xFF00) >> 8));

	v.push_back(static_cast<char>(m_guid.Data3 & 0x00FF));
	v.push_back(static_cast<char>((m_guid.Data3 & 0xFF00) >> 8));

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

