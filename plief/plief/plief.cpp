/*                  S T R C L E A R . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018-2023 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file plief.cpp
 *
 * Use https://github.com/lief-project/LIEF to modify RPATHs in
 * binaries
 */

#include <iostream>
#include <string>
#include <vector>
#include "cxxopts.hpp"
#include "LIEF/ELF.hpp"
#include "LIEF/LIEF.h"

int
main(int argc, const char *argv[])
{
    bool dump_mode = false;
    bool clear_mode = false;
    bool verbose = false;
    std::string new_rpath;

    cxxopts::Options options(argv[0], "A program to clear or replace rpaths in binaries\n");

    std::vector<std::string> nonopts;

    try
    {
	options
	    .set_width(70)
	    .add_options()
	    ("D,dump",       "Print rpath/runpath", cxxopts::value<bool>(dump_mode))
	    ("remove-rpath", "Remove the binary's rpath", cxxopts::value<bool>(clear_mode))
	    ("add-rpath",    "Add the specified rpath to the binary", cxxopts::value<std::string>(new_rpath))
	    ("v,verbose",  "Verbose reporting during processing", cxxopts::value<bool>(verbose))
	    ("h,help",     "Print help")
	    ;
	auto result = options.parse(argc, argv);

	nonopts = result.unmatched();

	if (result.count("help")) {
	    std::cout << options.help({""}) << std::endl;
	    return 0;
	}

	if (new_rpath.length() && clear_mode) {
	    std::cerr << "Error:  need to specify either addition or removal of rpath, not both\n";
	    return -1;
	}
    }
    catch (const cxxopts::exceptions::exception& e)
    {
	std::cerr << "error parsing options: " << e.what() << std::endl;
	return -1;
    }

    if (nonopts.size() != 1) {
	std::cerr << "Error:  need to specify a binary file to process\n";
	return -1;
    }

    std::unique_ptr<LIEF::ELF::Binary> binfo = std::unique_ptr<LIEF::ELF::Binary>{LIEF::ELF::Parser::parse(nonopts[0])};

    if (dump_mode) {
	// Thanks to the C api example for the hint on how to unpack this...
	LIEF::ELF::DynamicEntry *rue = binfo->get(LIEF::ELF::DYNAMIC_TAGS::DT_RUNPATH);
	if (rue) {
	    std::cout << reinterpret_cast<LIEF::ELF::DynamicEntryRunPath*>(rue)->name() << '\n';
	    return 0;
	}
	LIEF::ELF::DynamicEntry *rpe = binfo->get(LIEF::ELF::DYNAMIC_TAGS::DT_RPATH);
	if (rpe)
	    std::cout << reinterpret_cast<LIEF::ELF::DynamicEntryRpath*>(rpe)->name() << " (DT_RPATH)" << '\n';
	return 0;
    }

    if (clear_mode) {
	binfo->remove(LIEF::ELF::DYNAMIC_TAGS::DT_RPATH);
	binfo->remove(LIEF::ELF::DYNAMIC_TAGS::DT_RUNPATH);
	binfo->write(nonopts[0]);
	return 0;
    }

    if (new_rpath.length()) {
	LIEF::ELF::DynamicEntryRunPath npe(new_rpath);
	binfo->add(npe);
	binfo->write(nonopts[0]);
	return 0;
    }

    std::cout << options.help({""}) << std::endl;
    return -1;
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8

