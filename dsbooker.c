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

struct delaney{
	int size;
	int chambers[48][3];
	
	int m[48][2];
	
	int marker[48];
	int marker2[48];
};

struct delaney_collection{
	int size;
	struct delaney collection[1000];
};

struct delaney_collection library;

int calculateMinimal = 0;
int format = 0; //0 for ds-format, 1 for human-readable table
int libMode = 0; //0 read-only, 1 append

/*****************************************************************************/

void printDelaney(struct delaney *symbol){
	int i;
	fprintf(stdout, "|    | s0 | s1 | s2 | m01 | m12 |\n");
	fprintf(stdout, "|===============================|\n");
	for(i = 0; i<symbol->size; i++)
		fprintf(stdout, "| %2d | %2d | %2d | %2d | %3d | %3d |\n", i, symbol->chambers[i][0], symbol->chambers[i][1], symbol->chambers[i][2], symbol->m[i][0], symbol->m[i][1]);
	fprintf(stdout, "|===============================|\n");
	fprintf(stdout, "\n\n");
}

/*****************************************************************************/

void emptyDelaney(struct delaney *symbol, int size){
	int i,j;
	symbol->size = size;
	for(i=0; i < size; i++){
		for(j=0; j < 3; j++)
			symbol->chambers[i][j]=-1;
		symbol->m[i][0]=-1;
		symbol->m[i][1]=-1;
	}
}

/*****************************************************************************/
/*
 * Collapses the two chambers and if this method returns true then partition
 * contains the resulting partition.
 */
int collapse(struct delaney *symbol, int chamber1, int chamber2, int* partition){
	int stack[48*47][2];
	int stacksize;
	int i,j;
	int size = symbol->size;
	for(i=0; i<2; i++)
		if(symbol->m[chamber1][i]!=symbol->m[chamber2][i])
			return 0;
		
	if(partition[chamber1]==partition[chamber2])
		return 1; //already collapsed
		
	//union
	if(partition[chamber1]<partition[chamber2]){
		int oldvalue = partition[chamber2];
		for(i = 0; i<size; i++)
			if(partition[i]==oldvalue)
				partition[i]=partition[chamber1];
	} else {
		int oldvalue = partition[chamber1];
		for(i = 0; i<size; i++)
			if(partition[i]==oldvalue)
				partition[i]=partition[chamber2];
	}
	
	stack[0][0] = chamber1;
	stack[0][1] = chamber2;
	stacksize = 1;
	
	while(stacksize){
		stacksize--;
		int current1 = stack[stacksize][0];
		int current2 = stack[stacksize][1];
		for(j = 0; j<3; j++){
			int neighbour1 = symbol->chambers[current1][j];
			int neighbour2 = symbol->chambers[current2][j];
			for(i=0; i<2; i++)
				if(symbol->m[neighbour1][i]!=symbol->m[neighbour2][i])
					return 0;
			
			//union
			if(partition[neighbour1]<partition[neighbour2]){
				int oldvalue = partition[neighbour2];
				for(i = 0; i<size; i++)
					if(partition[i]==oldvalue)
						partition[i]=partition[neighbour1];
				stack[stacksize][0] = neighbour1;
				stack[stacksize][1] = neighbour2;
				stacksize++;
			} else if(partition[neighbour1]>partition[neighbour2]){
				int oldvalue = partition[neighbour1];
				for(i = 0; i<size; i++)
					if(partition[i]==oldvalue)
						partition[i]=partition[neighbour2];
				stack[stacksize][0] = neighbour1;
				stack[stacksize][1] = neighbour2;
				stacksize++;
			}
		}
	}
	
	return 1;
}

/*
 * Constructs the minimal Delaney symbol of symbol by adding symmetry
 */
void minimal_delaney(struct delaney *symbol, struct delaney *minimal_symbol){
	int size = symbol->size;
	int partition[size];
	int temp[size];
	int i, j;
	for(i=0; i<size; i++)
		partition[i]=i;
	
	//completely collapse symbol
	for(i=1; i<size; i++){
		//copy partition to temp
		for(j=0;j<size;j++)
			temp[j]=partition[j];
		
		//collapse 0 and i
		if(collapse(symbol, 0, i, temp)) //when successfull
			for(j=0;j<size;j++) //copy temp to partition
				partition[j]=temp[j];
	}
	
	//create new symbol from partition
	//labelling
	int newsize = 0;
	int old2new[size];
	int new2old[size];
	for(i=0; i<size; i++)
		old2new[i]=-1;
	for(i=0; i<size; i++){
		if(old2new[partition[i]]==-1){
			old2new[partition[i]]=newsize;
			new2old[newsize]=partition[i];
			newsize++;
		}
		old2new[i]=old2new[partition[i]];
	}
	
	//creation
	minimal_symbol->size = newsize;
	for(i=0; i<newsize; i++){
		minimal_symbol->m[i][0]=symbol->m[new2old[i]][0];
		minimal_symbol->m[i][1]=symbol->m[new2old[i]][1];
		minimal_symbol->chambers[i][0]=old2new[symbol->chambers[new2old[i]][0]];
		minimal_symbol->chambers[i][1]=old2new[symbol->chambers[new2old[i]][1]];
		minimal_symbol->chambers[i][2]=old2new[symbol->chambers[new2old[i]][2]];
	}
}

