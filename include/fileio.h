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

#include <cstdio>  // FILE
#include <string>
#include <vector>
//#include <fstream>
#include <utility>  // move

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


using std::string;
using std::vector;
using std::move;

constexpr char  PATHSEP =
#if !defined(_WIN32)
	'/'
#else
	'\\'
#endif // _WIN32
;  // Path separator

// Accessory types definitions -------------------------------------------------
//! \brief Wrapper around the FILE* to prevent hanging file descriptors
class FileWrapper {
    FILE*  m_dsc;
    bool  m_tidy;
public:
    //! \brief Constructor
    //!
    //! \param fd FILE*  - the file descriptor to be held
    //! \param cleanup=true bool  - close the file descriptor on destruction
    //! 	(typically false if stdin/out is supplied)
    FileWrapper(FILE* fd=nullptr, bool cleanup=true) noexcept
    : m_dsc(fd), m_tidy(cleanup)  {}

    //! \brief Copy constructor
    //! \note Any file descriptor should have a single owner
    FileWrapper(const FileWrapper&)=delete;

	//! \brief Move constructor
	// ATTENTION: fw.m_dsc is not set to nullptr by the default move operation
	// ATTENTION: std::vector will move their elements if the elements' move constructor
	// is noexcept, and copy otherwise (unless the copy constructor is not accessible)
    FileWrapper(FileWrapper&& fw) noexcept
    : FileWrapper(fw.m_dsc, fw.m_tidy)
    {
    	fw.m_dsc = nullptr;
    }

    //! \brief Copy assignment
    //! \note Any file descriptor should have the single owner
    FileWrapper& operator= (const FileWrapper&)=delete;

	//! \brief Move assignment
	// ATTENTION: fw.m_dsc is not set to nullptr by the default move operation
    FileWrapper& operator= (FileWrapper&& fw) noexcept
    {
    	reset(fw.m_dsc, fw.m_tidy);
    	fw.m_dsc = nullptr;
    	return *this;
    }

    //! \brief Destructor
    ~FileWrapper()  // noexcept by default
    {
        if(m_dsc && m_tidy) {
            fclose(m_dsc);
            m_dsc = nullptr;
        }
    }

    //! \brief Implicit conversion to the file descriptor
    //!
    //! \return FILE*  - self as a file descriptor
    operator FILE*() const noexcept  { return m_dsc; }

    //! \brief Reset the wrapper
    //!
    //! \param fd FILE*  - the file descriptor to be held
    //! \param cleanup=true bool  - close the file descriptor on destruction
    //! 	(typically false if stdin/out is supplied)
    //! \return void
	void reset(FILE* fd=nullptr, bool cleanup=true) noexcept
	{
        if(m_dsc && m_tidy && m_dsc != fd)
            fclose(m_dsc);
    	m_dsc = fd;
    	m_tidy = cleanup;
	}

    //! \brief Release ownership of the holding file
    //!
    //! \return FILE*  - file descriptor
    FILE* release() noexcept
    {
    	auto fd = m_dsc;
    	m_dsc = nullptr;
		return fd;
    }
};

//using FileWrappers = vector<FileWrapper>;

//! \brief Wrapper around the FILE* that holds also the filename giving ability
//! to reopen it and perform meaningful
// Note: we can't inherit from the FileWrapper because semantic of reset differs
class NamedFileWrapper {
	FileWrapper  m_file;  //!< File descriptor
	string  m_name;  //!< File name
public:
    //! \brief Constructor
    //! \pre Parent directory must exists
    //!
    //! \param filename const char*  - new file name to be opened
    //! \param mode const char*  - opening mode, the same as fopen() has
	NamedFileWrapper(const char* filename=nullptr, const char* mode=nullptr)
	: m_file(filename ? fopen(filename, mode) : nullptr)
	, m_name(filename ? filename : "")  {}

    //! \brief Copy constructor
    //! \note Any file descriptor should have a single owner
    NamedFileWrapper(const NamedFileWrapper&)=delete;

