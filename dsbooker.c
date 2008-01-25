/*
 *  dsbooker.c
 *  
 *
 *  Created by Nico Van Cleemput on 05/12/07.
 *
 *  Provides several bookkeeping operations for delaney symbols
 *
 *  gcc dsbooker.c -o dsbooker -Wall
 */

#include <stdio.h>
#include "basicdelaney.h"

DELANEY_COLLECTION library;

int calculateMinimal = 0;
int format = 0; //0 for ds-format, 1 for human-readable table
int libMode = 0; //0 read-only, 1 append
int verbose = 0; //0 no, 1 print out info to stderr

/*****************************************************************************/

void printOrbit(DELANEY *symbol, int s1, int s2){
	int i, chamber=0;
	int sigmas[2];
	int marker[symbol->size];
	sigmas[0]=s1;
	sigmas[1]=s2;
	fprintf(stderr, "s%ds%d-orbits\n", s1,s2);
	fprintf(stderr, "===========\n");
	fprintf(stderr, "symbol size = %d\n", symbol->size);
	//set markers to 0
	for(i=0; i<symbol->size; i++) marker[i]=0;
	while(chamber<symbol->size){
		if(!(marker[chamber])){
			if(s1-s2>1 || s2-s1>1)
				fprintf(stderr, "m{%d,%d} = 2\n", s1, s2);
			else
				fprintf(stderr, "m{%d,%d} = %d\n", s1, s2, symbol->m[chamber][s1<s2 ? s1 : s2]);
			fprintf(stderr, "%d", chamber);
			int j=0, next = symbol->chambers[chamber][sigmas[j]], size = 0;
			while(next!=chamber || j!=1){
				fprintf(stderr, " -> %d", next);
				marker[next]=1;
				if(j) size++;
				j = (j+1)%2;
				next = symbol->chambers[next][sigmas[j]];
			}
			fprintf(stderr, " -> %d\n", next);
			fprintf(stderr, "orbit size = %d\n\n", ++size);
			marker[chamber]=1;
		}
		chamber++;
	}
}

void fillm4orbit(DELANEY *symbol, int m, int value, int start){
	symbol->m[start][m]=value;
	int i=0, next = symbol->chambers[start][m];
	while(next!=start || i!=1){
		symbol->m[next][m]=value;
		i = (i+1)%2;
		next = symbol->chambers[next][m+i];
	}
}

int readDelaney(char *filename){
	int c;
	int offered = 0;
	FILE *lib;
	lib = fopen(filename, "r");
	if(lib==NULL) {
		perror("dsbooker");
		return 0;
	} else {
		while((c=getc(lib))!=EOF){
			if(c=='\n'){
			} else if(c!='<'){
				fprintf(stderr, "Error: Found %c and expected '<'.\n", c);
				return 0;
			} else {
				/******************************/
				offered++;
				int d1, d2;
				if(fscanf(lib, "%d.%d:", &d1, &d2)==0){
					fprintf(stderr, "Error: Illegal format: Must start with number.number.\n");
					return 0;
				}
				/******************************/
				DELANEY symbol;
				symbol.comment1 = d1;
				symbol.comment2 = d2;
				if(fscanf(lib, "%d", &d1)==0){
					fprintf(stderr, "Error: Illegal format. Cannot read size.\n");
					return 0;
				}
				emptyDelaney(&symbol, d1);
				if(fscanf(lib, "%d", &d1) && d1 != 2){
					fprintf(stderr, "Error: Currently only Delaney symbols with dimension 2 are supported.\n");
					return 0;
				}
				while((c=getc(lib))!=':');
				/******************************/
				int i = 0, j=0;
				while(j < 3){
					while(fscanf(lib, "%d", &d1)){
						if(d1-1>=symbol.size || i>=symbol.size){
							fprintf(stderr, "Error: Illegal format. Indices grow too large while building sigma_%d-functions: %d and %d, while size is %d.\n", j, d1, i, symbol.size);
							return 0;
						}
						symbol.chambers[i][j]=d1-1;
						symbol.chambers[d1-1][j]=i;
						while(symbol.chambers[i][j]!=-1) i++;
					}
					i=0;
					j++;
					while(j<3 && (c=getc(lib))!=',');
				}
				while((c=getc(lib))!=':');
				/******************************/
				i = 0; j=0;
				while(j < 2){
					while(fscanf(lib, "%d", &d1)){
						fillm4orbit(&symbol, j, d1, i);
						while(i<symbol.size && symbol.m[i][j]!=-1) i++;
					}
					i=0;
					j++;
					while(j<2 && (c=getc(lib))!=',');
				}
				while((c=getc(lib))!='>');
				
				if(calculateMinimal){
					DELANEY minsymbol;
					minimal_delaney(&symbol, &minsymbol);
					if(addSymbol2Library(&minsymbol, &library) && verbose){
						fprintf(stderr, "Added the following minimal symbol to the library:\n");
						printDelaney(&minsymbol, stderr);
					}
				} else {
					if(addSymbol2Library(&symbol, &library) && verbose){
						//fprintf(stderr, "Added the following symbol to the library:\n");
						//printDelaney(&symbol);
						exportDelaney(&symbol);
				
					}
				}
			}
		}
	}
	//if(verbose) {
		fprintf(stderr, "Added %d symbols to the library.\n", library.size);
		fprintf(stderr, "Offered %d symbols to the library.\n", offered);
	//}
	fclose(lib);
	return 1;
}

