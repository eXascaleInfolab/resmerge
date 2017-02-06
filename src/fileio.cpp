//! \brief File IO utils.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

//#include <stdexcept>
#include <cstring>  // strtok
#include <cmath>  // sqrt
#include <cassert>
#include <system_error>

#ifdef __unix__
#include <sys/stat.h>
#endif // __unix__

#include "internal.h"
#define INCLUDE_STL_FS
#include "fileio.h"


//using std::strtok;
//using std::feof;
//using std::ferror;
using std::error_code;
//using std::overflow_error;
using fs::path;
using fs::create_directories;
using fs::is_directory;
using fs::exists;
using fs::status;
using fs::directory_iterator;

// Accessory types definitions -------------------------------------------------
bool StringBuffer::readline(FILE* input)
{
#ifdef HEAVY_VALIDATION >= 2
	assert(input && "readline(), valid file stream should be specified");
#endif // HEAVY_VALIDATION
	// Read data from file until the string is read or an error occurs
	while(fgets(data() + m_cur, size() - m_cur, input) && data()[size()-2]) {
		m_cur = size() - 1;  // Start overwriting ending '0' of the string
		resize(size() + spagesize, 0);
	}
	// Note: prelast and last elements of the buffer will be always zero
	if(feof(input) || ferror(input)) {
		if(ferror(input))
			perror("readline(), File reading error");
		return false;
	}

	return true;
}

// Main types definitions ------------------------------------------------------
//void Cluster::validate()
//{
//	if(m_sum2 < m_sum)
//		throw overflow_error("validate(), m_sum2 is overflowed\n");
//}
//
//bool Cluster::extend(Id node)
//{
//	bool added = m_members.insert(node).second;
//	if(added)
//		updateStat(node);
//	return added;
//}
//
////pair<Cluster::IMembers, bool> Cluster::extend(Id node)
////{
////	auto res = m_members.insert(node);  // Uses NRVO return value optimization
////	if(res.second)
////		updateStat(node);
////	return res;
////}
////
////pair<Cluster::IMembers, bool> Cluster::extend(Id node, IMembers ims)
////{
////	auto res = m_members.insert(ims, node);  // Uses NRVO return value optimization
////	if(res.second)
////		updateStat(node);
////	return res;
////}
//
//bool Cluster::operator<(const Cluster& cl) const noexcept
//{
//	return m_members.size() < cl.m_members.size() || (m_members.size() == cl.m_members.size()
//		&& (m_sum < cl.m_sum || (m_sum == cl.m_sum && m_sum2 < cl.m_sum2)));
//}

// File I/O functions ----------------------------------------------------------
void ensureDir(const string& dir)
{
#if TRACE >= 3
	fprintf(stderr, "ensureDir(), ensuring existence of: %s\n", dir.c_str());
#endif // TRACE
	// Check whether the output directory exists and create it otherwise
	path  outdir = dir;
	if(!exists(outdir)) {
		error_code  err;
		if(!create_directories(outdir, err))
			fputs(string("ERROR ensureDir(), target directory '").append(dir)
				.append("' can't be created: ").append(err.message())
				.append("\n").c_str(), stderr);
	} else if(!is_directory(outdir))
		fputs(string("ERROR ensureDir(), target entry '").append(dir)
			.append("' already exists as non-directory path\n").c_str(), stderr);
}

