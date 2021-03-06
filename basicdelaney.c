/*
 *  basicdelaney.c
 *  
 *
 *  Created by Nico Van Cleemput on 24/01/08.
 *  Copyright 2008.
 *
 *  Offers some basic operations for two dimensional Delaney symbols.
 *
 *  compile with: 
 *  gcc basicdelaney.c -c
 */
 
 #include "basicdelaney.h"
 
 /*****************************************************************************/

void printDelaney(DELANEY *symbol, FILE *f){
	int i;
	fprintf(f, "|     | s0  | s1  | s2  | m01 | m12 |  c  |\n");
	fprintf(f, "|=========================================|\n");
	for(i = 0; i<symbol->size; i++)
		fprintf(f, "| %3d | %3d | %3d | %3d | %3d | %3d | %3d |\n", i, symbol->chambers[i][0], symbol->chambers[i][1], symbol->chambers[i][2], symbol->m[i][0], symbol->m[i][1], symbol->color[i]);
	fprintf(f, "|=========================================|\n");
	fprintf(f, "\n\n");
}

void exportLibrary(DELANEY_COLLECTION *library, int numbered, FILE *f){
	int i;
	for(i=0;i<library->size;i++){
		if(numbered)
			exportDelaneyNumbered(library->collection + i, i+1, i+1, f);
		else
			exportDelaney(library->collection + i, f);
	}
}

void exportDelaney(DELANEY *symbol, FILE *f){
	exportDelaneyNumbered(symbol, symbol->comment1, symbol->comment2, f);
}

void exportDelaneyNumbered(DELANEY *symbol, int nr1, int nr2, FILE *f){
	int i,j;
	//size information
	fprintf(f, "<%d.%d:%d 2:", nr1, nr2, symbol->size);
	
	//sigma
	fprintf(f, "%d", symbol->chambers[0][0]+1);
	for(i = 1; i < symbol->size; i++){
		if(symbol->chambers[i][0] >= i)
			fprintf(f, " %d", symbol->chambers[i][0]+1);
	}
	for(i = 1; i<3; i++){
		fprintf(f, ",%d", symbol->chambers[0][i]+1);
		for(j = 1; j < symbol->size; j++)
			if(symbol->chambers[j][i] >= j)
				fprintf(f, " %d", symbol->chambers[j][i]+1);
	}
	
	//m01
	fprintf(f, ":");
	int marker[symbol->size];
	for(i = 0; i < symbol->size; i++)
		marker[i]=0;
		
	for(i = 0; i < symbol->size; i++){
		if(!marker[i]){
			fprintf(f, " %d", symbol->m[i][0]);
			markorbit(symbol, marker, i, 0, 1, 0);
		}
	}
	
	
	//m12
	fprintf(f, ",");
	for(i = 0; i < symbol->size; i++)
		marker[i]=0;
		
	for(i = 0; i < symbol->size; i++){
		if(!marker[i]){
			fprintf(f, " %d", symbol->m[i][1]);
			markorbit(symbol, marker, i, 1, 2, 0);
		}
	}
	
	fprintf(f, ">\n");
}

/**
 * returns EOF when EOF is encountered
 */
