/*
 *  azul.c
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 * 
 *  gcc azul.c basicmath.c basicdelaney.o periodicgraph.o -o azul -Wall
 *
 */

#include "basicmath.h"
#include "utility.h"
#include "basicdelaney.h"
#include "periodicgraph.h"
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

DELANEY_COLLECTION octagon_library;
DELANEY_COLLECTION minimal_octagon_library;
DELANEY_COLLECTION azulenoid_library;
DELANEY_COLLECTION minimal_azulenoid_library;
DELANEY_COLLECTION minimal_azulenoid_library_unmarked; //unmarked versions of the tilings in minimal_azulenoid_library
DELANEY_COLLECTION minimal_unmarked_azulenoid_library; //minimal, unmarked versions of the tilings in minimal_azulenoid_library
DELANEY_COLLECTION translation_only_azulenoid_library;

int marked = 0;

int counter = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;
int countercanonical = 0;
int countercanonical2 = 0;

/*
 * Check to see if m01 is constant on the sigma_0 sigma_1 - orbits and checks the size of the orbits.
 * Only for debugging purposes: can be removed if everything is OK.
 */
int checkConsistency(DELANEY *symbol){
	int i, chamber=0;
	int marker[symbol->size];
	//set markers to 0
	for(i=0; i<symbol->size; i++) marker[i]=0;
	while(chamber<symbol->size){
		if(!(marker[chamber])){
			int j=0, next = symbol->chambers[chamber][j], size = 1;
			while(next!=chamber || j!=1){
				if(symbol->m[chamber][0]!=symbol->m[next][0]) return 0;
				marker[next]=1;
				if(j) size++;
				j = (j+1)%2;
				next = symbol->chambers[next][j];
			}
			marker[chamber]=1;
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
		for(j=0;j<60;j++) azulenoid.color[j]=0; //set all marks to zero

		for(j=48;j<60;j++) azulenoid.m[j][1]=3;
		for(j=1;j<7;j++) azulenoid.m[(j+2*i)%16][0]=5;
		for(j=7;j<17;j++) azulenoid.m[(j+2*i)%16][0]=7;
		for(j=48;j<52;j++) azulenoid.m[j][0]=5;
		for(j=52;j<56;j++) azulenoid.m[j][0]=7;

		if(marked){
			//mark azulene
			for(j=1;j<7;j++) azulenoid.color[(j+2*i)%16]=1;
			for(j=7;j<17;j++) azulenoid.color[(j+2*i)%16]=1;
			for(j=48;j<52;j++) azulenoid.color[j]=1;
			for(j=52;j<56;j++) azulenoid.color[j]=1;
		}
		
		int start = symbol->chambers[(1+2*i)%16][2];
		int newValue = symbol->m[start][0]+2*symbol->m[start][0]/getChambersInOrbit(symbol, start, 0, 1);
		azulenoid.m[start][0]=newValue;
		int j=0, next = symbol->chambers[start][0];
		while(next!=start || j!=1){
			azulenoid.m[next][0]=newValue;
			j = (j+1)%2;
			next = symbol->chambers[next][j];
		}

		j=0;
		start = symbol->chambers[(6+2*i)%16][2];
		newValue = azulenoid.m[start][0]+2*symbol->m[start][0]/getChambersInOrbit(symbol, start, 0, 1);
		azulenoid.m[start][0]=newValue;
		next = symbol->chambers[start][0];
		while(next!=start || j!=1){
			azulenoid.m[next][0]=newValue;
			j = (j+1)%2;
			next = symbol->chambers[next][j];
		}

		//moved this too after second adjustement.
		//this was otherwise not adjusted when they were
		//in the same face as the second adjustement
		azulenoid.m[56][0]=azulenoid.m[symbol->chambers[(1+2*i)%16][2]][0];
		azulenoid.m[57][0]=azulenoid.m[symbol->chambers[(1+2*i)%16][2]][0];

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

		//if(checkConsistency(&azulenoid)) {
			addSymbol2Library(&azulenoid, &azulenoid_library);
		//} else {
			//for debugging purposes: normally shouldn't get here if everything is ok.
		//	printDelaney(symbol, stdout);
		//	printDelaney(&azulenoid, stdout);
		//}
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
		if(addSymbol2Library(symbol, &octagon_library))
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
		symbol->color[i] = 0;
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
	//check mirror string
	int i = 0;
	while(i < 8 && *(circularString + i) == *(circularString + (-i + 8)%8))
		i++;
	if(i<8 && *(circularString + i) > *(circularString + (-i + 8)%8)) {
		return 0; //not canonical
	}
	
	for(i = 1; i<8; i++){
		int j = 0;
		while(j < 8 && *(circularString + j) == *(circularString + (j + i)%8))
			j++;
		if(j<8 && *(circularString + j) > *(circularString + (j + i)%8)) {
			return 0; //not canonical
		}
		
		//check mirror string
		j = 0;
		while(j < 8 && *(circularString + j) == *(circularString + (-j + i + 8)%8))
			j++;
		if(j<8 && *(circularString + j) > *(circularString + (-j + i + 8)%8)) {
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
	int marker[symbol->size]; //used to check if octagon is spanning (all vertices in octagon)
	int marker2[symbol->size]; //used to mark a single octagon
	int marker3[symbol->size]; //used to track which octagons are already visited
	
	//reset marker3
	for(j = 0; j < symbol->size; j++)
		marker3[j]=0;

	for(i=0; i< symbol->size; i++){
		if(symbol->m[i][0]==8 && !marker3[i]){
			markorbit(symbol, marker3, i, 0, 1, 0); //visit this octagon
			markorbit(symbol, marker2, i, 0, 1, 1); //track this octagon

			//reset marker
			for(j = 0; j < symbol->size; j++)
				marker[j]=0;
			
			for(j=0;j<symbol->size;j++)
				if(marker2[j] && !marker[j])
					markorbit(symbol, marker, j, 1, 2, 0);

			j=0;
			while(j<symbol->size && marker[j])
				j++;
			if(j==symbol->size){
				//found spanning octagon
				spanning++;
				
				j=0;
				while(j<symbol->size && !(marker2[j] && marker2[symbol->chambers[j][2]]))
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

void exportOnlyTranslation(int pg){
	int i;
	int j=1;
	for(i=0;i<azulenoid_library.size;i++)
		if(hasOnlyTranslation(azulenoid_library.collection + i)){
			if(!pg)
				exportDelaneyNumbered(azulenoid_library.collection + i, j++, (azulenoid_library.collection + i)->comment1, stdout);
			else{
				PeriodicGraph graph;
				if(createPeriodicGraph(azulenoid_library.collection + i, &graph)){
					exportPeriodicGraph(&graph, stdout, 0);
					fprintf(stdout, " # symbol ");
					exportDelaney(azulenoid_library.collection + i,stdout);
				}
			}
		}
}

struct face_enum{
	int faces[20];
	int size;
};

struct face_enum_collection{
	struct face_enum collection[200];
	int frequency[200];
	int collection_size;
};

void addFace2Enum(struct face_enum *faceEnum, int face){
	faceEnum->faces[faceEnum->size++]=face;
}

void canonicalFaceEnum(struct face_enum *original, struct face_enum *canon){
	int i,j;
	canon->size=0;
	for(i=4;i<=36;i++)
		for(j=0;j<original->size;j++)
			if(original->faces[j]==i)
				addFace2Enum(canon, i);
}

int faceEnumEqual(struct face_enum *faceEnum1, struct face_enum *faceEnum2){
	if(faceEnum1->size!=faceEnum2->size)
		return 0;
	int i;
	for(i=0;i<faceEnum1->size;i++)
		if(faceEnum1->faces[i]!=faceEnum2->faces[i])
			return 0;
	return 1;
}

void addFaceEnum2Collection(struct face_enum *faceEnum, struct face_enum_collection *collection){
	int i;
	canonicalFaceEnum(faceEnum, (collection->collection + (collection->collection_size)));
	for(i=0;i<collection->collection_size;i++)
		if(faceEnumEqual(collection->collection + i, collection->collection + collection->collection_size)){
			(*(collection->frequency + i))++;
			return;
		}
	*(collection->frequency + (collection->collection_size)) = 1;
	collection->collection_size++;
}

void printLibraryFaceSummary(DELANEY_COLLECTION *library, FILE *f){
	int i, j;
	int max = 0;
	int max_num = 0;
	struct face_enum faceEnum;
	struct face_enum_collection collection;
	collection.collection_size=0;
	for(i=0;i<library->size;i++){
		faceEnum.size = 0;
		DELANEY *symbol = library->collection + i;
		int marker[symbol->size];
		for(j = 0; j < symbol->size; j++)
			marker[j]=0;
		for(j = 0; j < symbol->size; j++){
			if(!marker[j]){
				addFace2Enum(&faceEnum, symbol->m[j][0]);
				if(symbol->m[j][0] > max)
					max = symbol->m[j][0];
				markorbit(symbol, marker, j, 0, 1, 0);
			}
		}
		addFaceEnum2Collection(&faceEnum, &collection);
		if(faceEnum.size>max_num)
			max_num=faceEnum.size;
	}
	fprintf(f, "\nmaximum size of a face: %d\nmaximum number of faces: %d\n", max, max_num);
	fprintf(f, "Number of different face collections: %d\n\n", collection.collection_size);
	
	int sum = 0;

	for(i=0;i<collection.collection_size;i++){
		fprintf(f, "%3d) %3d fundamental tiles with faces of order", i+1, *(collection.frequency + i));
		sum += *(collection.frequency + i);
		for(j=0;j<(collection.collection+i)->size;j++)
			fprintf(f, " %d", *((collection.collection+i)->faces + j));
		fprintf(f, " \n");
	}
	fprintf(f, "sum: %d \n", sum);
}

int getMaximumFaceSize(DELANEY *symbol){
	int marker[symbol->size];
	int j, max = 0;
	for(j = 0; j < symbol->size; j++)
		marker[j]=0;
	for(j = 0; j < symbol->size; j++){
		if(!marker[j]){
			if(symbol->m[j][0] > max)
				max = symbol->m[j][0];
			markorbit(symbol, marker, j, 0, 1, 0);
		}
	}
	return max;
}

void exportRestrictedSymbols(DELANEY_COLLECTION *library, FILE *f, int maximumSize){
	int i, count = 1;
	for(i=0;i<library->size;i++){
		if(getMaximumFaceSize(library->collection + i)<=maximumSize)
			exportDelaneyNumbered(library->collection + i, 1, count++, f);
	}
}

//investigate how many tilings are isomorphic as unmarked tilings, but not as marked tilings
void deepSummaryUnmarkedMarked(){
	//we look at the minimal azulenoid library, remove the marks and check for isomorphic structures

	//there is one entry in mapping for each element in the unmarked library
	//at position 0 is the number of marked tilings that correspond to that unmarked tiling
	//the following positions contain the numbers of the marked tilings
	int mapping[minimal_azulenoid_library.size][20];
	int i,j;
	minimal_azulenoid_library_unmarked.size=0;
	for(i = 0; i<minimal_azulenoid_library.size;i++){
		//clean mapping while going through the library
		//there will never be more mappings then elements we have viewed.
		mapping[i][0]=0;
		DELANEY unmarked;
		copyDelaney(minimal_azulenoid_library.collection + i, &unmarked);
		for(j=0;j<unmarked.size;j++){ //remove marks
			unmarked.color[j]=0;
		}
		
		int target = addSymbol2LibraryPosition(&unmarked, &minimal_azulenoid_library_unmarked);
		
		mapping[target][0]++;
		mapping[target][mapping[target][0]]=i;
	}
	
	//todo: maybe make a frequency table of the number of marked tilings per unmarked tilings
	//this seems to always be one or two
	int count = 0;
	int count2 = 0;
	for(i = 0; i<minimal_azulenoid_library_unmarked.size;i++){
		if(mapping[i][0]>1){
			count = count + mapping[i][0];
			count2++;
			exportDelaney(minimal_azulenoid_library_unmarked.collection+i,stdout);
		} else if(mapping[i][0]<1) {
			fprintf(stderr, "Illegal number: %d\n", mapping[i][0]);
		}
	}
	
	fprintf(stderr, "There are %d tilings that are isomorph as unmarked tilings, but not as marked tilings.\n", count);
	fprintf(stderr, "There are %d unmarked tilings that have nonisomorphic marked tilings.\n", count2);
}

//investigate how many tilings have isomorphic minimal tilings as unmarked tilings, but not as marked tilings
void deepSummaryUnmarkedMinimal(){
	//we look at the minimal azulenoid library, remove the marks and check for isomorphic, minimal structures

	//there is one entry in mapping for each element in the unmarked library
	//at position 0 is the number of marked tilings that correspond to that unmarked tiling
	//the following positions contain the numbers of the marked tilings
	int mapping[minimal_azulenoid_library.size][20];
	int i,j;
	minimal_azulenoid_library_unmarked.size=0;
	for(i = 0; i<minimal_azulenoid_library.size;i++){
		//clean mapping while going through the library
		//there will never be more mappings then elements we have viewed.
		mapping[i][0]=0;
		DELANEY unmarked;
		copyDelaney(minimal_azulenoid_library.collection + i, &unmarked);
		for(j=0;j<unmarked.size;j++){ //remove marks
			unmarked.color[j]=0;
		}
		DELANEY minimal;
		minimal_delaney(&unmarked, &minimal);
		
		int target = addSymbol2LibraryPosition(&minimal, &minimal_unmarked_azulenoid_library);
		
		mapping[target][0]++;
		mapping[target][mapping[target][0]]=i;
	}
	
	//todo: maybe make a frequency table of the number of marked tilings per unmarked tilings
	//this seems to always be one or two
	int count = 0;
	int count2 = 0;
	int max = 0;
	for(i = 0; i<minimal_unmarked_azulenoid_library.size;i++){
		if(max<mapping[i][0])
			max = mapping[i][0];
		if(mapping[i][0]>1){
			count = count + mapping[i][0];
			count2++;
		} else if(mapping[i][0]<1) {
			fprintf(stderr, "Illegal number: %d\n", mapping[i][0]);
		}
	}
	
	int frequency[max+1];
	for(i = 0; i<max+1;i++)
		frequency[i]=0;
	for(i = 0; i<minimal_unmarked_azulenoid_library.size;i++){
		frequency[mapping[i][0]]++;
	}
	
	for(i = 0; i<max+1;i++)
		fprintf(stderr, "There are %d unmarked minimal tilings that have %d marked ancestors.\n", frequency[i] , i);
	fprintf(stderr, "There are %d tilings that have isomorphic minimal tilings as unmarked tilings, but not as marked tilings.\n", count);
	fprintf(stderr, "There are %d unmarked minimal tilings that have more than one marked tilings as ancestors.\n", count2);
	for(i = 0; i<minimal_unmarked_azulenoid_library.size;i++)
		if(mapping[i][0]>3)
			for(j=1; j<=mapping[i][0]; j++)
				exportDelaney(minimal_azulenoid_library.collection+mapping[i][j],stdout);

}

int parseNumber(char *argv[]){
	int c, stop=0, number = 0;
    while(!stop && (c = *++argv[0])){
		switch (c) {
			case '0':
				number = number*10;
				break;
			case '1':
				number = number*10 + 1;
				break;
			case '2':
				number = number*10 + 2;
				break;
			case '3':
				number = number*10 + 3;
				break;
			case '4':
				number = number*10 + 4;
				break;
			case '5':
				number = number*10 + 5;
				break;
			case '6':
				number = number*10 + 6;
				break;
			case '7':
				number = number*10 + 7;
				break;
			case '8':
				number = number*10 + 8;
				break;
			case '9':
				number = number*10 + 9;
				break;
			case '_':
				stop=1;
				break;
			default:
				fprintf(stderr, "Error while parsing number: %c\n", c);
				stop = 1;
				number = 0;
				break;
		}
	}
	return number;
}

/*******************************************************/

int main(int argc, char *argv[])
{
	int export_minimal = 0;
	int export_octagon = 0;
	int export_azulenoid = 0;
	int export_onlytranslation = 0;
	int export_summary = 0;
	int export_restricted = 0;
    int restriction = 0;
	int export_cover = 0;
	int export_pg = 0;
	int export_deepsummary = 0;
	int c, error = 0;
	while (--argc > 0 && (*++argv)[0] == '-'){
		while ((c = *++argv[0]))
			switch (c) {
			case 'm':
				export_minimal = 1;
				break;
			case 'o':
				export_octagon = 1;
				break;
			case 'a':
				export_azulenoid = 1;
				break;
			case 't':
				export_onlytranslation = 1;
				break;
			case 's':
				export_summary = 1;
				break;
			case 'r':
				export_restricted = 1;
				restriction = parseNumber(argv);
				break;
			case 'd':
				export_deepsummary = parseNumber(argv);
				break;
			case 'c':
				export_cover = 1;
				break;
			case 'p':
				export_pg = 1;
				break;
			case 'M':
				marked = 1;
				break;
			case 'h':
				//print help
				fprintf(stderr, "The program azul calculates toroidal azulenoids.\n");
				fprintf(stderr, "Usage: azul [-moatsrcph]\n\n");
				fprintf(stderr, "Valid options:\n");
				fprintf(stderr, "  -m          : Output minimal symbols.\n");
				fprintf(stderr, "  -o          : Output the calculated octagon tilings.\n");
				fprintf(stderr, "  -a          : Output the calculated azulenoids.\n");
				fprintf(stderr, "  -t          : Output only azulenoids who's isometry group consists of only translations.\n");
				fprintf(stderr, "  -c          : Output translation only covers.\n");
				fprintf(stderr, "  -p          : Output periodic graphs.\n");
				fprintf(stderr, "  -s          : Output a summary.\n");
				fprintf(stderr, "  -d(number)_ : Output a deep summary.\n");
				fprintf(stderr, "                 1) marked tilings that are isomorphic as unmarked tilings.\n");
				fprintf(stderr, "                 2) marked tilings that have isomorphic minimal tilings as unmarked tilings.\n");
				fprintf(stderr, "  -r(number)_ : Output the calculated azulenoids restricted to a maximum face size.\n");
				fprintf(stderr, "  -h          : Print this help and return.\n");
				return 0;
			default:
				fprintf(stderr, "azul: illegal option %c\n", c);
				argc = 0;
				error = 1;
				break;
			}
	}
	if(error || argc != 0){
		fprintf(stderr, "Usage: azul [options]. Use azul -h for more information.\n");
		return 1;
	}
	
	octagon_library.size=0;
	minimal_octagon_library.size=0;
	azulenoid_library.size=0;
	minimal_azulenoid_library.size=0;
	
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
	fprintf(stderr, "Found %d canonical symbols\n", octagon_library.size);
	
	//calculate minimal octagon tilings
	
	DELANEY temp_minimal;
	for(i = 0; i<octagon_library.size;i++){
		minimal_delaney(octagon_library.collection+i,&temp_minimal);
		addSymbol2Library(&temp_minimal, &minimal_octagon_library);
	}
	fprintf(stderr, "Found %d minimal, canonical symbols\n\n", minimal_octagon_library.size);
	
	if(export_octagon){
		if(export_minimal)
			exportLibrary(&minimal_octagon_library, 1, stdout);
		else
			exportLibrary(&octagon_library, 1, stdout);
	}
	
	
	//determine size of minimal symbols
	int frequency[48];
	for(i=0;i<48;i++){
		frequency[i]=0;
	}
	for(i=0;i<minimal_octagon_library.size;i++){
		frequency[(minimal_octagon_library.collection + i)->size-1]++;
	}
	
	for(i=0;i<48;i++){
		if(frequency[i])
			fprintf(stderr, "%3d symbols with %2d chambers\n", frequency[i], i+1);
	}
	
	//determine spanning octagons
	
	fprintf(stderr, "\nLooking for spanning octagons in canonical symbols.\n");
	
	for(i=0;i<minimal_octagon_library.size;i++){
		if(countSpanningOctagons(minimal_octagon_library.collection + i)!=1){
			fprintf(stderr, "Symbol with %d spanning octagons.\n", countSpanningOctagons(minimal_octagon_library.collection + i));
		}
	}
	fprintf(stderr, "\nFound %d symbol%s that contain%s a not spanning octagon.\n", counternotspanning, counternotspanning == 1 ? "" : "s",  counternotspanning == 1 ? "s" : "");
	fprintf(stderr, "\nFound %d symbol%s that contain%s equivalent spanning octagons.\n", counterequivalentoctagons, counterequivalentoctagons == 1 ? "" : "s",  counterequivalentoctagons == 1 ? "s" : "");

	//azulenoids
	
	fprintf(stderr, "\nAzulenoids\n");
	fprintf(stderr, "==========\n");
	
	fprintf(stderr, "\nFound %d canonical azulenoids.\n", azulenoid_library.size);
	
	for(i = 0; i<azulenoid_library.size;i++){
		minimal_delaney(azulenoid_library.collection+i,&temp_minimal);
		addSymbol2Library(&temp_minimal, &minimal_azulenoid_library);
	}
	fprintf(stderr, "\nFound %d minimal, canonical azulenoids.\n", minimal_azulenoid_library.size);
	if(export_azulenoid){
		if(export_minimal)
			exportLibrary(&minimal_azulenoid_library, 1, stdout);
		else
			exportLibrary(&azulenoid_library, 1, stdout);
	}
	
	if(export_onlytranslation){
		exportOnlyTranslation(export_pg);
	}
	
	if(export_restricted){
		exportRestrictedSymbols(&minimal_azulenoid_library, stdout, restriction);
	}
	
	if(export_summary){
		printLibraryFaceSummary(&minimal_azulenoid_library, stdout);
	}

	int max = 0;
	int j;
	for(i = 0; i<minimal_azulenoid_library.size;i++){
		DELANEY symbol;
		makeOrientable(minimal_azulenoid_library.collection+i, &symbol);
		for(j=0; j<symbol.size;j++){
			int temp = getV(&symbol, j, 0, 1)*symbol.size;
			if(temp>max)
				max=temp;
		}
	}
	for(i = 0; i<minimal_azulenoid_library.size;i++){
		DELANEY symbol;
		makeOrientable(minimal_azulenoid_library.collection+i, &symbol);
		for(j=0; j<symbol.size;j++){
			int temp = getV(&symbol, j, 0, 2)*symbol.size;
			if(temp>max)
				max=temp;
		}
	}
	for(i = 0; i<minimal_azulenoid_library.size;i++){
		DELANEY symbol;
		makeOrientable(minimal_azulenoid_library.collection+i, &symbol);
		for(j=0; j<symbol.size;j++){
			int temp = getV(&symbol, j, 1, 2)*symbol.size;
			if(temp>max)
				max=temp;
		}
	}
	if(export_cover){
		for(i = 0; i<minimal_azulenoid_library.size;i++){
			DELANEY symbol;
			fprintf(stderr, "%d \n", i);
			if(makeOnlyTranslation(minimal_azulenoid_library.collection+i, &symbol))
				addSymbol2Library(&symbol, &translation_only_azulenoid_library);
		}
		if(export_pg) {
			for(i = 0; i<translation_only_azulenoid_library.size;i++){
				PeriodicGraph graph;
				if(createPeriodicGraph(translation_only_azulenoid_library.collection + i, &graph)){
					exportPeriodicGraph(&graph, stdout, 0);
					fprintf(stdout, " # symbol ");
					exportDelaney(minimal_azulenoid_library.collection+i,stdout);
				}
			}
		} else {
			exportLibrary(&translation_only_azulenoid_library, 1, stdout);
		}
	}
	if(export_deepsummary==1){
		deepSummaryUnmarkedMarked();
	} else if(export_deepsummary==2){
		deepSummaryUnmarkedMinimal();
	}
	return 0;
}