void parseHeader(FileWrapper& fcls, StringBuffer& line, size_t& clsnum, size_t& ndsnum) {
    //! Parse id value
    //! \return  - id value of 0 in case of parsing errors
	auto parseId = []() -> Id {
		char* tok = strtok(nullptr, " \t,\n");  // Note: the value can't be ended with ':'
		errno = 0;
		const auto val = strtoul(tok, nullptr, 10);
		if(errno)
			perror("WARNING parseId(), id value parsing error");
		return val;
	};

	// Process the header, which is a special initial comment
	// The target header is:  # Clusters: <cls_num>[,] Nodes: <cls_num>
	const char* const  clsmark = "clusters";
	const char* const  ndsmark = "nodes";
	while(line.readline(fcls)) {
		// Skip empty lines
		if(line.empty())
			continue;
		// Consider only subsequent comments
		if(line[0] != '#')
			break;

		// Tokenize the line
		char *tok = strtok(line + 1, " \t:,\n");  // Note: +1 to skip the leading '#'
		// Skip comment without the string continuation and continuous comment
		if(!tok || tok[0] == '#')
			continue;
		do {
			// Lowercase the token
			for(char* pos = tok; *pos; ++pos)
				*pos = tolower(*pos);

			// Identify the attribute and read it's value
			if(!strcmp(tok, clsmark)) {
				clsnum = parseId();
			} else if(!strcmp(tok, clsmark)) {
				ndsnum = parseId();
			} else {
				fprintf(stderr, "WARNING parseHeader(), the header parsing is omitted"
					" because of the unknown attribute: %s\n", tok);
				break;
			}
		} while(tok = strtok(nullptr, " \t:,\n"));

		// Validate the parsed values
		if(clsnum > ndsnum) {
			fprintf(stderr, "WARNING parseHeader(), clsnum (%lu) typically should be"
				" less than ndsnum (%lu)\n", clsnum, ndsnum);
			//assert(0 && "parseHeader(), clsnum typically should be less than ndsnum");
		}
		// Get following line for the unified subsequent processing
		line.readline(fcls);
		break;
	}
}

Id estimateNodes(size_t size)
{
	Id  ndsnum = 0;  // The estimated number of nodes
	if(size) {
		size_t  magn = 10;  // Decimal ids magnitude
		unsigned  img = 1;  // Index of the magnitude (10^1)
		size_t  reminder = size % magn;  // Reminder in bytes
		ndsnum = reminder / ++img;  //  img digits + 1 delimiter for each element
		while(size >= magn) {
			magn *= 10;
			ndsnum += (size - reminder) % magn / ++img;
			reminder = size % magn;
		}
	}
	return ndsnum;
}

Id estimateClusters(Id ndsnum)
{
	Id  clsnum = 0;  // The estimated number of clusters
	// Usually the number of clusters does not increase square root of the number of nodes
	// Note: do not estimate in case the number of nodes is not specified
	if(ndsnum)
		clsnum = sqrt(ndsnum) + 1;  // Note: +1 to consider rounding down
	return clsnum;
}

//void estimateSizes(size_t cmsbytes, size_t& ndsnum, size_t& clsnum)
//{
//	if(!ndsnum && cmsbytes) {
//		size_t  magn = 10;  // Decimal ids magnitude
//		unsigned  img = 1;  // Index of the magnitude (10^1)
//		size_t  reminder = cmsbytes % magn;  // Reminder in bytes
//		ndsnum = reminder / ++img;  //  img digits + 1 delimiter for each element
//		while(cmsbytes >= magn) {
//			magn *= 10;
//			ndsnum += (cmsbytes - reminder) % magn / ++img;
//			reminder = cmsbytes % magn;
//		}
//	}
//
//	// Usually the number of clusters does not increase square root of the number of nodes
//	// Note: do not estimate in case the number of nodes is not specified
//	if(!clsnum && ndsnum)
//		clsnum = sqrt(ndsnum) + 1;  // Note: +1 to consider rounding down
//}

// Interface functions definitions ---------------------------------------------
FileWrapper createFile(const string& outpname, bool rewrite)
{
//	ensureDir();

	return FileWrapper();
}
//	// Check whether the output already exists
//	if(exists(outpname)) {
//		fprintf(stderr, "WARNING, the output file '%s' already exists, rewrite it: %i\n"
//			, outpname.c_str(), args_info.rewrite_flag);
//		if(!args_info.rewrite_flag)
//			return 1;
//	}
//
//	FileWrapper  fout = fopen(outpname.path().c_str(), "w");
//	if(!fout)
//		perror(("ERROR, can't open the output file " + outpname).c_str());

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
				else perror(string("WARNING openFiles(), can't open ").append(detry.path()).c_str());
			}
		} else {
			FILE* fd = fopen(npath.c_str(), "r");
			if(fd)
				files.emplace_back(fd);
			else perror(string("WARNING openFiles(), can't open ").append(npath).c_str());
		}
	}

	if(!unexisting.empty()) {
		fprintf(stderr, "WARNING openFiles(), %lu of %lu file system entries do not exist: "
			, unexisting.size(), names.size());
		for(auto name: unexisting)
			fprintf(stderr, "  %s\n", name);
	}

#if TRACE >= 2
	printf("# openFiles(), opened %lu entries of %lu\n", files.size(), names.size());
#endif // TRACE

	return files;
}