int readSingleDelaney(DELANEY *symbol, FILE *f){
	int c;
	while((c=getc(f))!='<')
		if(c==EOF)
			return EOF;

	/******************************/
	int d1, d2;
	if(fscanf(f, "%d.%d:", &d1, &d2)==0){
		fprintf(stderr, "Error: Illegal format: Must start with number.number.\n");
		return 0;
	}
	symbol->comment1 = d1;
	symbol->comment2 = d2;
	/******************************/
	if(fscanf(f, "%d", &d1)==0){
		fprintf(stderr, "Error: Illegal format. Cannot read size.\n");
		return 0;
	}
	emptyDelaney(symbol, d1);
	if(fscanf(f, "%d", &d1) && d1 != 2){
		fprintf(stderr, "Error: Currently only Delaney symbols with dimension 2 are supported.\n");
		return 0;
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
	
	return 1;
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

/*****************************************************************************/

void emptyDelaney(DELANEY *symbol, int size){
	int i,j;
	symbol->size = size;
	for(i=0; i < size; i++){
		for(j=0; j < 3; j++)
			symbol->chambers[i][j]=-1;
		symbol->m[i][0]=-1;
		symbol->m[i][1]=-1;
		symbol->color[i]=0;
	}
}

/*****************************************************************************/
/*
 * Collapses the two chambers and if this method returns true then partition
 * contains the resulting partition.
 */
int collapse(DELANEY *symbol, int chamber1, int chamber2, int* partition){
	int stack[(symbol->size) * ((symbol->size) - 1)][2];
	int stacksize;
	int i,j;
	int size = symbol->size;
	for(i=0; i<2; i++)
		if(symbol->m[chamber1][i]!=symbol->m[chamber2][i])
			return 0;
	if(symbol->color[chamber1]!=symbol->color[chamber2])
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
			if(symbol->color[neighbour1]!=symbol->color[neighbour2])
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
void minimal_delaney(DELANEY *symbol, DELANEY *minimal_symbol){
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
		minimal_symbol->color[i]=symbol->color[new2old[i]];
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
int compare(DELANEY *symbol1, DELANEY *symbol2){
	int i=0;
	if(symbol1->size!=symbol2->size)
		return symbol1->size - symbol2->size;
	while(i<symbol1->size && symbol1->m[i][0] == symbol2->m[i][0])
		i++;
	if(i<symbol1->size)
		return (symbol1->m[i][0] - symbol2->m[i][0]);
		
	i=0;
	while(i<symbol1->size && symbol1->m[i][1] == symbol2->m[i][1])
		i++;
	if(i<symbol1->size)
		return (symbol1->m[i][1] - symbol2->m[i][1]);
		
	int j;
	for(j=0;j<3;j++){
		i=0;
		while(i<symbol1->size && symbol1->chambers[i][j] == symbol2->chambers[i][j])
			i++;
		if(i<symbol1->size)
			return (symbol1->chambers[i][j] - symbol2->chambers[i][j]);
	}

	i=0;
	while(i<symbol1->size && symbol1->color[i] == symbol2->color[i])
		i++;
	if(i<symbol1->size)
		return (symbol1->color[i] - symbol2->color[i]);
	
	return 0;
}

int debug = 0;

void doDebug(int i){
	debug=i;
}

/*
 * When this method returns relabelling will contain a canonical relabelling of the chambers 
 * that gives the chamber 'start' the label '0'.
 * This relabelling is based on a DFS that choses the children to visit in the order sigma_0 .. sigma_2
 */
void canonical_chamber_relabelling(DELANEY *symbol, int *relabelling, int start){
	int stack[symbol->size];
	int stacksize;
	int i, j;
	int visited[symbol->size];
	int index = 0;
	for(i=0; i<symbol->size; i++)
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
void apply_relabelling(DELANEY *origin, int *relabelling, DELANEY *image){
	int reverse_labelling[origin->size];
	int i;
	image->size = origin->size;
	for(i=0; i<origin->size; i++){
		reverse_labelling[relabelling[i]] = i;
	}
	
	for(i=0; i<origin->size; i++){
		image->m[i][0] = origin->m[relabelling[i]][0];
		image->m[i][1] = origin->m[relabelling[i]][1];
		image->color[i] = origin->color[relabelling[i]];
		image->chambers[i][0] = reverse_labelling[origin->chambers[relabelling[i]][0]];
		image->chambers[i][1] = reverse_labelling[origin->chambers[relabelling[i]][1]];
		image->chambers[i][2] = reverse_labelling[origin->chambers[relabelling[i]][2]];
	}
}

/*
 * canon_symbol contains the canonical form of symbol when this method returns
 */
void canonical_form(DELANEY *symbol, DELANEY *canon_symbol){
	int i;
	int  relabelling[symbol->size];
	int found=0; //true when we already found a possible candidate
	DELANEY temp_delaney;
	for(i=0; i<symbol->size; i++){
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

void copyDelaney(DELANEY *original, DELANEY *copy){
	copy->size = original->size;
	int i, j;
	for(j=0;j<original->size;j++){
		for(i=0;i<3;i++)
			copy->chambers[j][i]=original->chambers[j][i];
		copy->m[j][0]=original->m[j][0];
		copy->m[j][1]=original->m[j][1];
		copy->color[j]=original->color[j];
	}
}


int addSymbol2Library(DELANEY *symbol, DELANEY_COLLECTION *library){
	canonical_form(symbol, library->collection + library->size);
	int i = 0;
	while(i<library->size && compare(library->collection + library->size, library->collection + i)!=0)
		i++;
	if(i==library->size){
		library->size++;
		return 1;
	} else
		return 0;
}

//adds symbol to library and returns the position where it can be found
int addSymbol2LibraryPosition(DELANEY *symbol, DELANEY_COLLECTION *library){
	canonical_form(symbol, library->collection + library->size);
	int i = 0;
	while(i<library->size && compare(library->collection + library->size, library->collection + i)!=0)
		i++;
	if(i==library->size){
		library->size++;
	}
	return i;
}


int getChambersInOrbit(DELANEY *symbol, int start, int i, int j){
	int marker[symbol->size];
	int index;
	markorbit(symbol, marker, start, i, j, 1);
	int size = 0;
	for(index = 0; index<symbol->size; index++)
		if(marker[index])
			size++;
	return size;
}

int getOrbitSize(DELANEY *symbol, int start, int i, int j){
	int next = symbol->chambers[symbol->chambers[start][i]][j];
	if(next == start)
		return 1;
	int size = 2;
	while((next = symbol->chambers[symbol->chambers[next][i]][j])!=start) size++;
	return size;
}

int getV(DELANEY *symbol, int chamber, int i, int j){
	if(i==j)
		return 0;
	if(i+j==2)
		return 2/getOrbitSize(symbol, chamber, i, j);
	if(i+j==1)
		return (symbol->m[chamber][0])/getOrbitSize(symbol, chamber, i, j);
	if(i+j==3)
		return (symbol->m[chamber][1])/getOrbitSize(symbol, chamber, i, j);
	return 0;
}

void markorbit(DELANEY *symbol, int *marker, int chamber, int i, int j, int clean){
	//clean marker if needed
	int index;
	if(clean)
		for(index = 0; index<symbol->size; index++)
			marker[index]=0;
			
	//mark orbit
	marker[chamber]=1;
	marker[symbol->chambers[chamber][i]]=1;
	int next = symbol->chambers[symbol->chambers[chamber][i]][j];
	while(next!=chamber){
		marker[next]=1;
		marker[symbol->chambers[next][i]]=1;
		next = symbol->chambers[symbol->chambers[next][i]][j];	
	}
}


int isOrientable(DELANEY *symbol){
	int orientation[symbol->size];
	int stack[symbol->size];
	int stacksize;
	int i, j;
	
	for(i=0;i<symbol->size;i++)
		orientation[i]=0;
		
	orientation[0]=1;
	stack[0] = 0;
	stacksize = 1;
	
	while(stacksize>0){
		int chamber = stack[--stacksize];
		for(j=0; j<3; j++){
			if(orientation[symbol->chambers[chamber][j]]==0){
				orientation[symbol->chambers[chamber][j]]=-orientation[chamber];
				stack[stacksize++] = symbol->chambers[chamber][j];
			} else if(orientation[symbol->chambers[chamber][j]]==orientation[chamber]){
				return 0;
			}
		}
	}
	
	return 1;
}

void makeOrientable(DELANEY *symbol, DELANEY *copy){
	if(isOrientable(symbol)){
		copyDelaney(symbol, copy);
		return;
	}

	int orientation[symbol->size];
	int stack[symbol->size];
	int stacksize;
	int i, j;
	
	for(i=0;i<symbol->size;i++)
		orientation[i]=0;
		
	orientation[0]=1;
	stack[0] = 0;
	stacksize = 1;
	
	while(stacksize>0){
		int chamber = stack[--stacksize];
		for(j=0; j<3; j++){
			if(orientation[symbol->chambers[chamber][j]]==0){
				orientation[symbol->chambers[chamber][j]]=-orientation[chamber];
				stack[stacksize++] = symbol->chambers[chamber][j];
			}
		}
	}

	emptyDelaney(copy, (symbol->size)*2);

	for (i = 0; i < symbol->size; i++) {
		for (j = 0; j < 2; j++) {
			copy->m[i][j]=symbol->m[i][j];
			copy->m[i+symbol->size][j]=symbol->m[i][j];
			copy->color[i]=symbol->color[i];
			copy->color[i+symbol->size]=symbol->color[i];
		}
	}
	
	for (i = 0; i < symbol->size; i++) {
		for (j = 0; j < 3; j++) {
			int target = symbol->chambers[i][j];
			if(orientation[i]==orientation[target]){
				copy->chambers[i][j] = target + symbol->size;
				copy->chambers[i + symbol->size][j] = target;
			} else {
				copy->chambers[i][j] = target;
				copy->chambers[i + symbol->size][j] = target + symbol->size;
			}
		}
	}
}

//checks to see if the symbol is orientable and there occur no branchings
int hasOnlyTranslation(DELANEY *symbol){
	int i;
	if(!isOrientable(symbol))
		return 0;
	for(i=0;i<symbol->size;i++)
		if(getChambersInOrbit(symbol, i, 0, 1)!=2*(symbol->m[i][0]) || getChambersInOrbit(symbol, i, 1, 2)!=2*(symbol->m[i][1]) || getChambersInOrbit(symbol, i, 0, 2)!=4)
			return 0;
	return 1;
}

void copyInto(DELANEY *original, DELANEY *copy, int n){
	copy->size = n*original->size;
	int i, j, k;
	for(k=0;k<n;k++){
		for(j=0;j<original->size;j++){
			for(i=0;i<3;i++)
				if(original->chambers[j][i]!=-1)
					copy->chambers[j + k*original->size][i]=original->chambers[j][i] + k*original->size;
				else
					copy->chambers[j + k*original->size][i]=-1;
			copy->m[j + k*original->size][0]=original->m[j][0];
			copy->m[j + k*original->size][1]=original->m[j][1];
			copy->color[j + k*original->size]=original->color[j];
		}
	}
}

void tryEdge(DELANEY *symbol, int chamber, int sigma){
	int i;
	int m;
	for(i=0; i<3; i++){
		if(i!=sigma){
			if(i+sigma==1)
				m = symbol->m[chamber][0];
			else if(i+sigma==3)
				m = symbol->m[chamber][1];
			else
				m = 2;

			int sigmas[2];
			sigmas[0]=i;
			sigmas[1]=sigma;
			int j=0;
			int count=0;
			int current=chamber;
			while(symbol->chambers[current][sigmas[j]]!=-1){
				current = symbol->chambers[current][sigmas[j]];
				j=(j+1)%2;
				count++;
			}
			if(count==2*m-1){
				symbol->chambers[chamber][sigma]=current;
				symbol->chambers[current][sigma]=chamber;
				if(i*sigma!=0)
					tryOrbit(symbol, chamber, sigma, 0);
				else if(i+sigma==1)
					tryOrbit(symbol, chamber, sigma, 2);
				else if(i+sigma==2)
					tryOrbit(symbol, chamber, sigma, 1);
				return;
			}
		}
	}
}

void tryOrbit(DELANEY *symbol, int chamber, int sigma1, int sigma2){
	int sigmas[2];
	sigmas[0] = sigma1;
	sigmas[1] = sigma2;
	int i = 0;
	int current = chamber;
	while(symbol->chambers[current][sigmas[i]]!=-1 && symbol->chambers[current][sigmas[i]]!=chamber){
		current = symbol->chambers[current][sigmas[i]];
		i = (i+1)%2;
	}
	if(symbol->chambers[current][sigmas[i]]!=chamber)
		tryEdge(symbol, current, sigmas[i]);
}

void closeOrbits(DELANEY *symbol){
	int i, j;
	for(i=0; i<symbol->size; i++)
		for(j=0;j<3;j++)
			if(symbol->chambers[i][j]==-1)
				tryEdge(symbol, i, j);
}

int checkOrbit(DELANEY *symbol, int chamber, int sigma1, int sigma2){
	int j;
	int sigmas[2];
	sigmas[0]=sigma1;
	sigmas[1]=sigma2;
	int m;
	if(sigma1+sigma2==1)
		m = symbol->m[chamber][0];
	else if(sigma1+sigma2==3)
		m = symbol->m[chamber][1];
	else
		m = 2;
	
	int marker[symbol->size];
	for(j=0;j<symbol->size;j++)
		marker[j]=0;
	marker[chamber]=1;
		
	int incompleteOrbit=0;
	for(j = 0; j<2; j++){
		int i = j;
		int next = symbol->chambers[chamber][sigmas[i]];
		while(next != -1 && !marker[next]){
			i = (i+1)%2;
			marker[next]=1;
			next = symbol->chambers[next][sigmas[i]];
		}
		incompleteOrbit = incompleteOrbit || next == -1;
	}

	int size=0;
	for(j = 0; j<symbol->size;j++)
		if(marker[j])
			size++;
	if(incompleteOrbit)
		return size <= 2*m;
	else
		return size == 2*m;
}

int checkOrbits(DELANEY *symbol, int chamber, int sigma){
	int j;
	for(j=0;j<3;j++)
		if(j!=sigma && !checkOrbit(symbol, chamber, sigma, j))
			return 0;
	return 1;
}

int isCompleteSymbol(DELANEY *symbol){
	int i, j;
	for(j=0;j<symbol->size;j++)
		for(i=0;i<3;i++)
			if(symbol->chambers[j][i]==-1)
				return 0;
	return 1;
}

int completeSymbol(DELANEY *symbol, DELANEY *original, int start){
	int n = symbol->size / original->size;
	int i, j;
	for(j=start;j<symbol->size;j++)
		for(i=0;i<3;i++)
			if(symbol->chambers[j][i]==-1){
				int k;
				int jIndex = j % original->size;
				int jSymbol = (j - jIndex)/original->size;
				for(k=jSymbol;k<n;k++){ //k must start from jSymbol and not from jSymbol + 1 !!!
					if(original->chambers[jIndex][i] + k*original->size > j && symbol->chambers[original->chambers[jIndex][i] + k*original->size][i]==-1){
						DELANEY copy;
						copyDelaney(symbol, &copy);
						copy.chambers[j][i]=original->chambers[jIndex][i] + k*original->size;
						copy.chambers[original->chambers[jIndex][i] + k*original->size][i]=j;
						if(checkOrbits(&copy, j, i)){
							closeOrbits(&copy);
							if(completeSymbol(&copy, original, j)){
								copyDelaney(&copy, symbol);
								return 1;
							}
						}
					}
				}
			}
	return (isCompleteSymbol(symbol) && hasOnlyTranslation(symbol));
}

int makeOnlyTranslation(DELANEY *symbol, DELANEY *cover){
	DELANEY workcopy;
	int i;
	
	//first make sure the symbol is orientable, then check to see if it contains only translations
	makeOrientable(symbol, &workcopy);
	if(hasOnlyTranslation(&workcopy)){
		copyDelaney(&workcopy, cover);
		return 1;
	}
	
	//calculate maximum cover size
	int max = 0;
	int max_chamber = -1;
	int max_sigmas[3];
	for(i=0; i<workcopy.size;i++){
		int temp = getV(&workcopy, i, 0, 1);
		if(temp>max){
			max=temp;
			max_chamber = i;
			max_sigmas[0] = 0;
			max_sigmas[1] = 1;
			max_sigmas[2] = 2;
		}
	}
	for(i=0; i<workcopy.size;i++){
		int temp = getV(&workcopy, i, 0, 2);
		if(temp>max){
			max=temp;
			max_chamber = i;
			max_sigmas[0] = 0;
			max_sigmas[1] = 2;
			max_sigmas[2] = 1;
		}
	}
	for(i=0; i<workcopy.size;i++){
		int temp = getV(&workcopy, i, 1, 2);
		if(temp>max){
			max=temp;
			max_chamber = i;
			max_sigmas[0] = 1;
			max_sigmas[1] = 2;
			max_sigmas[2] = 0;
		}
	}

	//create fundamental patch
	DELANEY patch;
	emptyDelaney(&patch, workcopy.size);
	patch.m[max_chamber][0]=workcopy.m[max_chamber][0];
	patch.m[max_chamber][1]=workcopy.m[max_chamber][1];
	symbolDFS(&workcopy, &patch, max_sigmas, max_chamber);
	closeOrbits(&patch);
	
	//create cover
	copyInto(&patch, cover, max);
	
	//close max branching orbit
	int j=0;
	int count=0;
	int current=max_chamber;
	while(workcopy.chambers[current][max_sigmas[j]]!=max_chamber){
		for(i = 0; i<max; i++){
			cover->chambers[current + i*workcopy.size][max_sigmas[j]] = workcopy.chambers[current][max_sigmas[j]] + i*workcopy.size;
			cover->chambers[workcopy.chambers[current][max_sigmas[j]] + i*workcopy.size][max_sigmas[j]] = current + i*workcopy.size;
		}
		current = workcopy.chambers[current][max_sigmas[j]];
		j=(j+1)%2;
		count++;
	}	
	for(i = 0; i<max; i++){
		cover->chambers[current + i*workcopy.size][max_sigmas[1]] = workcopy.chambers[current][max_sigmas[1]] + ((i + 1)%max)*workcopy.size;
		cover->chambers[workcopy.chambers[current][max_sigmas[1]] + ((i + 1)%max)*workcopy.size][max_sigmas[1]] = current + i*workcopy.size;
	}
	closeOrbits(cover);

	//try closing remaining orbit
	return completeSymbol(cover, &workcopy, 0);
}

void symbolBFS(DELANEY *symbol, DELANEY *tree){
	int i;
	
	emptyDelaney(tree, symbol->size);

	int queue[symbol->size];
	int queueHead = 0;
	int queueTail = 0;
	
	queue[queueTail++]=0;
	tree->m[0][0]=symbol->m[0][0];
	tree->m[0][1]=symbol->m[0][1];
	
	while(queueTail>queueHead){
		int current = queue[queueHead++];
		for(i=0; i<3; i++){
			if(tree->m[symbol->chambers[current][i]][0]==-1){
				queue[queueTail++] = symbol->chambers[current][i];
				tree->m[symbol->chambers[current][i]][0]=symbol->m[symbol->chambers[current][i]][0];
				tree->m[symbol->chambers[current][i]][1]=symbol->m[symbol->chambers[current][i]][1];
				tree->color[symbol->chambers[current][i]]=symbol->color[symbol->chambers[current][i]];
				tree->chambers[current][i]=symbol->chambers[current][i];
				tree->chambers[symbol->chambers[current][i]][i]=current;
			}
		}
	}
}

void symbolDFS(DELANEY *symbol, DELANEY *tree, int *sigmas, int parent){
	int i;
	for(i=0; i<3; i++){
		if(tree->m[symbol->chambers[parent][sigmas[i]]][0]==-1){
			tree->m[symbol->chambers[parent][sigmas[i]]][0]=symbol->m[symbol->chambers[parent][sigmas[i]]][0];
			tree->m[symbol->chambers[parent][sigmas[i]]][1]=symbol->m[symbol->chambers[parent][sigmas[i]]][1];
			tree->color[symbol->chambers[parent][sigmas[i]]]=symbol->color[symbol->chambers[parent][sigmas[i]]];
			tree->chambers[parent][sigmas[i]]=symbol->chambers[parent][sigmas[i]];
			tree->chambers[symbol->chambers[parent][sigmas[i]]][sigmas[i]]=parent;
			symbolDFS(symbol, tree, sigmas, symbol->chambers[parent][sigmas[i]]);
		}
	}	
}