int readSingleDelaney(DELANEY *symbol, FILE *f){
	int c;
	while((c=getc(f))!='<')
		if(c==EOF)
			return EOF;

	/******************************/
	int d1, d2;
	if(fscanf(f, "%d.%d:", &d1, &d2)==0){
		fprintf(stderr, "Error: Illegal format: Must start with number.number.\n");
		return 1;
	}
	/******************************/
	if(fscanf(f, "%d", &d1)==0){
		fprintf(stderr, "Error: Illegal format. Cannot read size.\n");
		return 1;
	}
	emptyDelaney(symbol, d1);
	if(fscanf(f, "%d", &d1) && d1 != 2){
		fprintf(stderr, "Error: Currently only Delaney symbols with dimension 2 are supported.\n");
		return 1;
	}
	while((c=getc(f))!=':');
	/******************************/
	int i = 0, j=0;
	while(j < 3){
		while(fscanf(f, "%d", &d1)){
			if(d1-1>=symbol->size || i>=symbol->size){
				fprintf(stderr, "Error: Illegal format. Indices grow too large while building sigma_%d-functions: %d and %d, while size is %d.\n", j, d1, i, symbol->size);
				return 0;
			}
			symbol->chambers[i][j]=d1-1;
			symbol->chambers[d1-1][j]=i;
			while(symbol->chambers[i][j]!=-1) i++;
		}
		i=0;
		j++;
		while(j<3 && (c=getc(f))!=',');
	}
	while((c=getc(f))!=':');
	/******************************/
	i = 0; j=0;
	while(j < 2){
		while(fscanf(f, "%d", &d1)){
			fillm4orbit(symbol, j, d1, i);
			while(i<symbol->size && symbol->m[i][j]!=-1) i++;
		}
		i=0;
		j++;
		while(j<2 && (c=getc(f))!=',');
	}
	while((c=getc(f))!='>');
	
	if(calculateMinimal){
		DELANEY minsymbol;
		minimal_delaney(symbol, &minsymbol);
		if(addSymbol2Library(&minsymbol, &library) && verbose){
			fprintf(stderr, "Added the following minimal symbol to the library:\n");
			printDelaney(&minsymbol, stderr);
		}
	} else {
		if(addSymbol2Library(symbol, &library) && verbose){
			fprintf(stderr, "Added the following symbol to the library:\n");
			printDelaney(symbol, stderr);
		}
	}
	
	return 1;
}

int domain(){

return 1;
}

int main(int argc, char *argv[]){

	int c, error = 0;
	while (--argc > 0 && (*++argv)[0] == '-'){
		while (c = *++argv[0])
			switch (c) {
			case 'm':
				calculateMinimal = 1;
				break;
			case 'f':
				format = 0;
				break;
			case 't':
				format = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'a':
				libMode = 1;
				fprintf(stderr, "Warning: append currently not yet supported!\n");
				break;
			case 'h':
				//print help
				fprintf(stderr, "The program dsbooker provides several bookkeeping operations for delaney symbols.\n");
				fprintf(stderr, "Usage: dsbooker [-mftah] LIBFILE\n\n");
				fprintf(stderr, "Valid options:\n");
				fprintf(stderr, "  -m\t: Create a library of minimal symbols\n");
				fprintf(stderr, "  -f\t: Output file in ds format. Overwrites any t previously given.\n");
				fprintf(stderr, "  -t\t: Output human-readable tables. Overwrites any f previously given.\n");
				fprintf(stderr, "  -v\t: Print human-readable information to stderr.\n");
				fprintf(stderr, "  -a\t: Append new symbols to the library file.\n");
				fprintf(stderr, "  -h\t: Print this help and return.\n");
				return 0;
			default:
				fprintf(stderr, "dsbooker: illegal option %c\n", c);
				argc = 0;
				error = 1;
				break;
			}
	}
	if(error || argc != 1){
		fprintf(stderr, "Usage: dsbooker [options] LIBFILE. Use dsbooker -h for more information.\n");
		return -1;
	} else {
		if(!readDelaney(*argv) || !domain()){
			fprintf(stderr, "Warning: error while reading libfile. Aborting further execution of program.\n");
			return 1;
		}
	}
	
	return 0;
}
