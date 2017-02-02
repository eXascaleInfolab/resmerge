//! \brief File IO utils.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

#include <cstdio>
#include <cassert>

#define INCLUDE_STL_FS
#include "fileio.h"


using fs::is_directory;
using fs::exists;
using fs::status;
using fs::directory_iterator;

FileWrappers openFiles(vector<const char*>& names)
{
	FileWrappers files;  // NRVO (Return Value Optimization) is used

	assert(names.empty() && "openFiles(), entry names are expected");
	vector<const char*>  unexisting;
	for(auto name: names) {
		const auto  npath = fs::path(name);
		const auto  nstat = status(npath);
		// Check whether the file system entry exists
		if(!exists(nstat)) {
			unexisting.push_back(name);
			continue;
		}
		// Fetch all files under the specified directories (single level traversing)
		if(is_directory(nstat)) {
			for(const auto& detry: directory_iterator(npath)) {
				if(is_directory(detry))
					continue;
				FILE* fd = fopen(detry.path().c_str(), "r");
				if(fd)
					files.emplace_back(fd);
			}
		} else {
			FILE* fd = fopen(npath.c_str(), "r");
			if(fd)
				files.emplace_back(fd);
		}
	}

	if(!unexisting.empty()) {
		if(unexisting.size() == names.size())
			fputs("WARNING openFiles(), all specified file system entries do not exist\n", stderr);
		fputs("WARNING openFiles(), the file system entries do not exist: ", stderr);
		for(auto name: unexisting)
			fprintf(stderr, "  %s\n", name);
	}

#ifdef DEBUG
	printf("> openFiles(), opened %lu files\n", files.size());
#endif // DEBUG

	return files;
}
