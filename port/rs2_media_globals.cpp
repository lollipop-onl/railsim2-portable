// Why not uncomment svm / svv in lib/sysvalue.h beside the other SYSVALUE_*
// globals: lib/main.cpp, the only includer of sysvalue.h, sees the types
// through lib/udx.h, and udx.h leaves music.h and movie.h commented out.
// Making the two types visible there would put the DirectMusic and
// DirectShow types into every udx.h includer. Upstream never needed the
// definitions because it did not build lib/music.cpp or lib/movie.cpp, the
// only TUs that use them.

#include "headers.h"
#include <dshow.h>
#include "music.h"
#include "movie.h"

SYSVALUE_M svm;
SYSVALUE_V svv;
