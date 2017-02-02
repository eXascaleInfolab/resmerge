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

#include <vector>
#include <fstream>

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


using std::vector;

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

//! \brief Open files corresponding to the specified entries
//!
//! \param names vector<const char*>&  - file or directory names
//! \return FileWrappers  - opened files
FileWrappers openFiles(vector<const char*>& names);

#endif // FILEIO_H
