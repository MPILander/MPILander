# MPILander

There can only be one (MPI process)!

MPILander is a single-rank [mock library](https://en.wikipedia.org/wiki/Mock_object).
It aims to implement a full MPI 3.1 standard compatible MPI library with the most trivial program path: allowing *only one MPI process (rank)*!

## Why would I need MPILander?

MPILander is perfect for massively parallel applications and libraries that *also run well in single-rank mode*.
Often one faces the problem that either the main application or its dependencies are heavily relying on MPI functionality.
This complicates starting with one rank (e.g., still needs to call `mpiexec`) in many environments such as [CI](https://en.wikipedia.org/wiki/Continuous_integration) or intentionally serial (debug) workflows.

MPILander is here to help you with that.
Keep your regular MPI hard-wiring in place - and it will implement and provide everything you would expect from a MPI library, but only for exactly one and only one rank.
