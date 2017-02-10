//! \brief Internal declarations of the resolution level clusterings merger.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-05

#include <string>
#include <functional>  // hash

#include "internal.h"


using std::string;

// Internal types definitions --------------------------------------------------
void AggHash::add(Id id) noexcept
{
	++m_size;
	m_idsum += id;
	m_id2sum += id * id;
}

void AggHash::clear() noexcept
{
	m_size = 0;
	m_idsum = 0;
	m_id2sum = 0;
}

size_t AggHash::hash() const
{
	return std::hash<string>()(string(reinterpret_cast<const char*>(this), sizeof *this));
}

//// Internal functions definitions ----------------------------------------------
//size_t hash(const Cluster& cl)
//{
//	HashData  hdata(cl.size());
//
//	for(auto nid: cl) {
//		hdata.idsum += nid;
//		hdata.id2sum += nid * nid;
//	}
//
//	return std::hash<string>()(string(reinterpret_cast<const char*>(&hdata), sizeof hdata));
//}

