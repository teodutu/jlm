/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlm/backend/llvm/jlm2llvm/jlm2llvm.hpp>
#include <jlm/backend/llvm/rvsdg2jlm/rvsdg2jlm.hpp>
#include <jlm/frontend/llvm/InterProceduralGraphConversion.hpp>
#include <jlm/frontend/llvm/LlvmModuleConversion.hpp>
#include <jlm/ir/ipgraph-module.hpp>
#include <jlm/ir/print.hpp>
#include <jlm/ir/RvsdgModule.hpp>
#include <jlm/util/Statistics.hpp>

#include <jive/view.hpp>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>

#include <getopt.h>

class cmdflags {
public:
	inline
	cmdflags() {}

	std::string file;
	jlm::StatisticsDescriptor sd;
	std::string l2jdot_function;
	std::string r2jdot_function;
};

static void
print_usage(const std::string & app)
{
	std::cerr << "Usage: " << app << " --file <path-to-c-file>\n";
}

static void
parse_cmdflags(int argc, char ** argv, cmdflags & flags)
{
	static constexpr size_t file = 1;

	static struct option options[] = {
	  {"file", required_argument, NULL, file}
	, {NULL, 0, NULL, 0}
	};

	int opt;
	while ((opt = getopt_long_only(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case file: { flags.file = optarg; break; }
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (flags.file.empty()) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
}

int
main (int argc, char ** argv)
{
	/* TODO: Simplify this logic. */
	cmdflags flags;
	parse_cmdflags(argc, argv, flags);

	llvm::LLVMContext ctx;
	llvm::SMDiagnostic err;
	auto lm = llvm::parseIRFile(flags.file, err, ctx);

	if (!lm) {
		err.print(argv[0], llvm::errs());
		exit(1);
	}

	/* LLVM to JLM pass */
	auto jm = jlm::ConvertLlvmModule(*lm);
	auto rvsdgModule = jlm::ConvertInterProceduralGraphModule(*jm, flags.sd);
	jive::view(rvsdgModule->Rvsdg().root(), stdout);

	return 0;
}
