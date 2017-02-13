//! \brief File IO utils.
//!
//!	Interface macro
//! INCLUDE_STL_FS  - include STL filesystem library under fs namespace. This macros is
//! 	defined to avoid repetitive conditional inclusion of the STL FS.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

#ifndef FILEIO_H
#define FILEIO_H

#ifdef INCLUDE_STL_FS
#if defined(__has_include) && __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif defined(__has_include) && __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#else
	#error "STL filesystem is not available. The native alternative is not implemented."
#endif // __has_include
#endif // INCLUDE_STL_FS

#include "types.h"
#include "iotypes.h"


constexpr char  PATHSEP =
#if !defined(_WIN32)
	'/'
#else
	'\\'
#endif // _WIN32
;  // Path separator

// Accessory types definitions -------------------------------------------------

//! \brief Unordered container of NamedFileWrapper-s
using NamedFileWrappers = vector<NamedFileWrapper>;

// File I/O functions ----------------------------------------------------------
//! \brief Ensure existence of the specified directory
//!
//! \param dir const string&  - directory to be created if has not existed
//! \return void
void ensureDir(const string& dir);

//! \brief Get file size
//!
//! \param file const NamedFileWrapper&  - target file
//! \return size_t  - file size, -1 on error
size_t fileSize(const NamedFileWrapper& file) noexcept;

//! \brief  Parse the header of CNL file and validate the results
//! \post clsnum <= ndsnum if ndsnum > 0. 0 means not specified
//!
//! \param fcls NamedFileWrapper&  - the reading file
//! \param line StringBuffer&  - processing line (string, header) being read from the file
//! \param[out] clsnum size_t&  - resulting number of clusters if specified, 0 in case of parsing errors
//! \param[out] ndsnum size_t&  - resulting number of nodes if specified, 0 in case of parsing errors
//! \return void
void parseHeader(NamedFileWrapper& fcls, StringBuffer& line, size_t& clsnum, size_t& ndsnum);

//! \brief Estimate the number of nodes from the CNL file size
//!
//! \param size size_t  - the number of bytes in the CNL file
//! \param membership=1.f float  - average membership of the node,
//! 	> 0, typically ~= 1
//! \return Id  - estimated number of nodes
Id estimateNodes(size_t size, float membership=1.f) noexcept;

//! \brief Estimate the number of clusters from the number of nodes
//!
//! \param ndsnum Id - the number of nodes
//! \param membership=1.f float  - average membership of the node,
//! 	> 0, typically ~= 1
//! \return Id  - estimated number of clusters
Id estimateClusters(Id ndsnum, float membership=1.f) noexcept;

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

#endif // FILEIO_H
