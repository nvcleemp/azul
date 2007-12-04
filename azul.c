/*
 *  azul.c
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 * 
 *  gcc azul.c basicmath.c -o azul -Wall
 *
 */

#include "basicmath.h"
#include "utility.h"
#include <stdio.h>

#define MAX 124
#define MIN 4
#define STEP 2
#define SIZE 6

struct bucket{
	int colour;
	int size;
};

struct bucket_collection{
	struct bucket collection[8];
	int collection_size;
};

struct delaney{
	int size;
	int chambers[48][3];
	
	int m01[48];
	
	int marker[48];
};

struct delaney_collection{
	int size;
	struct delaney collection[623];
};

struct delaney_collection library;

struct delaney_collection minimal_library;

int counter = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;

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
	if(symbol->m01[chamber1]!=symbol->m01[chamber2])
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
			if(symbol->m01[neighbour1]!=symbol->m01[neighbour2])
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
		minimal_symbol->m01[i]=symbol->m01[new2old[i]];
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
	while(i<symbol1->size && symbol1->m01[i] == symbol2->m01[i])
		i++;
	if(i<symbol1->size)
		return (symbol1->m01[i] - symbol2->m01[i]);
		
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
		image->m01[i] = origin->m01[relabelling[i]];
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
		if(symbol->m01[i]==4){
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

void add_to_library(struct delaney *symbol){
	canonical_form(symbol, library.collection + library.size);
	int i = 0;
	while(i<library.size && compare(library.collection + library.size, library.collection + i)!=0)
		i++;
	if(i==library.size)
		library.size++;
}

void add_to_minimal_library(struct delaney *symbol){
        canonical_form(symbol, minimal_library.collection + minimal_library.size);
        int i = 0;
        while(i<minimal_library.size && compare(minimal_library.collection + minimal_library.size, minimal_library.collection + i)!=0)
                i++;
        if(i==minimal_library.size)
                minimal_library.size++;
}

/*****************************************************************************/

void printDelaney(struct delaney *symbol){
	int i;
	fprintf(stdout, "|    | s0 | s1 | s2 | m01 | m12 |\n");
	fprintf(stdout, "|===============================|\n");
	for(i = 0; i<symbol->size; i++)
		fprintf(stdout, "| %2d | %2d | %2d | %2d | %3d | %3d |\n", i, symbol->chambers[i][0], symbol->chambers[i][1], symbol->chambers[i][2], symbol->m01[i], 3);
	fprintf(stdout, "|===============================|\n");
	fprintf(stdout, "\n\n");
}

void markorbit(struct delaney *symbol, int chamber, int i, int j){
	symbol->marker[chamber]=1;
	symbol->marker[symbol->chambers[chamber][i]]=1;
	int next = symbol->chambers[symbol->chambers[chamber][i]][j];
	while(next!=chamber){
		symbol->marker[next]=1;
		symbol->marker[symbol->chambers[next][i]]=1;
		next = symbol->chambers[symbol->chambers[next][i]][j];	
	}
}

void exportDelaney(struct delaney *symbol){
	int i,j;
	//size information
	fprintf(stdout, "<1.1:%d 2:", symbol->size);
	
	//sigma
	fprintf(stdout, "%d", symbol->chambers[0][0]+1);
	for(i = 1; i < symbol->size; i++){
		if(symbol->chambers[i][0] >= i)
			fprintf(stdout, " %d", symbol->chambers[i][0]+1);
	}
	for(i = 1; i<3; i++){
		fprintf(stdout, ",%d", symbol->chambers[0][i]+1);
		for(j = 1; j < symbol->size; j++)
			if(symbol->chambers[j][i] >= j)
				fprintf(stdout, " %d", symbol->chambers[j][i]+1);
	}
	
	//m01
	fprintf(stdout, ":");
	for(i = 0; i < symbol->size; i++)
		symbol->marker[i]=0;
		
	for(i = 0; i < symbol->size; i++){
		if(!symbol->marker[i]){
			fprintf(stdout, " %d", symbol->m01[i]);
			markorbit(symbol, i, 0, 1);
		}
	}
	
	
	//m12
	fprintf(stdout, ",");
	for(i = 0; i < symbol->size; i++)
		symbol->marker[i]=0;
		
	for(i = 0; i < symbol->size; i++){
		if(!symbol->marker[i]){
			fprintf(stdout, " %d", 3);
			markorbit(symbol, i, 1, 2);
		}
	}
	
	fprintf(stdout, ">\n");
}

/*
 * Check to see if m01 is constant on the sigma_0 sigma_1 - orbit through chamber and checks the size of that orbit.
 */
int check_sigma0sigma1orbit(struct delaney *symbol, int chamber){
	int j;
	for(j = 0; j<2; j++){
		int i = j;
		int next = symbol->chambers[chamber][i];
		int size = 1;
		while(next != -1 && (next != chamber || i!=(j+1)%2) && symbol->m01[next]==symbol->m01[chamber]){
			i = (i+1)%2;
			size++;
			next = symbol->chambers[next][i];
		}
		if(next != -1 && (next != chamber || i!=(j+1)%2))
			return 0;
		if(next == chamber && i==(j+1)%2 && (2*symbol->m01[chamber])%size!=0)
			return 0;
		else if(size > 2*symbol->m01[chamber])
			return 0;
	}
	return 1;
}

/*
 * Construct the remaining sigma_0 edges.
 */
void complete_sigma0(struct delaney *symbol){
	int i = 16;
	while(symbol->chambers[i][0]!=-1 && i<47)
		i+=4;
	if(i==48){
		//new symbol!
		counter4++;
		add_to_library(symbol);
	} else {
		//first try loop
		symbol->chambers[i][0] = i;
		//symbol->chambers[i][2] cannot have sigma0 already assigned: must also be a loop
		symbol->chambers[symbol->chambers[i][2]][0] = symbol->chambers[i][2];
		if(check_sigma0sigma1orbit(symbol, i) && check_sigma0sigma1orbit(symbol, symbol->chambers[i][2]))
			complete_sigma0(symbol);
		//undo
		symbol->chambers[i][0] = -1;
		symbol->chambers[symbol->chambers[i][2]][0] = -1;
		
		//next try back-edge
		if(symbol->m01[i]==symbol->m01[symbol->chambers[i][2]]){
			symbol->chambers[i][0] = symbol->chambers[i][2];
			symbol->chambers[symbol->chambers[i][2]][0] = i;
			if(check_sigma0sigma1orbit(symbol, i))
				complete_sigma0(symbol);
			//undo
			symbol->chambers[i][0] = -1;
			symbol->chambers[symbol->chambers[i][2]][0] = -1;
		}
		
		
		//finally try other chambers
		int j;
		for(j = i+3; j<44; j+=4){
			if(symbol->chambers[j][0] == -1){
				//first j
				if(symbol->m01[i]==symbol->m01[j] &&
				   symbol->m01[symbol->chambers[i][2]]==symbol->m01[symbol->chambers[j][2]]){
					symbol->chambers[i][0] = j;
					symbol->chambers[j][0] = i;
					symbol->chambers[symbol->chambers[i][2]][0] = symbol->chambers[j][2];
					symbol->chambers[symbol->chambers[j][2]][0] = symbol->chambers[i][2];
					if(check_sigma0sigma1orbit(symbol, i) && check_sigma0sigma1orbit(symbol, symbol->chambers[i][2]))
						complete_sigma0(symbol);
					//undo
					symbol->chambers[i][0] = -1;
					symbol->chambers[j][0] = -1;
					symbol->chambers[symbol->chambers[i][2]][0] = -1;
					symbol->chambers[symbol->chambers[j][2]][0] = -1;
				}
				//then j+1
				if(symbol->m01[i]==symbol->m01[symbol->chambers[j][2]] &&
				   symbol->m01[symbol->chambers[i][2]]==symbol->m01[j]){
					symbol->chambers[i][0] = symbol->chambers[j][2];
					symbol->chambers[j][0] = symbol->chambers[i][2];
					symbol->chambers[symbol->chambers[i][2]][0] = j;
					symbol->chambers[symbol->chambers[j][2]][0] = i;
					if(check_sigma0sigma1orbit(symbol, i) && check_sigma0sigma1orbit(symbol, symbol->chambers[i][2]))
						complete_sigma0(symbol);
					//undo
					symbol->chambers[i][0] = -1;
					symbol->chambers[j][0] = -1;
					symbol->chambers[symbol->chambers[i][2]][0] = -1;
					symbol->chambers[symbol->chambers[j][2]][0] = -1;	
				}
			}		
		}
	}
}

/*
 * Complete the Delaney symbol with the calculated m01 values.
 */
void fill_m01(int *m01, struct delaney *symbol){
	int i, j;
	for(i=0; i<8; i++)
		for(j=0; j<4; j++)
			symbol->m01[16 + i*4 + j] = m01[i];
}

/*
 * Construct the basic Delaney symbol for the tiling. See text.
 */
void basicDelaney(struct delaney *symbol){
	int i;
	symbol->size = 48;
	for(i = 0; i< 48; i++){
		symbol->chambers[i][0] = -1;
		symbol->chambers[i][1] = -1;
		symbol->chambers[i][2] = -1;
		symbol->m01[i] = -1;
	}
	for(i = 1; i< 16; i=i+2){
		symbol->chambers[i][0] = (i - 1)%16;
		symbol->chambers[(i - 1)%16][0] = i;
		symbol->chambers[i][1] = (i + 1)%16;
		symbol->chambers[(i + 1)%16][1] = i;
		symbol->chambers[i][2]= 2*i + 16;
		symbol->chambers[i-1][2]= 2*i + 15;
		symbol->chambers[2*i + 16][2]= i;
		symbol->chambers[2*i + 15][2]= i-1;

		symbol->chambers[2*i + 14][1]= 2*i + 15;
		symbol->chambers[2*i + 15][1]= 2*i + 14;

		symbol->chambers[2*i + 15][0]= 2*i + 16;
		symbol->chambers[2*i + 16][0]= 2*i + 15;

		symbol->chambers[2*i + 16][1]= 2*i + 17;
		symbol->chambers[2*i + 17][1]= 2*i + 16;

		symbol->chambers[2*i + 17][2]= (2*i + 2)%32 + 16;
		symbol->chambers[(2*i + 2)%32 + 16][2]= 2*i + 17;
		
		symbol->m01[i]=8;
		symbol->m01[i-1]=8;
	}
}

/*
 * Check to see if the given circular string is canonical.
 */
int isCanonicalCircularString(int* circularString){
	int i;
	for(i = 1; i<8; i++){
		int j = 0;
		while(j < 8 && *(circularString + j) == *(circularString + (j + i)%8))
			j++;
		if(j<8 && *(circularString + j) > *(circularString + (j + i)%8)) {
			return 0; //not canonical
		}
	}
	return 1;
}

/*
 * Construct all the canonical circular strings and create the corresponding Delaney symbols
 */
void assign_m01(int* circularString, int position, struct bucket_collection *collection){
	int i;
	for(i = 0; i< collection->collection_size;i++){
		if((collection->collection + i)->size>0){
			*(circularString + position) = (collection->collection + i)->colour;
			(collection->collection + i)->size--;
			if(position<7)
				assign_m01(circularString, position+1, collection);
			else {
				//fprintf(stderr, "new circular string: %d - %d\n", counter, ++counter2);
				if(isCanonicalCircularString(circularString)){
					counter3++;
					counter2++;
					//fprintf(stdout, "%2d - %3d[%3d])\t", counter, counter2, counter3);
					//printintarray(circularString, 8);
					struct delaney symbol;
					basicDelaney(&symbol);
					fill_m01(circularString,&symbol);
					//printDelaney(&symbol);
					complete_sigma0(&symbol);
				}
			}
			(collection->collection + i)->size++;
		}
	}
}

/*
 * Creates a bucket collection based on the given array of numbers. This collection can then be used to enumerate all the circular strings.
 */
void createBucketCollection(struct bucket_collection *collection, int array[]){
	int i;
	collection->collection_size = 1;
	(collection->collection + collection->collection_size - 1)->colour=array[7];
	(collection->collection + collection->collection_size - 1)->size=1;
	for(i=6; i>=0; i--){
		if((collection->collection + collection->collection_size - 1)->colour==array[i]){
			(collection->collection + collection->collection_size - 1)->size++;
		} else {
			collection->collection_size++;
			(collection->collection + collection->collection_size - 1)->colour=array[i];
			(collection->collection + collection->collection_size - 1)->size=1;
		}
	}
}

/*
 * Check to see if the numbers satisfie condition D3 
 */
void checknumbers(int array[]){
	int i = 0;
	
	while(i < SIZE && array[i]>24){
		int temp = array[i];
		int count = 1;
		while(array[++i]==temp)
			count++;
		if(count*24<temp)
			return;
	}
	
	int denom = lcm(array[0], array[1]);
	for(i = 2; i < SIZE; i++)
		denom = lcm(denom, array[i]);
	int num = 0;
	for(i = 0; i < SIZE; i++)
		num += denom/array[i];
	if(num == denom) {
		//printf("%2d)\t", ++counter);
		//printintarray(array, SIZE+2);
		counter++;
		int ass[8];
		ass[0] = 4;
		struct bucket_collection coll;
		createBucketCollection(&coll, array);
		counter2=0;
		assign_m01(ass, 0, &coll);
		fprintf(stderr, "%2d) %3d canonical strings leading to %3d symbols\n", counter, counter2, counter4 - counter5);
		counter5 = counter4;
	}
}

void tick(int array[], int position){
	int i;
	if(position == 0 || array[position-1]==MAX){
		array[position]+=STEP;
		for(i = 0; i<position;i++)
			array[i]=array[position];
		checknumbers(array);
	} else {
		tick(array, position - 1);
	}
}    

int main()
{
	
		library.size=0;
		minimal_library.size=0;
        int array[SIZE+2];
        int i;
        for(i = 0; i<SIZE+2;i++)
                array[i]=MIN;
        while(array[SIZE - 1] < SIZE)
                tick(array, SIZE - 1);
	fprintf(stderr, "\nFound %d canonical circular strings\n", counter3);
	fprintf(stderr, "Found %d symbols\n", counter4);
	fprintf(stderr, "Found %d canonical symbols\n", library.size);
	struct delaney temp_minimal;
	for(i = 0; i<library.size;i++){
		minimal_delaney(library.collection+i,&temp_minimal);
		add_to_minimal_library(&temp_minimal);
	}
        fprintf(stderr, "Found %d minimal, canonical symbols\n\n", minimal_library.size);
	for(i=0;i<minimal_library.size;i++){
		exportDelaney(minimal_library.collection + i);
	}
	int frequentie[48];
	for(i=0;i<48;i++){
		frequentie[i]=0;
	}
	for(i=0;i<minimal_library.size;i++){
		frequentie[(minimal_library.collection + i)->size-1]++;
	}
	
	for(i=0;i<48;i++){
		if(frequentie[i])
			fprintf(stderr, "%d symbols with %d chambers\n", frequentie[i], i+1);
	}
	
	/*
	struct delaney symbol;
	basicDelaney(&symbol);
	printDelaney(&symbol);*/
	/*
	struct delaney symbol;
	int i;
	
	symbol.size=6;
	symbol.m01[0]=4;
	symbol.m01[1]=8;
	symbol.m01[2]=8;
	symbol.m01[3]=4;
	symbol.m01[4]=8;
	symbol.m01[5]=8;
	
	symbol.chambers[0][0]=0; symbol.chambers[0][1]=3; symbol.chambers[0][2]=1; 
	symbol.chambers[1][0]=1; symbol.chambers[1][1]=2; symbol.chambers[1][2]=0; 
	symbol.chambers[2][0]=2; symbol.chambers[2][1]=1; symbol.chambers[2][2]=5; 
	symbol.chambers[3][0]=3; symbol.chambers[3][1]=0; symbol.chambers[3][2]=4; 
	symbol.chambers[4][0]=4; symbol.chambers[4][1]=5; symbol.chambers[4][2]=3; 
	symbol.chambers[5][0]=5; symbol.chambers[5][1]=4; symbol.chambers[5][2]=2; 
	
	printDelaney(&symbol);
	
	struct delaney min_symbol;
	
	minimal_delaney(&symbol,  &min_symbol);
	printDelaney(&min_symbol);*/
	return 0;
}
