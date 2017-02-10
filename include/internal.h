//! \brief Internal declarations of the resolution level clusterings merger.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-05

#ifndef INTERNAL_H
#define INTERNAL_H

#include "types.h"


// Internal types --------------------------------------------------------------
//! \brief Aggregation hash of ids
class AggHash {
	Size  m_size;  //!< Size of the container
	Size  m_idsum;  //!<  Sum of the members
	Size  m_id2sum;  //!< Sum of the squared members
public:
    //! \brief Default constructor
	AggHash() noexcept
	: m_size(0), m_idsum(0), m_id2sum(0)  {}

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

    //! \brief Evaluate hash of the aggregation
    //!
    //! \return size_t  - resulting hash
	size_t hash() const;
};

////! HashData of the container
//struct HashData {
//	Size  size;  //!< Size of the container
//	Size  idsum;  //!<  Sum of the members
//	Size  id2sum;  //!< Sum of the squared members
//
//	HashData(Size num=0): size(num), idsum(0), id2sum(0)  {}
//};
//
//// Internal functions ----------------------------------------------------------
////! \brief Hash of the cluster
////!
////! \param cl const Cluster&  - the cluster to be hashed
////! \return size_t  - resulting hash
//size_t hash(const Cluster& cl);

#endif // INTERNAL_H
