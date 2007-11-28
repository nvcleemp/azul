/*
 *  basicmath.c
 *  
 *
 *  Created by Nico Van Cleemput on 07/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "basicmath.h"

int gcd(int a, int b){
    while(b!=0){
        int temp = b;
		b = a%b;
        a = temp;
	}
    return a;
}
    
int lcm(int a, int b){
	return (a*b)/gcd(a,b);
}
