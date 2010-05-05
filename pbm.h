/* pbm.h
 *
 * Copyright (c) 2010 Michael Forney
 */

#ifndef PBM_H
#define PBM_H 1

#include "ice.h"

void read_pbm(const char const * filename, uint32_t ** state, int * width, int * height);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax

