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
//#include <fstream>

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
//using std::pair;

// Accessory types definitions -------------------------------------------------
//! \brief Wrapper around the FILE* to prevent hanging file descriptors
class FileWrapper {
    FILE*  dsc;
    bool  tidy;
public:
    //! \brief Constructor
    //! \param fd FILE*  - the file descriptor to be held
    //! \param cleanup=true bool  - close the file descriptor on destruction
    //! 	(typically false if stdin/out is supplied)
    FileWrapper(FILE* fd=nullptr, bool cleanup=true) noexcept
    : dsc(fd), tidy(cleanup)  {}

    //! \brief Copy constructor
    //! \note Any file descriptor should have a single owner
    FileWrapper(const FileWrapper&)=delete;

	//! \brief Move constructor
	// ATTENTION: fw.dsc is not set to nullptr by the default move operation
	// ATTENTION:  std::vector will move their elements if the elements' move constructor is noexcept, and copy otherwise (unless the copy constructor is not accessible)
    FileWrapper(FileWrapper&& fw) noexcept
    : FileWrapper(fw.dsc, fw.tidy)
    {
    	fw.dsc = nullptr;
    }

    //! \brief Copy assignment
    //! \note Any file descriptor should have the single owner
    FileWrapper& operator= (const FileWrapper&)=delete;

	//! \brief Move assignment
	// ATTENTION: fw.dsc is not set to nullptr by the default move operation
    FileWrapper& operator= (FileWrapper&& fw) noexcept
    {
    	reset(fw.dsc, fw.tidy);
    	fw.dsc = nullptr;
    	return *this;
    }

    //! \brief Destructor
    ~FileWrapper()  // noexcept by default
    {
        if(dsc && tidy) {
            fclose(dsc);
            dsc = nullptr;
        }
    }

    //! \brief Implicit conversion to the file descriptor
    //!
    //! \return FILE*  - self as a file descriptor
    operator FILE*() const noexcept  { return dsc; }

    //! \brief Reset the wrapper
    //!
    //! \param fd FILE*  - the file descriptor to be held
    //! \param cleanup=true bool  - close the file descriptor on destruction
    //! 	(typically false if stdin/out is supplied)
    //! \return void
	void reset(FILE* fd=nullptr, bool cleanup=true) noexcept
	{
        if(dsc && tidy)
            fclose(dsc);
    	dsc = fd;
    	tidy = cleanup;
	}

    //! \brief Release ownership of the holding file
    //!
    //! \return FILE*  - file descriptor
    FILE* release() noexcept
    {
    	auto fd = dsc;
    	dsc = nullptr;
		return fd;
    }
};

using FileWrappers = vector<FileWrapper>;

using StringBase = vector<char>;

//! \brief String buffer to real file by lines using c-strings
class StringBuffer: public StringBase {
	constexpr static size_t  spagesize = 4096;

	size_t  m_cur;  //! Current position for the writing
public:
    //! \brief
    //! \post the allocated buffer will have size >= 2
    //!
    //! \param size=spagesize size_t  - size of the buffer
	StringBuffer(size_t size=spagesize): StringBase(size), m_cur(0)
	{
		if(size <= 2)
			size = 2;
		data()[0] = 0;
		data()[size-2] = 0;  // Set prelast element to 0
	}

	void reset(size_t size=spagesize)
	{
		// Reset writing position
		m_cur = 0;
		// Reset the buffer
		resize(size);
		shrink_to_fit();  // Free reserved memory
		data()[0] = 0;
		data()[size-2] = 0;  // Set prelast element to 0
	}

	bool readline(FILE* input);

    //! \brief whether the string is empty
    //!
    //! \return bool  - the line is empty
	bool empty() const  { return !front() || front() == '\n'; }

    //! \brief C-string including '\n' if it was present in the file
	operator char*() noexcept  { return data(); }

    //! \brief Const C-string including '\n' if it was present in the file
	operator const char*() const noexcept  { return data(); }
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
//! \param fcls FileWrapper&  - the reading file
//! \param line StringBuffer&  - processing line (string, header) being read from the file
//! \param[out] clsnum size_t&  - resulting number of clusters if specified, 0 in case of parsing errors
//! \param[out] ndsnum size_t&  - resulting number of nodes if specified, 0 in case of parsing errors
//! \return void
void parseHeader(FileWrapper& fcls, StringBuffer& line, size_t& clsnum, size_t& ndsnum);

//! \brief Estimate the number of nodes from the CNL file size
//!
//! \param size size_t  - the number of bytes in the CNL file
//! \return Id  - estimated number of nodes
Id estimateNodes(size_t size);

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
//! \return FileWrapper  - resulting output file opened for writing
//! 	or nullptr if exists and should not be rewritten
FileWrapper createFile(const string& outpname, bool rewrite=false);

//! \brief Open files corresponding to the specified entries
//!
//! \param names vector<const char*>&  - file or directory names
//! \return FileWrappers  - opened files
FileWrappers openFiles(vector<const char*>& names);

void mergeClusters(FileWrapper& fout, FileWrappers& files, Id cmin=0, Id cmax=0);

#endif // FILEIO_H
