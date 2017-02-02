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

#ifdef __has_include
	#if __has_include(<filesystem>)
		#include <filesystem>
		namespace fs = std::filesystem;
	#elif __has_include(<experimental/filesystem>)
		#include <experimental/filesystem>
		namespace fs = std::experimental::filesystem;
	#endif // __has_include
	//#include <system_error>
#else // Platform-dependent file IO
	#error "STL filesystem is not available. The native alternative is not implemented."
#endif // __has_include

#include "cmdline.h"  // Arguments parsing
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
	if(!args_info.output_given && args_info.inputs_num == 1
	&& is_directory(args_info.inputs[0])
	// Note: "../." like templates are not verified and result in the output to the ..cnl file
	&& args_info.inputs[0] != "." && args_info.inputs[0] != "..") {
		// Update default output filename in case single dir is specified
		outpname = args_info.inputs[0];
		outpname += ".cnl";
	}

	// Check whether the output already exists
	if(exists(outpname)) {
		fprintf(stderr, "WARNING, the output file '%s' already exists, rewrite it: %i\n"
			, outpname.c_str(), args_info.rewrite_flag);
		if(!args_info.rewrite_flag)
			return 1;
	}

//	// Open input files (clusterings on multiple resolution levels)
//	auto files = openFiles(args_info.inputs, args_info.inputs_num);
//	if(files.empty())
//		return 1;
//
//	auto clusters = loadClusters(files, args_info.fixed-nodes, args_info.min_size_arg, args_info.max_size_arg);
//	outputClusters(clusters)

    printf("Output file name: %s\n", outpname.c_str());
    return 0;
}
