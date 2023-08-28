# BRL-CAD external dependencies

This repository holds local copies of dependencies used by the [BRL-CAD](https://github.com/BRL-CAD/brlcad) computer aided design system.

# Quick start

* Clone this repository
```sh
git clone https://github.com/BRL-CAD/brlcad_externals
```
* Make a build directory
```sh
mkdir brlcad_externals_build && cd brlcad_externals_build
```
* Configure with CMake.  Individual components can be enabled or disabled, but the ENABLE_ALL flag is used to automatically turn on all the projects targeted for BRL-CAD bundling.
```sh
cmake ../brlcad_externals -DENABLE_ALL=ON
```
* Run the build process.  (Parallel building is supported and should succeed.)
```sh
cmake --build . --parallel
```

# Using the Build Outputs with BRL-CAD

At the moment this repository and BRL-CAD's support of it are still experimental.
To test it, the steps are as follows:

* Clone the BRL-CAD repository
```sh
git clone https://github.com/BRL-CAD/brlcad
```
* Remove the internal ext directories (be sure NOT to accidentally commit this change to the repository...)
```sh
cd brlcad && rm -rf misc/tools/ext src/other/ext
```
* Make a build directory
```sh
cd .. && mkdir brlcad_exttest_build && cd brlcad_exttest_build
```
* Configure with CMake.  If no custom CMAKE_INSTALL_PREFIX was supplied to the brlcad_externals build, BRL-CAD will know to look for the folder it needs in the user's home directory. If a custom location was used, specify the path holding the brlcad_ext output directory using knitr::inline_expr(-DBRLCAD_EXT_DIR=<your_root>) (brlcad_ext should in turn contain the extinstall and extnoinstall folders.)  If you also wish to test with Qt, you must currently enable that support in BRL-CAD as well.  (Note that the BRL-CAD configure process is responsible for staging the BRLCAD_EXT_DIR contents into the build directory, so it can take some time to complete...)
```sh
cmake ../brlcad -DBRLCAD_ENABLE_QT=ON
```
* Run the build process.  (Parallel building is supported and should succeed.)
```sh
cmake --build . --parallel
cmake --build . --parallel --target package
```

If all goes well, the final result should be a relocatable BRL-CAD archive
including both the main BRL-CAD outputs and the products of this external
dependencies repository.


# About This Repository

Projects in this repository are distinct from those stored directly in
BRL-CAD's src/other repository. The latter generally satisfy one of the
following conditions:

* Has modifications from upstream that are a) not of interest to the upstream
  and b) required for correct functioning in BRL-CAD.
* Has no active upstream to provide support.

Unlike those cases, BRL-CAD CAN (in principle) work correctly with system
installed versions of the libraries and utilities stored here.  Because not all
platforms and environments can provide all of these dependencies, this
repository serves as a "one stop shop" supplying developers with a way to build
the components they need from source code.

Although it is possible to install the products of these builds to system
locations and have BRL-CAD detect them there, the intended approach is for
BRL-CAD to bundle these dependences and provide them directly as part of its
own installers.  This allows (for example) Windows installations of BRL-CAD to
be far more self contained - users on such platforms generally expect to be
able to download and use a software package immediately, regardless of what is
or is not already present on the operating system.

The main BRL-CAD repository manages the bundling process - this repository's
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

# BRL-CAD Compilation Tools

Although most of the components here are intended for bundling with BRL-CAD,
there are a few exceptions to that rule.  A few of the projects (AStyle, patch,
LIEF, etc.) target a different installation folder and are used only for
BRL-CAD compilation (and/or support for building this externals repository.)
They can be identified by checking for a CMAKE_NOBUNDLE_INSTALL_PREFIX target
being used by their top level CMakeLists.txt file managing the
ExternalProject_Add build definition.

