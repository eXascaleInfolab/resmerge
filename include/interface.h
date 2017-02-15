//! \brief ResMerge interface functions
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-13

#ifndef INTERFACE_H
#define INTERFACE_H

#include <unordered_set>

#define INCLUDE_STL_FS
#include "fileio.h"


using std::unordered_set;

// Base types ------------------------------------------------------------------
//! Node Id
using Id = uint32_t;
//constexpr Id  ID_NONE = numeric_limits<Id>::max();

//! Size type
//! \note Larger than Id type with at least twice in magnitude
using Size = uint64_t;

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

// Accessory types -------------------------------------------------------------
//! Path separator
constexpr char  PATHSEP =
#if !defined(_WIN32)
	'/'
#else
	'\\'
#endif // _WIN32
;  // Path separator

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

//! \brief Unordered container of NamedFileWrapper-s
using NamedFileWrappers = vector<NamedFileWrapper>;

// Accessory functions ---------------------------------------------------------
//! \brief Load node base (unique node ids) from the CNL file optionally
//! 	filtering clusters by size
//!
//! \param file NamedFileWrapper&  - input collection of clusters in the CNL format
//! \param membership=1.f float  - average membership of the node,
//! 	> 0, typically ~= 1
//! \param cmin=0 Id  - min allowed cluster size
//! \param cmax=0 Id  - max allowed cluster size, 0 means any size
//! \return UniqIds  - resulting node bae
UniqIds loadNodes(NamedFileWrapper& file, float membership=1.f, Id cmin=0, Id cmax=0);

// Interface functions ---------------------------------------------------------
//! \brief Create output file if required
//!
//! \param outpname const string&  - name of the output file
//! \param rewrite=false bool  - whether to rewrite the file if exists
//! \return NamedFileWrapper  - resulting output file opened for writing
//! 	or nullptr if exists and should not be rewritten
NamedFileWrapper createFile(const string& outpname, bool rewrite=false);

//! \brief Open files corresponding to the specified entries
//!
//! \param names const FileNames&  - file or directory names
//! \return NamedFileWrappers  - opened files
NamedFileWrappers openFiles(const FileNames& names);

//! \brief Merge collections of clusters filtering by size retaining unique clusters
//! 	and optionally synchronizing with the node base (excluding non-listed nodes).
//! 	Typically used to flatten a hierarchy or multiple resolutions.
//! \note Clusters are unique respecting the order-independent members (node ids)
//! \pre fout should be empty
//!
//! \param fout NamedFileWrapper&  - output file for the resulting collection
//! \param files NamedFileWrappers&  - input collections
//! \param fbase NamedFileWrapper&  - input node base for the synchronization,
//! 	or an empty wrapper
//! \param cmin=0 Id  - min allowed cluster size
//! \param cmax=0 Id  - max allowed cluster size, 0 means any size
//! \param membership=1.f float  - average membership of the node,
//! 	> 0, typically ~= 1
//! \return bool  - the processing is successful
bool mergeCollections(NamedFileWrapper& fout, NamedFileWrappers& files
	, NamedFileWrapper& fbase, Id cmin=0, Id cmax=0
	, float membership=1.f);

//! \brief Extract the node base from the specified collections optionally
//! 	prefiltering clusters by size
//! \pre fout should be an empty
//!
//! \param fout NamedFileWrapper&  - output file for the resulting node base
//! \param files NamedFileWrappers&  - input collections
//! \param cmin=0 Id  - min allowed cluster size
//! \param cmax=0 Id  - max allowed cluster size, 0 means any size
//! \param membership=1.f float  - average membership of the node,
//! 	> 0, typically ~= 1
//! \return bool  - the processing is successful
bool extractBase(NamedFileWrapper& fout, NamedFileWrappers& files
	, Id cmin=0, Id cmax=0, float membership=1.f);

#endif // INTERFACE_H
