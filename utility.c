/*
 *  utility.h
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 *
 */

#include "utility.h"
#include <stdio.h>

void printintarray(int array[], int size){
	int i;
    for(i = 0; i <size; i++)
		printf("%2d  ", array[i]);
	printf("\n");
}