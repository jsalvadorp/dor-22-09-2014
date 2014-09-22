#ifndef STD_H
#define STD_H

#include "forward.h"

extern Module * std;

void init_std(Evaluator * e);

Atom error(Cons * args);

#endif
