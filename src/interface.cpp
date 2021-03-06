//! \brief ResMerge interface functions
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-13

#include <cstring>  // strlen
#include <unordered_map> // strlen
#include <cmath>  // sqrt
#include <cassert>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include "interface.h"


using std::unordered_map;
using std::invalid_argument;
using std::numeric_limits;
using std::find;
using fs::exists;
using fs::is_directory;
using fs::directory_iterator;


// Interface functions definitions ---------------------------------------------
NamedFileWrapper createFile(const string& outpname, bool rewrite)
{
	NamedFileWrapper  fout;  // Use NRVO optimization
	// Check whether the output already exists
	if(exists(outpname)) {
		fprintf(stderr, "WARNING createFile(), the output file '%s' already exists, rewrite it: %s\n"
			, outpname.c_str(), toYesNo(rewrite));
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

NamedFileWrappers openFiles(const FileNames& names)
{
	NamedFileWrappers files;  // NRVO (Return Value Optimization) is used

	assert(!names.empty() && "openFiles(), entry names are expected");
	FileNames  unexisting;  // Unexisting entries
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
			++inpdirs;
			for(const auto& detry: directory_iterator(npath)) {
				if(is_directory(detry))
					continue;
				NamedFileWrapper  finp(detry.path().c_str(), "r");
				if(finp)
					files.push_back(move(finp));
				else perror((string("WARNING openFiles(), can't open ") += detry.path()).c_str());
			}
		} else {
			++inpfiles;
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

	if(files.empty())
		fputs("WARNING openFiles(), the input data does not exist\n", stderr);
#if TRACE >= 1
	else printf("openFiles(), opened %lu files from the %u files and %u dirs\n"
			, files.size(), inpfiles, inpdirs);
#endif // TRACE

	return files;
}

bool mergeCollections(NamedFileWrapper& fout, NamedFileWrappers& files
, NamedFileWrapper& fbase, Id cmin, Id cmax, float membership)
{
	if(!fout) {
		fputs("ERROR extractBase(), the output file is undefined\n", stderr);
		return false;
	}
	// Validate the fout is empty
	{
		size_t  fosize = fout.size();
		if(fosize && fosize != size_t(-1)) {
			fputs("ERROR extractBase(), the output file should be empty\n", stderr);
			return false;
		}
	}

	// Load the node base, otherwise declare unique member node ids of the merged clusters
	// Note: the node base clusters are not filtered by size, because they might be loaded
	// either from the ground-truth collection or from the dedicated node base. The filtering
	// is performed only on the node base extraction
	auto  nodebase = loadNodes<Id, AccId>(fbase, membership);
	const bool nosync = nodebase.empty();  // Do not sync the node base

	// Write a stub header the actual values of clusters and nodes will be later,
	// so now just use stubs (' ' of the sufficient length) for them.
	string  idvalStub = string(ceil(numeric_limits<Id>::digits10), ' ');
	const string  hdrprefix = "# Clusters: ";
	const string  ndsprefix = " Nodes: ";
	{
		// Set first digit to zero to have a valid header in case the stub header will not be replaced
		constexpr char zval[] = "0,";
		idvalStub.replace(0, sizeof zval - 1, zval);  // Note: -1 to not include the null terminator
		string  header(hdrprefix + idvalStub + ndsprefix + idvalStub  // Note: ',' can be putted later on the place of ' ' after the actual value
			+ " Fuzzy: 0, Numbered: 0\n");
		fputs(header.c_str(), fout);
	}

	// Hashes of the clusters
	//using ClusterHashes = unordered_set<size_t>;
	using ClusterHash = daoc::AggHash<>;
	using ClusterHashes = vector<ClusterHash>;  // The same size_t (ClusterHash::hash) can be yielded for distinct ClusterHash
	using ClustersHashes = unordered_map<ClusterHash::IdT, ClusterHashes>;
	ClustersHashes  chashes;  // Hashes of the processed clusters
	// Note: it is not mandatory to evaluate and write the number of unique nodes
	// in the merged clusters, but it is much cheaper to do it on clusters merging
	// than on reading the formed files. It will reduce the number of allocations on reading.
#if TRACE >= 2
	AccId  totcls = 0;  // Total number of clusters read from all files
	AccId  totmbs = 0;  // Total number of members (nodes with repetitions) read from all files
	AccId  hashedmbs = 0;  // Total number of members (nodes with repetitions) in the hashed clusters from all files
#endif // TRACE
	// ATTENTION: without '\n' delimiter the terminating '\n' is read as an item
	constexpr char  mbdelim[] = " \t\n";  // Delimiter for the members
	// Note: strings defined out of the cycle to avoid reallocations
	StringBuffer  line;  // Reading line
	string  clstr;  // Writing cluster string
	vector<Id>  cnds;  // Cluster nodes. Note: a dedicated container is required to filter clusters by size
	Id  cfltnum = 0;  // The number of filtered out clusters
	for(auto& file: files) {
		// Note: CNL [CSN] format only is supported
		size_t  clsnum = 0;  // The number of clusters
		size_t  ndsnum = 0;  // The number of nodes

		// Parse header and read the number of clusters if specified
		parseCnlHeader(file, line, clsnum, ndsnum);

		// Estimate the number of nodes and clusters in the file if not specified
		uint8_t  estimnds = 0;  // Estimation flag
		if(!ndsnum) {
			size_t  cmsbytes = -1;
			cmsbytes = file.size();
			if(cmsbytes != size_t(-1)) {  // File length fetching failed
				ndsnum = estimateCnlNodes(cmsbytes, membership);
				estimnds = 1;
			} else if(clsnum) {
				ndsnum = 2 * clsnum; // / membership;  // Note: use optimistic estimate instead of pessimistic (square / membership) to not overuse the memory
				estimnds = 2;
			}
		}
		if(!clsnum && ndsnum) {
			clsnum = estimateClusters(ndsnum, membership);
#if TRACE >= 2
			fprintf(stderr, "mergeCollections(), %lu nodes (estimated: %u)"
				", %lu estimated clusters\n", ndsnum, estimnds, clsnum);
#endif // TRACE
		} else {
#if TRACE >= 2
			fprintf(stderr, "mergeCollections(), specified %lu clusters, %lu nodes\n"
				, clsnum, ndsnum);
#endif // TRACE
		}

		// Preallocate space for the clusters hashes
		if(chashes.bucket_count() * chashes.max_load_factor() < clsnum)
			chashes.reserve(clsnum);
		// Preallocate space for nodes
		if(nosync && nodebase.bucket_count() * nodebase.max_load_factor() < ndsnum)
			nodebase.reserve(ndsnum);
		// Note: typically the cluster size does not increase the square root of the number of nodes
		cnds.reserve(sqrt(ndsnum));

		// Load clusters
#if TRACE >= 2
		size_t  fclsnum = 0;  // The number of read clusters from the file
#endif // TRACE
		daoc::AggHash<Id, AccId>  agghash;  // Aggregation hash for the cluster nodes (ids)
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
#if VALIDATE >= 2
				if(!nid && tok[0] != '0') {
					fprintf(stderr, "WARNING mergeCollections(), conversion error of '%s' into 0: %s\n"
						, tok, strerror(errno));
					continue;
				}
#endif // VALIDATE
#if TRACE >= 2
				++totmbs;  // Update the total number of read members
#endif // TRACE
				// Filter by the node base if required
				if(nosync || nodebase.count(nid)) {
					cnds.push_back(nid);
					agghash.add(nid);
					clstr.append(tok) += ' ';
				}
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
			++fclsnum;  // The number of valid read lines, i.e. clusters
#endif // TRACE

			// Filter read cluster by size
			if(cnds.empty()) {
#if VALIDATE >= 2
				assert(!agghash.size() && clstr.empty()
					&& "mergeCollections(), asynchronous internal containers");
#endif // VALIDATE
				++cfltnum;
				continue;
			}
			if(cnds.size() >= cmin && (!cmax || cnds.size() <= cmax)) {
				// Form the node base if it was not specified explicitly
				if(!nosync)
					nodebase.insert(cnds.begin(), cnds.end());
				// Save clstr to the output file if such hash has not been processed yet
				const auto ch = agghash.hash();
				const auto ich = chashes.find(ch);
				if(ich == chashes.end()
				|| std::find(ich->second.begin(), ich->second.end(), agghash) == ich->second.end()) {
					chashes[ch].push_back(agghash);
#if TRACE >= 2
					hashedmbs += agghash.size();
#endif // TRACE
					clstr.pop_back();  // Remove the ending ' '
					// Consider the case when ' ' was added after '\n'
                    if(clstr.back() != '\n')
						clstr.push_back('\n');
					if(fputs(clstr.c_str(), fout) == EOF) {
						perror("ERROR mergeCollections(), merged clusters output failed");
						return false;
					}
				} else ++cfltnum;
			} else ++cfltnum;
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
		if(fprintf(fout, "%lu,", nodebase.size()) < 0)
			perror("WARNING mergeCollections(), failed to update the file header with the number of nodes");
	} else perror(("WARNING mergeCollections(), can't reopen '" + fout.name()
		+ "', the stub header has not been replaced").c_str());
#if TRACE >= 2
	fprintf(stderr, "mergeCollections(),  merged %lu clusters, %lu members into"
		" %lu clusters, %lu members, %u clusters filtered out. Resulting rations: %G clusters, %G members\n"
		, totcls, totmbs, chashes.size(), hashedmbs, cfltnum
		, float(chashes.size()) / totcls, float(hashedmbs) / totmbs);
#endif // TRACE
	printf("%u clusters filtered, remained: %lu\n", cfltnum, chashes.size());

	return true;
}

bool extractBase(NamedFileWrapper& fout, NamedFileWrappers& files, Id cmin, Id cmax
, float membership)
{
	if(!fout) {
		fputs("ERROR extractBase(), the output file is undefined\n", stderr);
		return false;
	}
	// Validate the fout is empty
	{
		size_t  fosize = fout.size();
		if(fosize && fosize != size_t(-1)) {
			fputs("ERROR extractBase(), the output file should be empty\n", stderr);
			return false;
		}
	}

	// Write a stub header the actual values of clusters and nodes will be later,
	// so now just use stubs (' ' of the sufficient length) for them.
	string  idvalStub = string(ceil(numeric_limits<Id>::digits10), ' ');
	const string  hdrprefix = "# Clusters: 1, Nodes: ";  // Store node base as a single cluster
	{
		// Set first digit to zero to have a valid header in case the stub header will not be replaced
		constexpr char zval[] = "0,";
		idvalStub.replace(0, sizeof zval - 1, zval);  // Note: -1 to not include the null terminator
		string  header(hdrprefix + idvalStub  // Note: ',' can be putted later on the place of ' ' after the actual value
			+ " Fuzzy: 0, Numbered: 0\n");
		fputs(header.c_str(), fout);
	}

	UniqIds  nodebase;  // Unique node ids
#if TRACE >= 2
	AccId  totcls = 0;  // Total number of clusters read from all files
	AccId  totmbs = 0;  // Total number of members (nodes with repetitions) read from all files
#endif // TRACE
	// ATTENTION: without '\n' delimiter the terminating '\n' is read as an item
	constexpr char  mbdelim[] = " \t\n";  // Delimiter for the members
	// Note: strings defined out of the cycle to avoid reallocations
	StringBuffer  line;  // Reading line
	vector<Id>  cnds;  // Cluster nodes. Note: a dedicated container is required to filter clusters by size
	for(auto& file: files) {
		// Note: CNL [CSN] format only is supported
		size_t  clsnum = 0;  // The number of clusters
		size_t  ndsnum = 0;  // The number of nodes

		// Parse header and read the number of clusters if specified
		parseCnlHeader(file, line, clsnum, ndsnum);

		// Estimate the number of nodes in the file if not specified
		if(!ndsnum) {
			size_t  cmsbytes = file.size();
			if(cmsbytes != size_t(-1))  // File length fetching failed
				ndsnum = estimateCnlNodes(cmsbytes, membership);
			else if(clsnum)
				ndsnum = 2 * clsnum; // / membership;  // Note: use optimistic estimate instead of pessimistic (square / membership) to not overuse the memory
#if TRACE >= 2
			fprintf(stderr, "extractBase(), estimated %lu nodes\n", ndsnum);
#endif // TRACE
		}
#if TRACE >= 2
		else fprintf(stderr, "extractBase(), specified %lu nodes\n", ndsnum);
#endif // TRACE

		// Preallocate space for the nodes, but do it seldom and only if not filtered,
		// because the filtering might cause much smaller node base than it can be estimated
		if(nodebase.bucket_count() * nodebase.max_load_factor() < ndsnum / 25)
			nodebase.reserve(ndsnum);
		// Note: typically the cluster size does not increase the square root of the number of nodes
		cnds.reserve(sqrt(ndsnum));

		// Load clusters
#if TRACE >= 2
		size_t  fclsnum = 0;  // The number of read clusters from the file
#endif // TRACE
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
					fprintf(stderr, "WARNING extractBase(), empty cluster"
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
#if VALIDATE >= 2
				if(!nid && tok[0] != '0') {
					fprintf(stderr, "WARNING extractBase(), conversion error of '%s' into 0: %s\n"
						, tok, strerror(errno));
					continue;
				}
#endif // VALIDATE
#if TRACE >= 2
				++totmbs;  // Update the total number of read members
#endif // TRACE
				// Filter by the node base if required
				cnds.push_back(nid);
			} while((tok = strtok(nullptr, mbdelim)));
#if TRACE >= 2
			++fclsnum;  // The number of valid read lines, i.e. clusters
#endif // TRACE

			// Filter read cluster by size and form the nodebase
			if(cnds.size() >= cmin && (!cmax || cnds.size() <= cmax))
				nodebase.insert(cnds.begin(), cnds.end());
			// Prepare outer vars for the next iteration
			cnds.clear();
		} while(line.readline(file));
#if TRACE >= 2
		totcls += fclsnum;
#endif // TRACE
	}

	// Update the header with the actual number of clusters
	if(fout.reopen("r+")) {
		fseek(fout, hdrprefix.size(), SEEK_SET);
		// Write the actual number of stored nodes as a single cluster
		if(fprintf(fout, "%lu,", nodebase.size()) < 0)
			perror("WARNING extractBase(), failed to update the file header with the number of nodes");
	} else perror(("WARNING extractBase(), can't reopen '" + fout.name()
		+ "', the stub header has not been replaced").c_str());
#if TRACE >= 2
	fprintf(stderr, "extractBase(),  merged %lu clusters, %lu members into"
		" the base of %lu nodes. Members ratio to the nodebase: %G\n"
		, totcls, totmbs, nodebase.size(), totmbs / float(nodebase.size()));
#endif // TRACE

	// Output the nodebase
	errno = 0;
	if(!fseek(fout, 0, SEEK_END)) {
		for(auto nid: nodebase)
			fprintf(fout, "%u ", nid);
		fputs("\n", fout);
	}
	if(errno) {
		perror("ERROR, node base output failed");
		return false;
	}

	return true;
}