	//! \brief Move constructor
	// ATTENTION: std::vector will move their elements if the elements' move constructor
	// is noexcept, and copy otherwise (unless the copy constructor is not accessible)
    NamedFileWrapper(NamedFileWrapper&& fw) noexcept
    : m_file(move(fw.m_file)), m_name(move(fw.m_name))  {}

    //! \brief Copy assignment
    //! \note Any file descriptor should have the single owner
    NamedFileWrapper& operator= (const NamedFileWrapper&)=delete;

	//! \brief Move assignment
    NamedFileWrapper& operator= (NamedFileWrapper&& fw) noexcept
    {
    	m_file = move(fw.m_file);
    	m_name = move(fw.m_name);
    	return *this;
    }

    //! \brief File name
    //!
    //! \return const string&  - file name
    const string& name() const noexcept  { return m_name; }

    //! \brief Implicit conversion to the file descriptor
    //!
    //! \return FILE*  - file descriptor
    operator FILE*() const noexcept  { return m_file; }

    //! \brief Reopen the file under another mode
    //!
    //! \param mode const char*  - the mode of operations, the same as in fopen()
    //! \return NamedFileWrapper&  - the reopened file or closed (if can't be opened)
    NamedFileWrapper& reopen(const char* mode)
    {
		m_file.reset(freopen(nullptr, mode, m_file));  // m_name.c_str()
		return *this;
    }

    //! \brief Reset the file, closes current file and opens another one
    //! \pre Parent directory must exists
    //!
    //! \param filename const char*  - new file name to be opened
    //! \param mode const char*  - opening mode, the same as fopen() has
    //! \return NamedFileWrapper&  - the newly opened file or just the old one closed
	NamedFileWrapper& reset(const char* filename, const char* mode)
	{
		if(filename) {
			m_file.reset(fopen(filename, mode));
			m_name = filename;
		} else m_file.reset();
		return *this;
	}

    //! \brief Release ownership of the holding file
    //!
    //! \return FILE*  - file descriptor
    FILE* release() noexcept  { return m_file.release(); }
};

//! \brief Unordered container of NamedFileWrapper-s
using NamedFileWrappers = vector<NamedFileWrapper>;

//! \brief Base of the StringBuffer
using StringBufferBase = vector<char>;

//! \brief String buffer to real file by lines using c-strings
class StringBuffer: protected StringBufferBase {
	constexpr static size_t  spagesize = 4096;  // Small page size on x64

	size_t  m_cur;  //! Current position for the writing
//protected:
//	StringBufferBase::size();
public:
    //! \brief
    //! \post the allocated buffer will have size >= 2
    //!
    //! \param size=spagesize size_t  - size of the buffer
	StringBuffer(size_t size=spagesize): StringBufferBase(size), m_cur(0)
	{
		if(size <= 2)
			size = 2;
		data()[0] = 0;  // Set first element to 0
		data()[size-2] = 0;  // Set prelast element to 0
	}

    //! \brief Reset the string and it's shrink the allocated buffer
    //!
    //! \param size=spagesize size_t  - new initial size of the string buffer
    //! \return void
	void reset(size_t size=spagesize)
	{
		// Reset writing position
		m_cur = 0;
		// Reset the buffer
		resize(size);
		shrink_to_fit();  // Free reserved memory
		data()[0] = 0;  // Set first element to 0
		data()[size-2] = 0;  // Set prelast element to 0
	}

    //! \brief Read line from the file and store including the terminating '\n' symbol
    //!
    //! \param input FILE*  - processing file
    //! \return bool  - whether the following line available and the current one
    //! 	is read without any errors
	bool readline(FILE* input);

    //! \brief whether the string is empty
    //!
    //! \return bool  - the line is empty
	bool empty() const  { return !front() || front() == '\n'; }

    //! \brief C-string including '\n' if it was present in the file
	operator char*() noexcept  { return data(); }

    //! \brief Const C-string including '\n' if it was present in the file
	operator const char*() const noexcept  { return data(); }

    //! \brief Make public indexing operators
	using StringBufferBase::operator[];
	using StringBufferBase::at;
};

