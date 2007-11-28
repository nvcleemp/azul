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

int counter = 0;
int counter2 = 0;
int counter3 = 0;

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
					fprintf(stdout, "%2d - %3d[%3d])\t", counter, counter2, counter3);
					printintarray(circularAssignment, 8);
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
        return 0;
}
