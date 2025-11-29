# CT Seminars MIPT DREC 2025

Prepared a compact set of my solutions of CT seminars's tasks: processes (`fork/exec`), signals, pthreads, and SysV semaphores with shared memory. Each source builds independently with `gcc`; add `-pthread` (and `-lm` for trig examples) when needed. IPC samples use paired programs; clean leftover semaphores/shared memory via `ipcs`/`ipcrm` after crashes. No Makefile — compile the specific file you want to run.
