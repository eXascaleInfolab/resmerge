//! \brief The Dao of Clustering library - Robust & Fine-grained Deterministic Clustering for Large Networks
//! 	Shared [en/de]coding types and operations.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-21

#ifndef CODING_HPP
#define CODING_HPP

#include <cstdint>  // uintX_t
//#include <cstddef>  // size_t
#include <string>  // uintX_t
#include <functional>  // hash
//#include <cstring>  // memcmp
#include <type_traits>  // is_integral


namespace daoc {

using std::string;
using std::is_integral;

// Type Declarations ---------------------------------------------------
//! \brief Aggregation hash of ids
//! \pre Template types should be integral
//!
//! \tparam Id  - type of the member ids
//! \tparam AccId  - type of the accumulated Ids and accumulated squares of Ids
//! should have at least twice magnitude of the Id type (i.e. squared)
template<typename Id=uint32_t, typename AccId=uint64_t>
class AggHash {
	static_assert(is_integral<Id>::value && is_integral<AccId>::value
		&& sizeof(AccId) >= 2*sizeof(Id), "AggHash, types constraints are violated");

	// ATTENTION: type of the m_size should not be less than of m_idsum to
	// avoid gaps filled with trash on memory alignment
	AccId  m_size;  //!< Size of the container
	AccId  m_idsum;  //!< Sum of the member ids
	AccId  m_id2sum;  //!< Sum of the squared member ids
public:
    //! \brief Default constructor
	AggHash() noexcept
	: m_size(0), m_idsum(0), m_id2sum(0) {}

    //! \brief Add id to the aggregation
    //!
    //! \param id Id  - id to be included into the hash
    //! \return void
	void add(Id id) noexcept;

    //! \brief Clear/reset the aggregation
    //!
    //! \return void
	void clear() noexcept;

    //! \brief Number of the aggregated ids
    //!
    //! \return size_t  - number of the aggregated ids
	size_t size() const noexcept  { return m_size; }

    //! \brief Sum of the aggregated ids
    //!
    //! \return size_t  - sum of the aggregated ids
	size_t idsum() const noexcept  { return m_idsum; }

    //! \brief Sum of squares of the aggregated ids
    //!
    //! \return size_t  - sum of squares of the aggregated ids
	size_t id2sum() const noexcept  { return m_id2sum; }

//    //! \brief The hash is empty
//    //!
//    //! \return bool  - the hash is empty
//	bool empty() const noexcept  { return !m_size; }

    //! \brief Evaluate hash of the aggregation
    //!
    //! \return size_t  - resulting hash
	size_t hash() const;

    //! \brief Operator less
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	inline bool operator <(const AggHash& ah) const noexcept;

    //! \brief Operator less or equal
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	inline bool operator <=(const AggHash& ah) const noexcept;

    //! \brief Operator greater
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	bool operator >(const AggHash& ah) const noexcept  { return !(*this <= ah); }

    //! \brief Operator greater or equal
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	bool operator >=(const AggHash& ah) const noexcept  { return !(*this < ah); }

    //! \brief Operator equal
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	inline bool operator ==(const AggHash& ah) const noexcept;

    //! \brief Operator unequal (not equal)
    //!
    //! \param ah const AggHash&  - comparing object
    //! \return bool operator  - result of the comparison
	bool operator !=(const AggHash& ah) const noexcept  { return !(*this == ah); }
};

// Type Definitions ----------------------------------------------------
template<typename Id, typename AccId>
void AggHash<Id, AccId>::add(Id id) noexcept
{
	++m_size;
	m_idsum += id;
	m_id2sum += id * id;
}

template<typename Id, typename AccId>
void AggHash<Id, AccId>::clear() noexcept
{
	m_size = 0;
	m_idsum = 0;
	m_id2sum = 0;
}

template<typename Id, typename AccId>
size_t AggHash<Id, AccId>::hash() const
{
	// ATTENTION: requires filling with zero memory alignment trash or avoid the padding
	return std::hash<string>()(string(reinterpret_cast<const char*>(this), sizeof *this));
}

template<typename Id, typename AccId>
bool AggHash<Id, AccId>::operator <(const AggHash& ah) const noexcept
{
	return m_size < ah.m_size || (m_size == ah.m_size
		&& (m_idsum < ah.m_idsum || (m_idsum == ah.m_idsum && m_id2sum < ah.m_id2sum)));
}

template<typename Id, typename AccId>
bool AggHash<Id, AccId>::operator <=(const AggHash& ah) const noexcept
{
	return m_size < ah.m_size || (m_size == ah.m_size
		&& (m_idsum < ah.m_idsum || (m_idsum == ah.m_idsum && m_id2sum <= ah.m_id2sum)));
}

template<typename Id, typename AccId>
bool AggHash<Id, AccId>::operator ==(const AggHash& ah) const noexcept
{
	return m_size == ah.m_size && m_idsum == ah.m_idsum && m_id2sum == ah.m_id2sum;
	//return !memcmp(this, &ah, sizeof(AggHash));  // Note: memcmp returns 0 on full match
}

}  // daoc

#endif // CODING_HPP