/*****************************************************************************/

/*
 * positive when symbol1 > symbol2
 * 0 when symbol1 == symbol2
 * negative when symbol1 < symbol2
 */
int compare(struct delaney *symbol1, struct delaney *symbol2){
	int i=0;
	if(symbol1->size!=symbol2->size)
		return symbol1->size - symbol2->size;
	while(i<symbol1->size && symbol1->m[i][0] == symbol2->m[i][0])
		i++;
	if(i<symbol1->size)
		return (symbol1->m[i][0] - symbol2->m[i][0]);
		
	int j;
	for(j=0;j<3;j++){
		i=0;
		while(i<symbol1->size && symbol1->chambers[i][j] == symbol2->chambers[i][j])
			i++;
		if(i<symbol1->size)
			return (symbol1->chambers[i][j] - symbol2->chambers[i][j]);
	}
	
	return 0;
}

/*
 * When this method returns relabelling will contain a canonical relabelling of the chambers 
 * that gives the chamber 'start' the label '0'.
 * This relabelling is based on a DFS that choses the children to visit in the order sigma_0 .. sigma_2
 */
void canonical_chamber_relabelling(struct delaney *symbol, int *relabelling, int start){
	int stack[48];
	int stacksize;
	int i, j;
	int visited[48];
	int index = 0;
	for(i=0; i<48; i++)
		visited[i]=0;
	
	relabelling[index++] = start;
	visited[start]=1;
	stack[0] = start;
	stacksize = 1;
	
	while(stacksize>0){
		int chamber = stack[--stacksize];
		for(j=0; j<3; j++){
			if(!visited[symbol->chambers[chamber][j]]){
				visited[symbol->chambers[chamber][j]] = 1;
				relabelling[index++]=symbol->chambers[chamber][j];
				stack[stacksize++] = symbol->chambers[chamber][j];
			}
		}
	}
}

/*
 * Applies the relabelling to origin and fills image
 */
void apply_relabelling(struct delaney *origin, int *relabelling, struct delaney *image){
	int reverse_labelling[48];
	int i;
	image->size = origin->size;
	for(i=0; i<origin->size; i++)
		reverse_labelling[relabelling[i]] = i;
	
	for(i=0; i<origin->size; i++){
		image->m[i][0] = origin->m[relabelling[i]][0];
		image->m[i][1] = origin->m[relabelling[i]][1];
		image->chambers[i][0] = reverse_labelling[origin->chambers[relabelling[i]][0]];
		image->chambers[i][1] = reverse_labelling[origin->chambers[relabelling[i]][1]];
		image->chambers[i][2] = reverse_labelling[origin->chambers[relabelling[i]][2]];
	}
}

/*
 * canon_symbol contains the canonical form of symbol when this method returns
 */
void canonical_form(struct delaney *symbol, struct delaney *canon_symbol){
	int i;
	int  relabelling[48];
	int found=0; //true when we already found a possible candidate
	struct delaney temp_delaney;
	for(i=0; i<symbol->size; i++){
		if(symbol->m[i][0]==4){
			// all our symbol contain at least 2 fours so
			// we can only have a canonical form if we start
			// with such a chamber
			canonical_chamber_relabelling(symbol, relabelling, i);
			if(found){
				apply_relabelling(symbol, relabelling, &temp_delaney);
				if(compare(&temp_delaney, canon_symbol)<0)
					apply_relabelling(symbol, relabelling, canon_symbol);
			} else {
				apply_relabelling(symbol, relabelling, canon_symbol);
				found = 1;
			}
		}
	}
}

/*
 * returns 1 if a new entry was made in the library.
 */
int add2library(struct delaney *symbol){
	canonical_form(symbol, library.collection + library.size);
	int i = 0;
	while(i<library.size && compare(library.collection + library.size, library.collection + i)!=0)
		i++;
	if(i==library.size) {
		library.size++;
		return 1;
	} else
		return 0;
}

void fillm4orbit(struct delaney *symbol, int m, int value, int start){
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
				int d1, d2;
				if(fscanf(lib, "%d.%d:", &d1, &d2)==0){
					fprintf(stderr, "Error: Illegal format: Must start with number.number.\n");
					return 0;
				}
				/******************************/
				struct delaney symbol;
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
			}
		}
	}
	fclose(lib);
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
		readDelaney(*argv);
	}
	
	return 0;
}