void mergeClusters(FileWrapper& fout, FileWrappers& files, Id cmin, Id cmax)
{
	if(!fout)
		return;

	ClusterHashes  procls;  // Processed clusters
//	Clusters  cls;  // Use NRVO - named return value optimization
//	Clusters  remcs;  // Clusters postponed for the removement in case their nodes appear in cls

	for(auto& file: files) {
		// Note: CNL [CSN] format only is supported
		StringBuffer  line;
		size_t  clsnum = 0;  // The number of clusters
		size_t  ndsnum = 0;  // The number of nodes
		parseHeader(file, line, clsnum, ndsnum);
		// Validate and correct the number of clusters if required
		// Note: it's better to reallocate a few times than too much overconsume the memory
		if(ndsnum && clsnum > ndsnum)
			clsnum = ndsnum;

		if(!clsnum) {
			bool  estimated = false;  // Whether the number of nodes/clusters is estimated
			size_t  cmsbytes = 0;
			if(!ndsnum) {
#ifdef __unix__  // sqrt(cmsbytes) lines => linebuf = max(4-8Kb, sqrt(cmsbytes) * 2) with dynamic realloc
				struct stat  filest;
				int fd = fileno(file);
				if(fd != -1 && !fstat(fd, &filest))
					cmsbytes = filest.st_size;
				//fprintf(stderr, "# %s: %lu bytes\n", fname, cmsbytes);
#endif // __unix
				// Get length of the file
				if(!cmsbytes) {
					fseek(file, 0, SEEK_END);
					cmsbytes = ftell(file);  // The number of bytes in the input communities
					fseek(file, 0, SEEK_SET);
				}
				if(cmsbytes && cmsbytes != size_t(-1)) {  // File length fetching failed
					ndsnum = estimateNodes(cmsbytes);
					estimated = true;
				}
			}
			clsnum = estimateClusters(ndsnum);
#if TRACE >= 2
			fprintf(stderr, "# loadClusters(), %lu bytes, %lu nodes (estimated: %u)"
				", %lu clusters\n", cmsbytes, ndsnum, estimated, clsnum);
#endif // TRACE
		}
#if TRACE >= 2
		fprintf(stderr, "# loadClusters(),  %lu clusters\n", clsnum);
#endif // TRACE

		// Preallocate space for the clusters
//		if(procls.bucket_count() < clsnum)
//			;

		// Load clusters

//		//fprintf(stderr, "# %lu clusters, %lu nodes\n", clsnum, ndsnum);
//		if(clsnum || ndsnum) {
//			const size_t  rsvsize = ndsnum + clsnum;  // Note: bimap has the same size of both sides
//#if TRACE >= 2
//			fprintf(stderr, "# loadClusters(), preallocating"
//				" %lu (%lu, %lu) elements, estimated: %u\n", rsvsize, ndsnum, clsnum, estimated);
//#endif // TRACE
//			inp_interf.reserve_vertices_modules(rsvsize, rsvsize);
//		}
//
//		size_t iline = 0;  // Payload line index (internal id of the cluster)
//		size_t members = 0;  // Evaluate the actual number of members (nodes including repetitions)
//		do {
//			char *tok = strtok(const_cast<char*>(line.data()), " \t");
//
//			// Skip comments
//			// Note: Boost bimap of multiset does not support .reserve(),
//			// but has rehash()
//			// so do not look for the header
//			if(!tok || tok[0] == '#')
//				continue;
//			++iline;  // Start modules (clusters) id from 1
//			// Skip the cluster id if present
//			if(tok[strlen(tok) - 1] == '>') {
//				tok = strtok(nullptr, " \t");
//				// Skip empty clusters
//				if(!tok)
//					continue;
//			}
//			do {
//				// Note: this algorithm does not support fuzzy overlaps (nodes with defined shares),
//				// the share part is skipped if exists
//				inp_interf.add_vertex_module(stoul(tok), iline);
//				// Note: the number of nodes can't be evaluated here simply incrementing the value,
//				// because clusters might have overlaps, i.e. the nodes might have multiple membership
//				++members;
//			} while((tok = strtok(nullptr, " \t")));
//		} while(getline(input, line));
//
//		// Rehash the nodes decreasing the allocated space and number of buckets
//		// for the faster iterating if required
//		inp_interf.shrink_to_fit_modules();
	}
}
