# About

Kaadugal is a parallelized multi-core C++ implementation of the random forests
algorithm for classification, regression, and structured prediction problems.
Kaadugal is optimized for performance on multi-core systems with tens
of processors and a shared memory.

Kaadugal is known to work on multiple platforms including Linux and Windows.
It uses C++11 features and therefore requires a compiler that supports
these features.

Kaadugal is the English phoneticization of the word காடுகள் (forests)
in the Tamil language.

# Installation

Kaadugal is a header only library that can simply be included in your projects.
The kaadugal/include path should be in your project include path.

The examples directory contains a toy problem for training and testing simple
classification and structured prediction problems. The steps to build the
examples are:

```bash
$ pwd
<SOME_DIR>/kaadugal
$ mkdir build && cd build
$ cmake ../examples/
$ make
```

To run the example classification problem first learn the forest:

```bash
./classify learn ../examples/data/<DATA> <CONFIG>
```

Then test the forest on input data using

```bash
./classify test ../examples/data/<DATA> <CONFIG>
```

# Contact

Srinath Sridhar (srinaths@umich.edu)

Max Planck Institute for Informatics
