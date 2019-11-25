//! \brief Resolution level clusterings merger with filtering.
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-01

#include <cassert>
#include "cmdline.h"  // Arguments parsing
#include "macrodef.h"
#include "interface.h"


using fs::is_directory;

//! \brief Arguments parser
struct ArgParser: gengetopt_args_info {
	ArgParser(int argc, char **argv) {
		auto  err = cmdline_parser(argc, argv, this);
		if(err)
			throw std::invalid_argument("Arguments parsing failed" + std::to_string(err));
	}

	~ArgParser() {
		cmdline_parser_free(this);
	}
};


int main(int argc, char **argv)
{
	ArgParser  args_info(argc, argv);

	if(!args_info.inputs_num) {
		fputs("ERROR, input clusterings are required\n", stderr);
		cmdline_parser_print_help();
		return 1;
	}

	// Get output file name
	string  outpname = args_info.output_arg;  // Default output name
	{
		string  name = string(args_info.inputs[0]);  // Name of the first entry
		// Remove trailing '/', '\\'
		while(name.size() && name.back() == PATHSEP)
			name.pop_back();
		// Update default output filename in case single dir is specified
		if(!args_info.output_given && args_info.inputs_num == 1) {
			if(is_directory(name)
			// Note: "../." like templates are not verified and result in the output to the ..cnl file
			&& name != "." && name != "..")
				outpname = name + (args_info.extract_base_flag ? "_base.cnl" : ".cnl");
			else if(args_info.extract_base_flag) {
				auto isep = name.find_last_of("./\\");
				if(isep != string::npos && name[isep] == '.')
					name.insert(isep, "_base");
				else name += "_base.cnl";
				outpname = name;
			}
		}
	}
	printf("Arguments parsed:\n\tmode: %s\n\toutput: %s\n"
		, args_info.extract_base_flag ? "extract" : "merge [& sync]" , outpname.c_str());

	// Create the output file checking it's existence
	NamedFileWrapper fout = createFile(outpname, args_info.rewrite_flag);
	if(!fout)
		return 1;
#if TRACE >= 2
	puts(("Output file created: " + fout.name()).c_str());
#endif // TRACE

	// Open the node base file to sync with it
	NamedFileWrapper  fbase;
	if(args_info.sync_base_given) {
		auto files = openFiles({args_info.sync_base_arg});
		if(files.empty())
			return 1;
#if VALIDATE >= 2
		assert(files.size() == 1 && "a single node base file is expected");
#endif // VALIDATE
		fbase = move(files.front());
	}

	// Open input files (clusterings on multiple resolution levels)
	FileNames  names;
	names.reserve(args_info.inputs_num);
	for(size_t i = 0; i < args_info.inputs_num; ++i)
		names.push_back(args_info.inputs[i]);
	auto files = openFiles(names);
	if(files.empty())
		return 1;

	bool success = false;
	if(!args_info.extract_base_flag)
		success = mergeCollections(fout, files, fbase, args_info.btm_size_arg
			, args_info.top_size_arg, args_info.membership_arg);
	else success = extractBase(fout, files, args_info.btm_size_arg
			, args_info.top_size_arg, args_info.membership_arg);
	if(success)
		printf("%lu CNL files processed into %s\n", files.size(), outpname.c_str());
	else fputs("WARNING, CNL files processing failed\n", stderr);

    return !success;  // Return 0 on success
}
