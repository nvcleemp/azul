/*
 *  periodicgraph.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/03/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _PERIODICGRAPH_H // if not defined
#define _PERIODICGRAPH_H // define PeriodicGraph

#include <stdio.h>
#include <stdlib.h>
#include "basicdelaney.h"

struct __pgraph{
	int order;
	int size;
	struct __pgedge *edges;
	double x[120];//TODO: make pointer and use malloc
	double y[120];//TODO: make pointer and use malloc
};

struct __pgedge{
	int from;
	int to;
	int x;
	int y;
};

typedef struct __pgraph PeriodicGraph;
typedef struct __pgedge PGEdge;

int createPeriodicGraph(DELANEY *symbol, PeriodicGraph *graph);

void exportPeriodicGraph(PeriodicGraph *graph, FILE *f, int endLine);

#endif // end if not defined, and end the header file
