//! \brief Base types of the resolution level clusterings merger.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-05

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>
#include <unordered_set>


// Global MACROSES:
//	- HEAVY_VALIDATION  - use alternative evaluations to validate results
//		- 0  - turn off heavy validation
//		- 1  - default value for the heavy validation
//		- 2  - extra heavy validation (might duplicate already performed heavy validation)
//		- 3  - cross validation of functions (executed on each call, but only once is enough)
//
//	- TRACE, TRACE_EXTRA  - detailed tracing under debug (trace nodes weights)
//		- 0  - turn off the tracing
//		- 1  - brief tracing that can be used in release to show warnings, etc.
//		- 2  - detailed tracing for DEBUG
//		- 3  - extra detailed tracing
//
// NOTE: undefined maro definition is interpreted as having value 0

#ifndef TRACE
#ifdef DEBUG
	#define TRACE 2
#elif !defined(NDEBUG)  // RELEASE, !NDEBUG
	#define TRACE 1
//#else  // RELEASE, NDEBUG
//	#define TRACE 0
#endif // DEBUG
#endif // TRACE

#ifndef HEAVY_VALIDATION
#ifdef DEBUG
	#define HEAVY_VALIDATION 2
#elif !defined(NDEBUG)  // RELEASE, !NDEBUG
	#define HEAVY_VALIDATION 1
//#else  // ELEASE, NDEBUG
//	#define HEAVY_VALIDATION 0
#endif // DEBUG
#endif // HEAVY_VALIDATION

using std::vector;
using std::unordered_set;

// Main types declarations -----------------------------------------------------
//! Node Id
using Id = uint32_t;
//constexpr Id  ID_NONE = numeric_limits<Id>::max();
////! Ids type, unordered
//using Ids = vector<Id>;

//! Size type
//! \note Larger than Id type with at least twice in magnitude
using Size = uint64_t;

////!< Cluster, unordered container of nodes
//using Cluster = vector<Id>;

//! Hashes of the clusters
using ClusterHashes = unordered_set<size_t>;

//! Unique ids
using UniqIds = unordered_set<Id>;

////! Clusters indexed by their hash
////! \note Even in case of accidential loss of a few clusters caused by the hash
////! collision, it will not make any noticeable impact on th subsequent evaluation
//using Clusters = unordered_map<size_t, unique_ptr<Cluster>>;

//! Input file names
using FileNames = vector<const char*>;

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

//    //! \brief The hash is empty
//    //!
//    //! \return bool  - the hash is empty
//	bool empty() const noexcept  { return !m_size; }

    //! \brief Evaluate hash of the aggregation
    //!
    //! \return size_t  - resulting hash
	size_t hash() const;
};

#endif // TYPES_H