// Main types definitions ------------------------------------------------------
////! \brief Cluster contains of the unique nodes
//class Cluster {
//public:
//	using Members = set<Id>;
//	using IMembers = Members::iterator;
//
//private:
//	Members  m_members;  //!< Members (ids of the nodes)
//	Size  m_sum;  //!< Sum of the members
//	Size  m_sum2;  //!< Sum of the squared members
//
//protected:
//    //! \brief Update cluster statistics
//    //!
//    //! \param node Id
//    //! \return void
//	void updateStat(Id node) noexcept
//	{
//		m_sum += node;
//		m_sum2 += node * node;
//	}
//
//public:
//    //! \brief Validate the cluster
//    //! Throw an exception in case of overflow or other issues
//    //!
//    //! \return void
//	inline void validate();
//
//    //! \brief Add node to the cluster if the cluster hasn't had it
//    //!
//    //! \param node Id  - node id to be added
//    //! \return bool  - whether the node is added
//	inline bool extend(Id node);
////    //! \return pair<IMembers, bool> extend(Id node)  - iterator to the target node
////    //! 	and whether insertion was made (or the node already was present)
////	inline pair<IMembers, bool> extend(Id node);
//
////    //! \brief Add node to the cluster if the cluster hasn't had it
////    //!
////    //! \param node  - node id to be added
////    //! \param ims  - insertion hint
////    //! \return pair<IMembers, bool> extend(Id node)  - iterator to the target node
////    //! 	and whether insertion was made (or the node already was present)
////	inline pair<IMembers, bool> extend(Id node, IMembers ims);
//
//    //! \brief Operator less
//    //!
//    //! \param cl const Cluster&  - candidate cluster to be compared
//    //! \return bool  - whether this cluster less than cl
//	inline bool operator<(const Cluster& cl) const noexcept;
//};

// Will be used in the xmeasure
////! Clusters type, ordered by the cluster size
//using Clusters = set<Cluster>;

// File I/O functions ----------------------------------------------------------
//! \brief Ensure existence of the specified directory
//!
//! \param dir const string&  - directory to be created if has not existed
//! \return void
void ensureDir(const string& dir);

////! \brief
////!
////! \param fsm istream&
////! \param line string&
////! \param clsnum size_t&
////! \param ndsnum size_t&
////! \return void
//void parseHeader(istream& fsm, string& line, size_t& clsnum, size_t& ndsnum);

//! \brief  Parse the header of CNL file
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
Id estimateNodes(size_t size, float membership=1.f);

//! \brief Estimate the number of clusters from the number of nodes
//!
//! \param ndsnum Id - the number of nodes
//! \return Id  - estimated number of clusters
inline Id estimateClusters(Id ndsnum);

////! \brief Estimate zeroized values
////!
////! \param cmsbytes size_t  - the number of bytes in the file, require to estimate
////! 	the number of nodes
////! \param ndsnum size_t&  - the estimated number of nodes if 0, otherwise omit it
////! \param clsnum size_t&  - the estimated number of clusters if 0 and ndsnum is
////! 	specified, otherwise omit it
////! \return void
//void estimateSizes(size_t cmsbytes, size_t& ndsnum, size_t& clsnum);

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
//! \param names vector<const char*>&  - file or directory names
//! \return NamedFileWrappers  - opened files
NamedFileWrappers openFiles(vector<const char*>& names);

//! \brief Merge collections of clusters filtering by size retaining unique clusters.
//! 	Typically used to flatten a hierarchy or multiple resolutions.
//! \note Clusters are unique respecting the order-independent members (node ids)
//!
//! \param fout NamedFileWrapper&  - output file for the resulting collection
//! \param files NamedFileWrappers&  - input collections
//! \param cmin=0 Id  - min allowed cluster size
//! \param cmax=0 Id  - max allowed cluster size, 0 means any size
//! \return void
void mergeCollections(NamedFileWrapper& fout, NamedFileWrappers& files, Id cmin=0, Id cmax=0);

#endif // FILEIO_H
