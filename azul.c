/*
 *  azul.c
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 * 
 *  gcc azul.c basicmath.c basicdelaney.o -o azul -Wall
 *
 */

#include "basicmath.h"
#include "utility.h"
#include "basicdelaney.h"
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

DELANEY_COLLECTION library;
DELANEY_COLLECTION minimal_library;
DELANEY_COLLECTION azulenoid_library;
DELANEY_COLLECTION minimal_azulenoid_library;

int counter = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;
int countercanonical = 0;
int countercanonical2 = 0;

/*****************************************************************************/

void markorbit(DELANEY *symbol, int chamber, int i, int j){
	symbol->marker[chamber]=1;
	symbol->marker[symbol->chambers[chamber][i]]=1;
	int next = symbol->chambers[symbol->chambers[chamber][i]][j];
	while(next!=chamber){
		symbol->marker[next]=1;
		symbol->marker[symbol->chambers[next][i]]=1;
		next = symbol->chambers[symbol->chambers[next][i]][j];	
	}
}

/*
 * Check to see if m01 is constant on the sigma_0 sigma_1 - orbits and checks the size of the orbits.
 * Only for debugging purposes: can be removed if everything is OK.
 */
int checkConsistency(DELANEY *symbol){
	int i, chamber=0;
	//set markers to 0
	for(i=0; i<symbol->size; i++) symbol->marker[i]=0;
	while(chamber<symbol->size){
		if(!(symbol->marker[chamber])){
			int j=0, next = symbol->chambers[chamber][j], size = 1;
			while(next!=chamber || j!=1){
				symbol->marker[next]=1;
				if(j) size++;
				j = (j+1)%2;
				next = symbol->chambers[next][j];
			}
			symbol->marker[chamber]=1;
			if((2*symbol->m[chamber][0])%size!=0) return 0;
		}
		chamber++;
	}
	return 1;
}

/*
 * Insert azulene
 */
void insertAzulene(DELANEY *symbol){
	int i, j;
	for(i=0; i<8; i++){
		DELANEY azulenoid;
		copyDelaney(symbol, &azulenoid);
		azulenoid.size=60;
		for(j=48;j<60;j++) azulenoid.m[j][1]=3;
		for(j=1;j<7;j++) azulenoid.m[(j+2*i)%16][0]=5;
		for(j=7;j<17;j++) azulenoid.m[(j+2*i)%16][0]=7;
		for(j=48;j<52;j++) azulenoid.m[j][0]=5;
		for(j=52;j<56;j++) azulenoid.m[j][0]=7;
		
		int start = symbol->chambers[(1+2*i)%16][2];
		int newValue = symbol->m[start][0]+2*symbol->m[start][0]/getChambersInOrbit(symbol, start, 0, 1);
		//fprintf(stderr, "DEBUG: %d / %d => %d\n", symbol->m01[start],getOrbitSize(symbol, start, 0, 1), symbol->m01[start]/getOrbitSize(symbol, start, 0, 1));
		azulenoid.m[start][0]=newValue;
		int j=0, next = symbol->chambers[start][0];
		while(next!=start || j!=1){
			azulenoid.m[next][0]=newValue;
			j = (j+1)%2;
			next = symbol->chambers[next][j];
			//fprintf(stderr, "here, with %d and %d\n", next, j);
		}
		azulenoid.m[56][0]=newValue;
		azulenoid.m[57][0]=newValue;

		j=0;
		start = symbol->chambers[(6+2*i)%16][2];
		newValue = azulenoid.m[start][0]+2*symbol->m[start][0]/getChambersInOrbit(symbol, start, 0, 1);
		//fprintf(stderr, "DEBUG: %d / %d => %d\n", symbol->m01[start],getOrbitSize(symbol, start, 0, 1), symbol->m01[start]/getOrbitSize(symbol, start, 0, 1));
		azulenoid.m[start][0]=newValue;
		next = symbol->chambers[start][0];
		while(next!=start || j!=1){
			azulenoid.m[next][0]=newValue;
			j = (j+1)%2;
			next = symbol->chambers[next][j];
		}
		azulenoid.m[58][0]=newValue;
		azulenoid.m[59][0]=newValue;

		int chamber1 = symbol->chambers[(1+2*i)%16][2];
		int chamber2 = symbol->chambers[(6+2*i)%16][2];
		int chamber3 = symbol->chambers[(7+2*i)%16][2];
		int chamber4 = symbol->chambers[(16+2*i)%16][2];

		azulenoid.chambers[(1+2*i)%16][0]=51;
		azulenoid.chambers[(6+2*i)%16][0]=48;
		azulenoid.chambers[(7+2*i)%16][0]=55;
		azulenoid.chambers[(16+2*i)%16][0]=52;
		azulenoid.chambers[chamber1][0]=56;
		azulenoid.chambers[chamber2][0]=58;
		azulenoid.chambers[chamber3][0]=59;
		azulenoid.chambers[chamber4][0]=57;

		azulenoid.chambers[48][0]=(6+2*i)%16;
		azulenoid.chambers[48][1]=49;
		azulenoid.chambers[48][2]=58;
		azulenoid.chambers[49][0]=50;
		azulenoid.chambers[49][1]=48;
		azulenoid.chambers[49][2]=54;
		azulenoid.chambers[50][0]=49;
		azulenoid.chambers[50][1]=51;
		azulenoid.chambers[50][2]=53;
		azulenoid.chambers[51][0]=(1+2*i)%16;
		azulenoid.chambers[51][1]=50;
		azulenoid.chambers[51][2]=56;
		azulenoid.chambers[52][0]=(16+2*i)%16;
		azulenoid.chambers[52][1]=53;
		azulenoid.chambers[52][2]=57;
		azulenoid.chambers[53][0]=54;
		azulenoid.chambers[53][1]=52;
		azulenoid.chambers[53][2]=50;
		azulenoid.chambers[54][0]=53;
		azulenoid.chambers[54][1]=55;
		azulenoid.chambers[54][2]=49;
		azulenoid.chambers[55][0]=(7+2*i)%16;
		azulenoid.chambers[55][1]=54;
		azulenoid.chambers[55][2]=59;
		azulenoid.chambers[56][0]=chamber1;
		azulenoid.chambers[56][1]=57;
		azulenoid.chambers[56][2]=51;
		azulenoid.chambers[57][0]=chamber4;
		azulenoid.chambers[57][1]=56;
		azulenoid.chambers[57][2]=52;
		azulenoid.chambers[58][0]=chamber2;
		azulenoid.chambers[58][1]=59;
		azulenoid.chambers[58][2]=48;
		azulenoid.chambers[59][0]=chamber3;
		azulenoid.chambers[59][1]=58;
		azulenoid.chambers[59][2]=55;

		if(checkConsistency(&azulenoid)) {
			addSymbol2Library(&azulenoid, &azulenoid_library);
		} else {
			//for debugging purposes: normally shouldn't get here if everything is ok.
			printDelaney(symbol, stdout);
			printDelaney(&azulenoid, stdout);
		}
	}
}

