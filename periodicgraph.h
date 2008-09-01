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
	int faceCount;
	struct __pgedge *edges;
	struct __pgface *faces;
	int colored[200];
	double x[120];//TODO: make pointer and use malloc
	double y[120];//TODO: make pointer and use malloc
};

struct __pgedge{
	int from;
	int to;
	int x;
	int y;
};

struct __pgface{
	int order;
	//int *vertices; TODO: use malloc
	int vertices[60];
};

typedef struct __pgraph PeriodicGraph;
typedef struct __pgedge PGEdge;
typedef struct __pgface PGFace;

int createPeriodicGraph(DELANEY *symbol, PeriodicGraph *graph);

void exportPeriodicGraph(PeriodicGraph *graph, FILE *f, int endLine);

#endif // end if not defined, and end the header file
