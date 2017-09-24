# aftermath
Collection of c++ header files providing support for various mathematical tasks.

Currently contains the following sections:
- **algebra** Some generic math, most importantly including `matrix` types and `permuations`.
- **format** Provides MATLAB v4 .mat file support for `matrix` types.
- **probability** Contains some basic distribution types; as opposed to standard c++, the intention of these headers is to provide a framework for working with mathematical/statistical properties of said distributions; (pseudo-) random number generation is dealt with in the next section (_random_).
- **random** Provides samplers for some probability distributions. It features an implementation of ziggurat method (see _Marsaglia and Tsang \[2000\]_ and _Doornik \[1997\]_) for generating normal random variables, offering a slight speed-up compared to the built-in c++ implementation.

_Note on the deviation from standard c++ distribution/sampling_. The current standard is to let distribution types perform sampling via `operator ()`; we, on the other hand, introduce _sampler types_ to perform the sampling via the same `operator ()` interface, and let _distribution types_ expose just the mathematical properties of the distributions. We think this is an important distinction; it also covers the standard case in that c++ distribution types are also sampler types.
