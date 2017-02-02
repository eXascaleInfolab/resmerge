//! \brief Resolution level clusterings merger with filtering.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

#include <cstdio>
#include <string>

#include "cmdline.h"  // Arguments parsing
#define INCLUDE_STL_FS
#include "fileio.h"


using std::string;
using fs::is_directory;
using fs::exists;

int main(int argc, char **argv)
{
	gengetopt_args_info  args_info;
	auto  err = cmdline_parser(argc, argv, &args_info);
	if(err)
		return err;

	if(!args_info.inputs_num) {
		fputs("ERROR, input clusterings are required\n", stderr);
		cmdline_parser_print_help();
		return 1;
	}

	// Get output file name
	string  outpname = args_info.output_arg;
	{
		string  name = string(args_info.inputs[0]);  // Name of the first entry
		// Update default output filename in case single dir is specified
		if(!args_info.output_given && args_info.inputs_num == 1
		&& is_directory(name)
		// Note: "../." like templates are not verified and result in the output to the ..cnl file
		&& name != "." && name != "..")
			outpname = name + ".cnl";
	}

	// Check whether the output already exists
	if(exists(outpname)) {
		fprintf(stderr, "WARNING, the output file '%s' already exists, rewrite it: %i\n"
			, outpname.c_str(), args_info.rewrite_flag);
		if(!args_info.rewrite_flag)
			return 1;
	}

	// Open input files (clusterings on multiple resolution levels)
	vector<const char*>  names;
	names.reserve(args_info.inputs_num);
	for(size_t i = 0; i < args_info.inputs_num; ++i)
		names.push_back(args_info.inputs[i]);
	auto files = openFiles(names);
	if(files.empty())
		return 1;

//	auto clusters = loadClusters(files, args_info.fixed-nodes, args_info.min_size_arg, args_info.max_size_arg);
//	outputClusters(clusters)

    printf("Output file name: %s\n", outpname.c_str());
    return 0;
}
