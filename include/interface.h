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

#define INCLUDE_STL_FS
#include "fileio.hpp"


using namespace daoc;

// Base types ------------------------------------------------------------------
//! Node Id
using Id = uint32_t;
//constexpr Id  ID_NONE = numeric_limits<Id>::max();

//! AccId type
//! \note Larger than Id type with at least twice in magnitude
using AccId = uint64_t;

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

//! \brief Unordered container of NamedFileWrapper-s
using NamedFileWrappers = vector<NamedFileWrapper>;

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
