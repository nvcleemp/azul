/*
 *  basicdelaney.h
 *  
 *
 *  Created by Nico Van Cleemput on 24/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _BASICDELANEY_H // if not defined
#define _BASICDELANEY_H // define BasicDelaney

#include <stdio.h>

struct __delaney{
	int size;
	int chambers[60][3];
	
	int m[60][2];
	
	int marker[60];
	int marker2[60];
	
	int comment1;
	int comment2;
};

struct __delaney_collection{
	int size;
	struct __delaney collection[2000];
};

typedef struct __delaney DELANEY;
typedef struct __delaney_collection DELANEY_COLLECTION;

//print a Delaney symbol
void printDelaney(DELANEY *symbol, FILE *f);

void exportDelaney(DELANEY *symbol);

void exportDelaneyNumbered(DELANEY *symbol, int nr1, int nr2);

void emptyDelaney(DELANEY *symbol, int size);

int collapse(DELANEY *symbol, int chamber1, int chamber2, int* partition);

void minimal_delaney(DELANEY *symbol, DELANEY *minimal_symbol);

int compare(DELANEY *symbol1, DELANEY *symbol2);

void canonical_chamber_relabelling(DELANEY *symbol, int *relabelling, int start);

void apply_relabelling(DELANEY *origin, int *relabelling, DELANEY *image);

void canonical_form(DELANEY *symbol, DELANEY *canon_symbol);

void copyDelaney(DELANEY *original, DELANEY *copy);

int addSymbol2Library(DELANEY *symbol, DELANEY_COLLECTION *library);

int getChambersInOrbit(DELANEY *symbol, int start, int i, int j);

int getOrbitSize(DELANEY *symbol, int start, int i, int j);

void mark_orbit(DELANEY *symbol, int *marker, int chamber, int i, int j, int clean);

#endif // end if not defined, and end the header file
