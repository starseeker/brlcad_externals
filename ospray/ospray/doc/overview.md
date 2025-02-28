OSPRay Overview
===============

Intel OSPRay is an **o**pen source, **s**calable, and **p**ortable **ray**
tracing engine for high-performance, high-fidelity visualization on
Intel Architecture CPUs. OSPRay is part of the [Intel oneAPI Rendering
Toolkit](https://software.intel.com/en-us/rendering-framework) and is
released under the permissive [Apache 2.0
license](http://www.apache.org/licenses/LICENSE-2.0).

The purpose of OSPRay is to provide an open, powerful, and easy-to-use
rendering library that allows one to easily build applications that use
ray tracing based rendering for interactive applications (including both
surface- and volume-based visualizations). OSPRay is completely
CPU-based, and runs on anything from laptops, to workstations, to
compute nodes in HPC systems.

OSPRay internally builds on top of [Intel Embree] and [Intel ISPC
(Implicit SPMD Program Compiler)](https://ispc.github.io/), and fully
exploits modern instruction sets like Intel SSE4, AVX, AVX2, AVX-512 and NEON
to achieve high rendering performance, thus a CPU with support for at
least SSE4.1 is required to run OSPRay on x86_64 architectures.
A CPU with support for NEON is required to run OSPRay on ARM64 architectures.


OSPRay Support and Contact
--------------------------

OSPRay is under active development, and though we do our best to
guarantee stable release versions a certain number of bugs,
as-yet-missing features, inconsistencies, or any other issues are
still possible. Should you find any such issues please report
them immediately via [OSPRay's GitHub Issue
Tracker](https://github.com/ospray/OSPRay/issues) (or, if you should
happen to have a fix for it,you can also send us a pull request); for
missing features please contact us via email at
<ospray@googlegroups.com>.

To receive release announcements simply ["Watch" the OSPRay
repository](https://github.com/ospray/OSPRay) on GitHub.

