/*
 *  m01.c
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 * 
 *  Gives the solutions for
 *  1/x_1 + .... + 1/x_(SIZE)=1
 *  with x_i from MIN to MAX using STEP.
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
	int chambers[48][3];
	
	int m01[48];
};

int counter = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;

/*****************************************************************************/
/* positive when symbol1 > symbol2
 * 0 when symbol1 == symbol2
 * negative when symbol1 < symbol2
 */
int compare(struct delaney *symbol1, struct delaney *symbol2){
	int i=0;
	while(i<48 && symbol1->m01[i] == symbol2->m01[i])
		i++;
	if(i<48)
		return (symbol1->m01[i] - symbol2->m01[i]);
		
	int j;
	for(j=0;j<3;j++){
		i=0;
		while(i<48 && symbol1->chambers[i][j] == symbol2->chambers[i][j])
			i++;
		if(i<48)
			return (symbol1->chambers[i][j] - symbol2->chambers[i][j]);
	}
	
	return 0;
}

/* When this method returns relabelling will contain a canonical relabelling of the chambers 
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
				relabelling[index++]=symbol->chambers[chamber][j];
				stack[stacksize++] = symbol->chambers[chamber][j];
			}
		}
	}
}

/* Applies the relabelling to origin and fills image
 */
void apply_relabelling(struct delaney *origin, int *relabelling, struct delaney *image){
	int reverse_labelling[48];
	int i;
	for(i=0; i<48; i++)
		reverse_labelling[relabelling[i]] = i;
	
	for(i=0; i<48; i++){
		image->m01[i] = origin->m01[relabelling[i]];
		image->chambers[i][0] = reverse_labelling[origin->chambers[relabelling[i]][0]];
		image->chambers[i][1] = reverse_labelling[origin->chambers[relabelling[i]][1]];
		image->chambers[i][2] = reverse_labelling[origin->chambers[relabelling[i]][2]];
	}
}

/*canon_symbol contains the canonical form of symbol when this method returns
 */
void canonical_form(struct delaney *symbol, struct delaney *canon_symbol){
	int i;
	int  relabelling[48];
	int found=0; //true when we found a possible candidate
	for(i=0; i<48; i++){
		if(symbol->m01[i]==4){
			// all our symbol contain at least 2 fours so
			// we can only have a canonical form if we start
			// with such a chamber
			canonical_chamber_relabelling(symbol, relabelling, i);
			if(found){
			} 
			
		}
		
	
	}
}

/*****************************************************************************/

void printDelaney(struct delaney *symbol){
	int i;
	fprintf(stdout, "|    | s0 | s1 | s2 | m01 | m12 |\n");
	fprintf(stdout, "|===============================|\n");
	for(i = 0; i<48; i++)
		fprintf(stdout, "| %2d | %2d | %2d | %2d | %3d | %3d |\n", i, symbol->chambers[i][0], symbol->chambers[i][1], symbol->chambers[i][2], symbol->m01[i], 3);
	fprintf(stdout, "|===============================|\n");
	fprintf(stdout, "\n\n");
}

int check_sigma0sigma1orbit(struct delaney *symbol, int chamber){
	int j;
	for(j = 0; j<2; j++){
		int i = 0;
		int next = symbol->chambers[chamber][i];
		int size = 1;
		while(next != -1 && next != chamber && symbol->m01[next]==symbol->m01[chamber]){
			i = (i+1)%2;
			size++;
			next = symbol->chambers[next][i];
		}
		if(next != -1 && next != chamber)
			return 0;
		if(next == chamber && (2*symbol->m01[chamber])%size!=0)
			return 0;
		else if(size > 2*symbol->m01[chamber])
			return 0;
	}
	return 1;
}

void complete_sigma0(struct delaney *symbol){
	int i = 16;
	while(symbol->chambers[i][0]!=-1 && i<47)
		i+=4;
	if(i==48){
		int j = 16;
		while(j<47 && check_sigma0sigma1orbit(symbol, j))
			j += 4;
		if(j==48){
			//new symbol!
			counter4++;
			printDelaney(symbol);
		}
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

void fill_m01(int *m01, struct delaney *symbol){
	int i, j;
	for(i=0; i<8; i++)
		for(j=0; j<4; j++)
			symbol->m01[16 + i*4 + j] = m01[i];
}

void basicDelaney(struct delaney *symbol){
	int i;
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

int isCanonical(int* circularAssignment){
	int i;
	for(i = 1; i<8; i++){
		int j = 0;
		while(j < 8 && *(circularAssignment + j) == *(circularAssignment + (j + i)%8))
			j++;
		if(j<8 && *(circularAssignment + j) > *(circularAssignment + (j + i)%8)) {
			return 0; //not canonical
		}
	}
	return 1;
}

void assign_m01(int* circularAssignment, int position, struct bucket_collection *collection){
	int i;
	for(i = 0; i< collection->collection_size;i++){
		if((collection->collection + i)->size>0){
			*(circularAssignment + position) = (collection->collection + i)->colour;
			(collection->collection + i)->size--;
			if(position<7)
				assign_m01(circularAssignment, position+1, collection);
			else {
				//fprintf(stderr, "new circular string: %d - %d\n", counter, ++counter2);
				if(isCanonical(circularAssignment)){
					counter3++;
					counter2++;
					//fprintf(stdout, "%2d - %3d[%3d])\t", counter, counter2, counter3);
					//printintarray(circularAssignment, 8);
					struct delaney symbol;
					basicDelaney(&symbol);
					fill_m01(circularAssignment,&symbol);
					//printDelaney(&symbol);
					complete_sigma0(&symbol);
				}
			}
			(collection->collection + i)->size++;
		}
	}
}

void create_bucket_collection(struct bucket_collection *collection, int array[]){
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
		create_bucket_collection(&coll, array);
		counter2=0;
		assign_m01(ass, 0, &coll);
		fprintf(stderr, "%d : %d\n", counter, counter2);
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
        int array[SIZE+2];
        int i;
        for(i = 0; i<SIZE+2;i++)
                array[i]=MIN;
        while(array[SIZE - 1] < SIZE)
                tick(array, SIZE - 1);
		fprintf(stderr, "Found %d canonical circular strings\n", counter3);
		fprintf(stderr, "Found %d symbols\n", counter4);
		/*
		struct delaney symbol;
		basicDelaney(&symbol);
		printDelaney(&symbol);*/
        return 0;
}
