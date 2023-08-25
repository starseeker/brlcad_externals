# BRL-CAD external dependencies

This repository holds local copies of dependencies used by the [BRL-CAD](https://github.com/BRL-CAD/brlcad) computer aided design system.

Unlike the code stored directly in BRL-CAD's src/other repository, which
generally either has modifications from upstream that are required for correct
functioning in BRL-CAD or has no active upstream at all, BRL-CAD can (in
principle) work correctly with system installed versions of the libraries and
utilities found here.  Because not all platforms and environemnts can provide
all of these dependencies, this repository serves as a "one stop shop" to supply
developers with the components they need.

Although it is possible in principle to install the products of these builds to
system locations and have BRL-CAD detect them there, the intended mode of operation
is for BRL-CAD to bundle these dependences and provide them directly as part of
its own installers.  This allows (for example) Windows installations of BRL-CAD
to be far more self contained - users on such platforms generally expect to be
able to download and use a software package immediately, regardless of what is
or is not already present on the operating system.

The BRL-CAD repository itself manages the bundling process - this repository's
responsibility is to prepare suitable inputs and manage the 3rd party building
processes across all targeted platforms.

In broad, the structure of the source tree is two tiered - the top level directories
each denote a specific dependency project (Tcl, Qt, GDAL, etc.).  Within that
directory are found:

* The upstream source code (usually this should be a vanilla copy of the upstream)
* Our managing CMakeLists.txt file defining the logic for building that project
* Any patch files or other supporting resources not part of the upstream build.

Sometimes third party libraries will depend on other third party libraries (for
example, libpng's use of zlib) - when a bundled component that is a dependency
of other bundled projects is enabled, the expectation is that the other bundled
components will use our copy rather than system copies of those libraries.