/*
 * Check to see if m01 is constant on the sigma_0 sigma_1 - orbit through chamber and checks the size of that orbit.
 */
int check_sigma0sigma1orbit(DELANEY *symbol, int chamber){
	int j;
	for(j = 0; j<2; j++){
		int i = j;
		int next = symbol->chambers[chamber][i];
		int size = 1;
		while(next != -1 && (next != chamber || i!=(j+1)%2) && symbol->m[next][0]==symbol->m[chamber][0]){
			i = (i+1)%2;
			size++;
			next = symbol->chambers[next][i];
		}
		if(next != -1 && (next != chamber || i!=(j+1)%2))
			return 0;
		if(next == chamber && i==(j+1)%2 && (2*symbol->m[chamber][0])%size!=0)
			return 0;
		else if(size > 2*symbol->m[chamber][0])
			return 0;
	}
	return 1;
}

/*
 * Construct the remaining sigma_0 edges.
 */
void complete_sigma0(DELANEY *symbol){
	int i = 16;
	while(symbol->chambers[i][0]!=-1 && i<47)
		i+=4;
	if(i==48){
		//new symbol!
		counter4++;
		insertAzulene(symbol);
		if(addSymbol2Library(symbol, &library))
			countercanonical++;
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
		if(symbol->m[i][0]==symbol->m[symbol->chambers[i][2]][0]){
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
				if(symbol->m[i][0]==symbol->m[j][0] &&
				   symbol->m[symbol->chambers[i][2]][0]==symbol->m[symbol->chambers[j][2]][0]){
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
				if(symbol->m[i][0]==symbol->m[symbol->chambers[j][2]][0] &&
				   symbol->m[symbol->chambers[i][2]][0]==symbol->m[j][0]){
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
void fill_m01(int *m01, DELANEY *symbol){
	int i, j;
	for(i=0; i<8; i++)
		for(j=0; j<4; j++)
			symbol->m[16 + i*4 + j][0] = m01[i];
}

/*
 * Construct the basic Delaney symbol for the tiling. See text.
 */
void basicDelaney(DELANEY *symbol){
	int i;
	symbol->size = 48;
	for(i = 0; i< 48; i++){
		symbol->chambers[i][0] = -1;
		symbol->chambers[i][1] = -1;
		symbol->chambers[i][2] = -1;
		symbol->m[i][0] = -1;
		symbol->m[i][1] = 3;
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
		
		symbol->m[i][0]=8;
		symbol->m[i-1][0]=8;
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
					DELANEY symbol;
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
		fprintf(stderr, "%2d) %3d canonical strings leading to %3d symbols, %3d canonical. ", counter, counter2, counter4 - counter5, countercanonical - countercanonical2);
		fprintf(stderr, "(%2d %2d %2d %2d %2d %2d %2d %2d)\n", array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]);
		counter5 = counter4;
		countercanonical2 = countercanonical;
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

/*******************************************************/
int counternotspanning = 0;
int counterequivalentoctagons = 0;
int countSpanningOctagons(DELANEY *symbol){
	int i, j;
	int spanning = 0;
	int notspanning = 0;
	int equivalent = 0;
	//reset marker2
	for(j = 0; j < symbol->size; j++)
		symbol->marker2[j]=0;

	for(i=0; i< symbol->size; i++){
		if(symbol->m[i][0]==8 && !symbol->marker2[i]){
			//reset marker
			for(j = 0; j < symbol->size; j++)
				symbol->marker[j]=0;
			
			symbol->marker2[i]=1;
			markorbit(symbol, i, 1, 2);
			int next = symbol->chambers[i][0];
			j=0;
			while(next != i || j!=1){
				j = (j+1)%2;
				markorbit(symbol, next, 1, 2);
				symbol->marker2[next]=1;
				next = symbol->chambers[next][j];
			}
			j=0;
			while(j<symbol->size && symbol->marker[j])
				j++;
			if(j==symbol->size){
				//found spanning octagon
				spanning++;
				
				//reset marker
				for(j = 0; j < symbol->size; j++)
					symbol->marker[j]=0;
				markorbit(symbol, i, 0, 1);
				j=0;
				while(j<symbol->size && !(symbol->marker2[j] && symbol->marker2[symbol->chambers[j][2]]))
					j++;
				if(j<symbol->size)
					equivalent = 1;
			} else {
				notspanning++;
			}
		}
	}
	if(notspanning)
		counternotspanning++;
		
	if(equivalent)
		counterequivalentoctagons++;
	
	return spanning;

}

/*******************************************************/

int main()
{
	
	library.size=0;
	minimal_library.size=0;
	azulenoid_library.size=0;
	fprintf(stderr, "Octagon tilings\n");
	fprintf(stderr, "===============\n");
    int array[SIZE+2];
    int i;
    for(i = 0; i<SIZE+2;i++)
		array[i]=MIN;
	while(array[SIZE - 1] < SIZE)
		tick(array, SIZE - 1);
	fprintf(stderr, "\nFound %d canonical circular strings\n", counter3);
	fprintf(stderr, "Found %d symbols\n", counter4);
	fprintf(stderr, "Found %d canonical symbols\n", library.size);
	DELANEY temp_minimal;
	for(i = 0; i<library.size;i++){
		//fprintf(stderr, "at %d \n", i);
		minimal_delaney(library.collection+i,&temp_minimal);
		addSymbol2Library(&temp_minimal, &minimal_library);
	}
        fprintf(stderr, "Found %d minimal, canonical symbols\n\n", minimal_library.size);
	for(i=0;i<minimal_library.size;i++){
		//exportDelaneyNumbered(minimal_library.collection + i, i+1, i+1);
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
	
	fprintf(stderr, "\nLooking for spanning octagons in canonical symbols.\n");
	
	for(i=0;i<library.size;i++){
		if(countSpanningOctagons(library.collection + i)!=1){
			fprintf(stderr, "Symbol with %d spanning octagons.\n", countSpanningOctagons(library.collection + i));
		}
	}
	fprintf(stderr, "\nFound %d symbol%s that contain%s a not spanning octagon.\n", counternotspanning, counternotspanning == 1 ? "" : "s",  counternotspanning == 1 ? "s" : "");
	fprintf(stderr, "\nFound %d symbol%s that contain%s equivalent spanning octagons.\n", counterequivalentoctagons, counterequivalentoctagons == 1 ? "" : "s",  counterequivalentoctagons == 1 ? "s" : "");

	fprintf(stderr, "\nAzulenoids\n");
	fprintf(stderr, "==========\n");
	
	fprintf(stderr, "\nFound %d canonical azulenoids.\n", azulenoid_library.size);
	
	for(i = 0; i<azulenoid_library.size;i++){
		//fprintf(stderr, "at %d \n", i);
		minimal_delaney(azulenoid_library.collection+i,&temp_minimal);
		addSymbol2Library(&temp_minimal, &minimal_azulenoid_library);
	}
	fprintf(stderr, "\nFound %d minimal, canonical azulenoids.\n", minimal_azulenoid_library.size);
	//for(i=0;i<13;i++){
/*	for(i=0;i<minimal_azulenoid_library.size;i++){
		exportDelaneyNumbered(minimal_azulenoid_library.collection + i, i+1, i+1);
		//printDelaney(minimal_azulenoid_library.collection + i);
	}*/
	for(i=0;i<minimal_azulenoid_library.size;i++){
		//exportDelaneyNumbered(minimal_azulenoid_library.collection + i, i+1, i+1);
		//printDelaney(minimal_azulenoid_library.collection + i);
	}
	
	return 0;
}
