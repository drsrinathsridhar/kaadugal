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

#### NEWS
##### [15-Oct-2014]: Classification forests are done! See below for usage example.
##### [1-Oct-2014]: This library isn't fully implemented yet. I am actively working on it though and hope to finish soon.

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
./classify train ../examples/config/<CONFIG_FILE> <OUTPUT_FOREST_PATH> ../examples/data/<DATA_FILE>
```

Then test the forest on input data using

```bash
./classify test <INPUT_FOREST_PATH> ../examples/data/<DATA_FILE>
```

# Contact

Srinath Sridhar (srinaths@umich.edu)
Max Planck Institute for Informatics
