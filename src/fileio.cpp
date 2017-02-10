//! \brief File IO utils.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

#include <cstring>  // strtok
#include <cmath>  // sqrt
#include <cassert>
#include <system_error>  // error_code
#include <stdexcept>
#include <limits>

#ifdef __unix__
#include <sys/stat.h>
#endif // __unix__

#include "internal.h"
#define INCLUDE_STL_FS
#include "fileio.h"


using std::error_code;
using std::invalid_argument;
using std::numeric_limits;
using std::to_string;
using fs::path;
using fs::create_directories;
using fs::is_directory;
using fs::exists;
using fs::status;
using fs::directory_iterator;

// Accessory types definitions -------------------------------------------------
bool StringBuffer::readline(FILE* input)
{
#if HEAVY_VALIDATION >= 2
	assert(input && "readline(), valid file stream should be specified");
#endif // HEAVY_VALIDATION
	// Read data from file until the string is read or an error occurs
	while(fgets(data() + m_cur, size() - m_cur, input) && data()[size()-2]) {
#if TRACE >= 3  // Verified
		fprintf(stderr, "readline(), resizing buffer of %lu bytes, %lu pos: %s\n"
			, size(), m_cur, data());
#endif // TRACE
		m_cur = size() - 1;  // Start overwriting ending '0' of the string
		resize(size() + spagesize, 0);
	}
#if HEAVY_VALIDATION >= 2
	assert((!m_cur || strlen(data()) >= m_cur) && "readline(), string size validation failed");
#endif // HEAVY_VALIDATION
	m_cur = 0;  // Reset the writing (appending) position
	// Note: prelast and last elements of the buffer will be always zero

	// Check for errors
	if(feof(input) || ferror(input)) {
		if(ferror(input))
			perror("readline(), file reading error");
		return false;  // No more lines can be read
	}

	return true;  // More lines can be read
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

void parseHeader(NamedFileWrapper& fcls, StringBuffer& line, size_t& clsnum, size_t& ndsnum) {
    //! Parse id value
    //! \return  - id value of 0 in case of parsing errors
	auto parseId = []() -> Id {
		char* tok = strtok(nullptr, " \t,");  // Note: the value can't be ended with ':'
		errno = 0;
		const auto val = strtoul(tok, nullptr, 10);
		if(errno)
			perror("WARNING parseId(), id value parsing error");
		return val;
	};

	// Process the header, which is a special initial comment
	// The target header is:  # Clusters: <cls_num>[,] Nodes: <cls_num>
	constexpr char  clsmark[] = "clusters";
	constexpr char  ndsmark[] = "nodes";
	constexpr char  attrnameDelim[] = " \t:,";
	while(line.readline(fcls)) {
		// Skip empty lines
		if(line.empty())
			continue;
		// Consider only subsequent comments
		if(line[0] != '#')
			break;

		// Tokenize the line
		char *tok = strtok(line + 1, attrnameDelim);  // Note: +1 to skip the leading '#'
		// Skip comment without the string continuation and continuous comment
		if(!tok || tok[0] == '#')
			continue;
		uint8_t  attrs = 0;  // The number of read attributes
		do {
			// Lowercase the token
			for(char* pos = tok; *pos; ++pos)
				*pos = tolower(*pos);

			// Identify the attribute and read it's value
			if(!strcmp(tok, clsmark)) {
				clsnum = parseId();
				++attrs;
			} else if(!strcmp(tok, ndsmark)) {
				ndsnum = parseId();
				++attrs;
			} else {
				fprintf(stderr, "WARNING parseHeader(), the header parsing is omitted"
					" because of the unexpected attribute: %s\n", tok);
				break;
			}
		} while((tok = strtok(nullptr, attrnameDelim)) && attrs < 2);

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

Id estimateNodes(size_t size, float membership)
{
	if(membership <= 0)
		throw invalid_argument("estimateNodes(), membership = "
			+ to_string(membership) + " should be positive\n");
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
	return ndsnum / membership;
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

// Interface functions definitions ---------------------------------------------
NamedFileWrapper createFile(const string& outpname, bool rewrite)
{
	NamedFileWrapper  fout;  // Use NRVO optimization
	// Check whether the output already exists
	if(exists(outpname)) {
		fprintf(stderr, "WARNING createFile(), the output file '%s' already exists, rewrite it: %u\n"
			, outpname.c_str(), rewrite);
		if(!rewrite)
			return fout;
	} else {
		// Ensure that target directory exists
		auto  idir = outpname.rfind(PATHSEP);
		if(idir != string::npos) {
			// Validate that filename is not a dirname
			if(idir == outpname.size()-1)
				throw invalid_argument("createFile(), a file name is expected: " + outpname);
			ensureDir(outpname.substr(0, idir));
		}
	}

	if(!fout.reset(outpname.c_str(), "w")) {
		const char*  msg = errno ? strerror(errno) : "\n";
		if(errno)
			//perror("ERROR createFile(), the output file '" + outpname + "' can't be created\n");
			throw std::ios_base::failure("ERROR createFile(), the output file '" + outpname
				 + "' can't be created: " + msg);
	}
	return fout;
}

NamedFileWrappers openFiles(vector<const char*>& names)
{
	NamedFileWrappers files;  // NRVO (Return Value Optimization) is used

	assert(!names.empty() && "openFiles(), entry names are expected");
	vector<const char*>  unexisting;  // Unexisting entries
#if TRACE >= 1
	Id  inpfiles = 0;  // The number of input files
	Id  inpdirs = 0;  // The number of input dirs
#endif // TRACE
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
				NamedFileWrapper  finp(detry.path().c_str(), "r");
				if(finp)
					files.push_back(move(finp));
				else perror((string("WARNING openFiles(), can't open ") += detry.path()).c_str());
			}
		} else {
			NamedFileWrapper  finp(npath.c_str(), "r");
			if(finp)
				files.push_back(move(finp));
			else perror((string("WARNING openFiles(), can't open ") += npath).c_str());
		}
	}

	if(!unexisting.empty()) {
		fprintf(stderr, "WARNING openFiles(), %lu of %lu file system entries do not exist: "
			, unexisting.size(), names.size());
		for(auto name: unexisting)
			fprintf(stderr, "  %s\n", name);
	}

#if TRACE >= 1
	printf("openFiles(), opened %lu files from the %u files and %u dirs\n"
		, files.size(), inpfiles, inpdirs);
#endif // TRACE

	return files;
}

void mergeCollections(NamedFileWrapper& fout, NamedFileWrappers& files, Id cmin, Id cmax)
{
	if(!fout)
		return;

	// Write a stub header the actual values of clusters and nodes will be later,
	// so now just use stubs (' ' of the sufficient length) for them.
	string  idvalStub = string(ceil(numeric_limits<Id>::digits10), ' ');
	const string  hdrprefix = "# Clusters: ";
	const string  ndsprefix = " Nodes: ";
	{
		// Set first digit to zero to have a valid header in case the stub header will not be replaced
		constexpr char zval[] = "0,";
		idvalStub.replace(0, sizeof zval - 1, zval);  // Note: -1 to not include the null terminator
		string  header(hdrprefix + idvalStub + ndsprefix + idvalStub  // Note: ',' might be putted on the place of ' ' after the actual value
			+ " Fuzzy: 0, Numbered: 0\n");
		fputs(header.c_str(), fout);
	}

	ClusterHashes  chashes;  // Hashes of the processed clusters
	// Note: it is not mandatory to evaluate and write the number of unique nodes
	// in the merged clusters, but it is much cheaper to do it on clusters merging
	// than on reading the formed files. It will reduce the number of allocations on reading.
	UniqIds  uniqnds;  // Unique member node ids of the merged clusters
#if TRACE >= 2
	Size  totcls = 0;  // Total number of clusters read from all files
	Size  totmbs = 0;  // Total number of members (nodes with repetitions) read from all files
	Size  hashedmbs = 0;  // Total number of members (nodes with repetitions) in the hashed clusters from all files
#endif // TRACE
	constexpr char  mbdelim[] = " \t";  // Delimiter for the members
	// Note: strings defined out of the cycle to avoid reallocations
	StringBuffer  line;  // Reading line
	string  clstr;  // Writing cluster string
	vector<Id>  cnds;  // Cluster nodes. Note: a dedicated container is required to filter clusters by size
	for(auto& file: files) {
		// Note: CNL [CSN] format only is supported
		size_t  clsnum = 0;  // The number of clusters
		size_t  ndsnum = 0;  // The number of nodes

		// Parse header and read the number of clusters if specified
		parseHeader(file, line, clsnum, ndsnum);
		// Validate and correct the number of clusters if required
		// Note: it's better to reallocate a few times than too much overconsume the memory
		if(ndsnum && clsnum > ndsnum)
			clsnum = ndsnum;

		// Estimate the number of clusters in the file if not specified
		if(!clsnum) {
			size_t  cmsbytes = 0;
			if(!ndsnum) {
#ifdef __unix__  // sqrt(cmsbytes) lines => linebuf = max(4-8Kb, sqrt(cmsbytes) * 2) with dynamic realloc
				struct stat  filest;
				int fd = fileno(file);
				if(fd != -1 && !fstat(fd, &filest))
					cmsbytes = filest.st_size;
				//fprintf(stderr, "  %s: %lu bytes\n", fname, cmsbytes);
#endif // __unix
				// Get length of the file
				if(!cmsbytes) {
					fseek(file, 0, SEEK_END);
					cmsbytes = ftell(file);  // The number of bytes in the input communities
					rewind(file);  // Set position to the begin of the file
				}
				if(cmsbytes && cmsbytes != size_t(-1))  // File length fetching failed
					ndsnum = estimateNodes(cmsbytes);
			}
			clsnum = estimateClusters(ndsnum);
#if TRACE >= 2
			fprintf(stderr, "mergeCollections(), %lu bytes, %lu nodes (estimated: %u)"
				", %lu clusters\n", cmsbytes, ndsnum, cmsbytes != 0, clsnum);
#endif // TRACE
		} else {
#if TRACE >= 2
			fprintf(stderr, "mergeCollections(), specified %lu clusters, %lu nodes\n"
				, clsnum, ndsnum);
#endif // TRACE
			if(!ndsnum)
				ndsnum = clsnum * clsnum;  // The expected number of nodes
		}

		// Preallocate space for the clusters hashes
		if(chashes.bucket_count() * chashes.max_load_factor() < clsnum)
			chashes.reserve(clsnum);
		// Preallocate space for nodes
		if(uniqnds.bucket_count() * uniqnds.max_load_factor() < ndsnum)
			uniqnds.reserve(ndsnum);
		cnds.reserve(ndsnum);

		// Load clusters
#if TRACE >= 2
		size_t  fclsnum = 0;  // The number of read clusters from the file
#endif // TRACE
		AggHash  agghash;  // Aggregation hash for the cluster nodes (ids)
		do {
			char *tok = strtok(line, mbdelim);  // const_cast<char*>(line.data())

			// Skip comments
			if(!tok || tok[0] == '#')
				continue;
			// Skip the cluster id if present
			if(tok[strlen(tok) - 1] == '>') {
				const char* cidstr = tok;
				tok = strtok(nullptr, mbdelim);
				// Skip empty clusters, which actually should not exist
				if(!tok) {
					fprintf(stderr, "WARNING mergeCollections(), empty cluster"
						" exists: '%s', skipped\n", cidstr);
					continue;
				}
			}
			do {
				// Note: only node id is parsed, share part is skipped if exists,
				// but potentially can be considered in NMI and F1 evaluation.
				// In the latter case abs diff of shares instead of co occurrence
				// counting should be performed.
				Id  nid = strtoul(tok, nullptr, 10);
				cnds.push_back(nid);
				agghash.add(nid);
				clstr.append(tok) += ' ';
				// Note: the number of nodes can't be evaluated here simply incrementing the value,
				// because clusters might have overlaps, i.e. the nodes might have multiple membership
				//
				// Note: besides the overlaps the collection might represent the
				// flattened hierarchy, where each nodes has multiple membership
				// (to each former level) without the actual node sharing, or
				// this sharing should consider distinct belonging ratio
				// ~ inversely proportional to the  number of nodes in the cluster
			} while((tok = strtok(nullptr, mbdelim)));
#if TRACE >= 2
			totmbs += agghash.size();
#endif // TRACE

			// Filter read cluster by size
			if(cnds.size() >= cmin && (!cmax || cnds.size() >= cmax)) {
				uniqnds.insert(cnds.begin(), cnds.end());
#if TRACE >= 2
				++fclsnum;  // The number of valid read lines, i.e. clusters
#endif // TRACE
				// Save clstr to the output file if such hash has not been processed yet
				if(chashes.insert(agghash.hash()).second) {
#if TRACE >= 2
					hashedmbs += agghash.size();
#endif // TRACE
					clstr.pop_back();  // Remove the ending ' '
					// Consider the case when ' ' was added after '\n'
                    if(clstr.back() != '\n')
						clstr.push_back('\n');
					fputs(clstr.c_str(), fout);
				}
			}
			// Prepare outer vars for the next iteration
			cnds.clear();
			agghash.clear();  // Clear the hash
			clstr.clear();  // Clear (but not reallocate) outputting cluster string
		} while(line.readline(file));
#if TRACE >= 2
		totcls += fclsnum;
#endif // TRACE
	}

	// Update the header with the actual number of clusters
	if(fout.reopen("r+")) {
		fseek(fout, hdrprefix.size(), SEEK_SET);
		// Write the actual number of stored clusters
		if(fprintf(fout, "%lu,", chashes.size()) < 0)
			perror("WARNING mergeCollections(), failed to update the file header with the number of clusters");
		// Write the number of unique nodes in the stored clusters
		fseek(fout, hdrprefix.size() + idvalStub.size() + ndsprefix.size(), SEEK_SET);
		if(fprintf(fout, "%lu,", uniqnds.size()) < 0)
			perror("WARNING mergeCollections(), failed to update the file header with the number of nodes");
	} else perror(("WARNING mergeCollections(), can't reopen '" + fout.name()
		+ "', the stub header has not been replaced").c_str());
#if TRACE >= 2
	fprintf(stderr, "mergeCollections(),  merged %lu clusters, %lu members into"
		" %lu clusters, %lu members. Resulting rations: %G clusters, %G members\n"
		, totcls, totmbs, chashes.size(), hashedmbs
		, float(chashes.size()) / totcls, float(hashedmbs) / totmbs);
#endif // TRACE
}
