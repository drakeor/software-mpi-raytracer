# Software MPI Raytracer

This is a simple software raytracer implemented in both OpenMP and MPI for
a parallel programming class I was doing. It's written in C and requires no
external libraries. The code is quite a bit of a mess and there's some improperly
implementing things, but it served it's purpose

![Example Picture](https://drakeor.com/content/images/size/w1100/2021/06/rendered.png)

# Compilation

Compilers used:

- gcc (C99 standard)
- mpicc

A standard Makefile is included "make" in the directory should build the project.
Code should build without needing to use additional libraries.

I used linmath and render_bmp for linear algebra and saving bmp images respectively.
Code from these libraries are marked as not my code.

# Running

All executables are in the bin/ folder.
OpenMP executables are appended with the number of threads they will run
MPI executable can be called with "mpirun -np N bin/raytracer_mpi"
Serial executable is just "bin/raytracer"

After running, "rendered.bmp" will be generated in the root directory that has the final image.

# 3rd party files

- linmath.h -> 3rd party linear algebra library
- render_bmp.c -> 3rd party img saving library