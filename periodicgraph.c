/*
 *  periodicgraph.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/03/08.
 *  Copyright 2008.
 *
 *  Offers some basic operations for two dimensional Delaney symbols.
 *
 *  compile with: 
 *  gcc periodicgraph.c -c
 */
 

#include "periodicgraph.h"

struct EdgeElement {
	int fromChamber;
	int toChamber;
	int sigma;
	int toNext;
	int toPrev;
	struct EdgeElement *prev;
	struct EdgeElement *next;
};

int transformHexagonPatchToParallelogram(DELANEY *symbol, DELANEY *patch){
	printDelaney(symbol, stdout);
	exportDelaney(symbol, stdout);
	//find edge of patch
	int startChamber = 0, startSigma = 0;
	int i, j, k;
	while(startChamber<symbol->size && patch->chambers[startChamber][startSigma]!=-1)
		if(startSigma==2){
			startSigma=0;
			startChamber++;
		} else {
			startSigma++;
		}
	if(startChamber==symbol->size)
		return 0;

	int edgeChambers[symbol->size][symbol->size][3];
	
	for(i=0; i<symbol->size; i++)
		for(j=0; j<symbol->size; j++)
			for(k=0; k<3; k++)
				edgeChambers[i][j][k]=-1;
	int edgeSize = 0;
	
	struct EdgeElement edgeStart;
	edgeStart.fromChamber = startChamber;
	edgeStart.toChamber = symbol->chambers[startChamber][startSigma];
	edgeStart.sigma = startSigma;
	edgeStart.toPrev = (startSigma + 1)%3;
	edgeStart.toNext = (startSigma + 2)%3;
	edgeStart.prev = NULL;
	edgeStart.next = NULL;
	edgeChambers[startChamber][symbol->chambers[startChamber][startSigma]][startSigma]=edgeSize++;

	struct EdgeElement *last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toNext;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = sigmas[(i+1)%2];
		next->toNext = last->toPrev;
		next->prev = last;
		next->next = NULL;
		last->next = next;
		last = next;
	}
	
	edgeChambers[last->fromChamber][last->toChamber][last->sigma]=-1;
	last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toPrev;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = last->toNext;
		next->toNext = sigmas[(i+1)%2];
		next->prev = NULL;
		next->next = last;
		last->prev = next;
		last = next;
	}

	struct EdgeElement *endOfEdge = last;
	while(endOfEdge->next!=NULL)
		endOfEdge = endOfEdge->next;
	
	int pathFrom = last->fromChamber;
	int pathTo = last->toChamber;

	//create path along which to cut the patch

	//start with marking the faces
	int faceMarker[patch->size];
	int faceCount=0;
	
	for(i=0; i<patch->size;i++)
		faceMarker[i]=-1;
	
	for(i=0; i<patch->size;i++){
		if(faceMarker[i]==-1){
			faceMarker[i]=faceCount;
			int s = 0;
			int chamber = i;
			while(patch->chambers[chamber][s]!=-1 && patch->chambers[chamber][s]!=i){
				chamber = patch->chambers[chamber][s];
				s = (s+1)%2;
				faceMarker[chamber]=faceCount;
			}
			s = 1;
			chamber = i;
			while(patch->chambers[chamber][s]!=-1 && patch->chambers[chamber][s]!=i){
				chamber = patch->chambers[chamber][s];
				s = (s+1)%2;
				faceMarker[chamber]=faceCount;
			}
		
			faceCount++;
		}
	}

	for(i=0; i<patch->size;i++)
		if(faceMarker[i]==-1)
			return 0;

	//break s2 along the edge to prevent finding the same edge
	DELANEY tempPatch;
	copyDelaney(patch, &tempPatch);
	struct EdgeElement *currentEdgeEl = last;
	while(currentEdgeEl!=NULL){
		tempPatch.chambers[currentEdgeEl->fromChamber][2]=-1;
		tempPatch.chambers[currentEdgeEl->toChamber][2]=-1;
		currentEdgeEl = currentEdgeEl->next;
	}

	//create the graph
	int faceGraph[faceCount][faceCount];
	for(i=0; i<faceCount; i++)
		for(j=0; j<faceCount; j++)
			faceGraph[i][j]=0;
			
	for(i=0; i<patch->size;i++)
		if(tempPatch.chambers[i][2]!=-1)
			faceGraph[faceMarker[i]][faceMarker[tempPatch.chambers[i][2]]]=1;
	
	//search path between two orbits by looking for a shortest path in the created graph
	int startFace = faceMarker[pathFrom];
	int targetFace = faceMarker[pathTo];
	int precessors[faceCount];
	int distances[faceCount];
	for(i=0; i<faceCount; i++){
		precessors[i]=-1;
		distances[i]=faceCount+1;
	}
	
	int queue[faceCount];
	int queueHead = 0;
	int queueTail = 0;
	
	queue[queueTail++]=startFace;
	precessors[startFace]=-2;
	distances[startFace]=0;
	
	while(queueTail>queueHead && precessors[targetFace]==-1){
		int current = queue[queueHead++];
		for(i=0; i<faceCount; i++){
			if(faceGraph[current][i] && distances[i]==faceCount+1){
				queue[queueTail++]=i;
				distances[i]=distances[current]+1;
				precessors[i]=current;
			}
		}
	}

	if(precessors[targetFace]==-1){
		perror("target not reached.");
		return 0;
	}

	//create the shortest path
	int path[distances[targetFace]];
	
	path[distances[targetFace]-1]=targetFace;
	for(i=distances[targetFace]-2; i>=0; i--)
		path[i] = precessors[path[i+1]];
		
	//create edge from path
	int newEdge[distances[targetFace]*2];

	int tempChamber = pathFrom;
	
	while(faceMarker[tempChamber]!=targetFace){
		i=1;
		j=0;
		while(faceMarker[symbol->chambers[tempChamber][2]]!=path[j/2]){
			//walk through face until we find a connection to the next face
			tempChamber = symbol->chambers[tempChamber][i];
			i = (i+1)%2;
		}
		newEdge[j] = tempChamber;
		j++;
		tempChamber = symbol->chambers[tempChamber][2];
		newEdge[j] = tempChamber;
		j++;
	}
	
	//break patch along new edge
	for(i=0; i<distances[targetFace]*2; i++){
		patch->chambers[patch->chambers[newEdge[i]][0]][0]=-1;
		patch->chambers[newEdge[i]][0]=-1;
	}
	
	//glue patch along part of old edge
	//search backward from end of path to find cornerpoint
	currentEdgeEl = endOfEdge;
	int nrOfTiles = 0;
	
	while(nrOfTiles!=3){
		currentEdgeEl = currentEdgeEl->prev;
		nrOfTiles = 0;
		int chamber, m, sigma;
		chamber = currentEdgeEl->fromChamber;
		m = symbol->m[chamber][0];
		for(k=0; k<m; k++){
			if(patch->chambers[chamber][j]==-1)
				nrOfTiles++;
			if(patch->chambers[symbol->chambers[chamber][j]][sigma]==-1)
				nrOfTiles++;
			chamber = symbol->chambers[symbol->chambers[chamber][j]][sigma];
		}
	}
	
	struct EdgeElement *stopEdgeEl = currentEdgeEl;
	currentEdgeEl = endOfEdge;
	while(currentEdgeEl != stopEdgeEl){
		patch->chambers[currentEdgeEl->fromChamber][0] = currentEdgeEl->toChamber;
		patch->chambers[currentEdgeEl->toChamber][0] = currentEdgeEl->fromChamber;
		currentEdgeEl = currentEdgeEl -> prev;
	}

	//TODO: free memory from edge elements
	
	return 1;
}

int createPeriodicGraphFromQuadrangularPatch(DELANEY *symbol, PeriodicGraph *graph, DELANEY *patch){
	int i, j, k;
	//find edge
	int startChamber = 0, startSigma = 0;
	while(startChamber<symbol->size && patch->chambers[startChamber][startSigma]!=-1)
		if(startSigma==2){
			startSigma=0;
			startChamber++;
		} else {
			startSigma++;
		}
	if(startChamber==symbol->size)
		return 0;

	int edgeChambers[symbol->size][symbol->size][3];
	
	for(i=0; i<symbol->size; i++)
		for(j=0; j<symbol->size; j++)
			for(k=0; k<3; k++)
				edgeChambers[i][j][k]=-1;
	int edgeSize = 0;
	
	struct EdgeElement edgeStart;
	edgeStart.fromChamber = startChamber;
	edgeStart.toChamber = symbol->chambers[startChamber][startSigma];
	edgeStart.sigma = startSigma;
	edgeStart.toPrev = (startSigma + 1)%3;
	edgeStart.toNext = (startSigma + 2)%3;
	edgeStart.prev = NULL;
	edgeStart.next = NULL;
	edgeChambers[startChamber][symbol->chambers[startChamber][startSigma]][startSigma]=edgeSize++;

	struct EdgeElement *last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toNext;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = sigmas[(i+1)%2];
		next->toNext = last->toPrev;
		next->prev = last;
		next->next = NULL;
		last->next = next;
		last = next;
	}
	
	struct EdgeElement *edgeNorthEnd = last->prev;
	struct EdgeElement *edgeWestEnd = &edgeStart;
	for(i=0; i<edgeChambers[last->toChamber][last->fromChamber][last->sigma]; i++)
		edgeWestEnd = edgeWestEnd->next;

	struct EdgeElement *edgeNorthStart = edgeWestEnd->next;
	
	edgeChambers[last->fromChamber][last->toChamber][last->sigma]=-1;
	last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toPrev;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = last->toNext;
		next->toNext = sigmas[(i+1)%2];
		next->prev = NULL;
		next->next = last;
		last->prev = next;
		last = next;
	}
	
	struct EdgeElement *edgeWestStart = last->next;

	//mark vertices
	int vertexMarker[symbol->size];
	int vertexCount = 0;
	for(i=0; i<symbol->size; i++)
		vertexMarker[i]=-1;
	
	for(i=0; i<symbol->size; i++)
		if(vertexMarker[i]==-1){
			vertexMarker[i]=vertexCount;
			vertexMarker[symbol->chambers[i][1]]=vertexCount;
			int next = symbol->chambers[symbol->chambers[i][1]][2];
			while(next!=i){
				vertexMarker[next]=vertexCount;
				vertexMarker[symbol->chambers[next][1]]=vertexCount;
				next = symbol->chambers[symbol->chambers[next][1]][2];	
			}
			graph->x[vertexCount]=0.0;
			graph->y[vertexCount]=0.0;
			vertexCount++;
		}

	
	int edgeMarker[symbol->size];
	int edgeCount = 0;
		
	for(i=0; i<symbol->size; i++)
		edgeMarker[i]=-1;
	
	for(i=0; i<symbol->size; i++)
		if(edgeMarker[i]==-1){
				edgeMarker[i]=edgeCount;
				edgeMarker[symbol->chambers[i][0]]=edgeCount;
				int next = symbol->chambers[symbol->chambers[i][0]][2];
				while(next!=i){
					edgeMarker[next]=edgeCount;
					edgeMarker[symbol->chambers[next][0]]=edgeCount;
					next = symbol->chambers[symbol->chambers[next][0]][2];	
				}
				edgeCount++;
		}
			
	int edgeCounter = 0;
	for(i=0; i<symbol->size; i++)
		edgeMarker[i]=0;
	
	//PGEdge *edges = (PGEdge *)malloc(edgeCount*sizeof(PGEdge));
	PGEdge edges[edgeCount];

	struct EdgeElement *current = edgeWestStart;
	
	int westLength = 0;
	while(current!=edgeWestEnd){
		westLength++;
		current = current->next;
	}
	
	current = edgeNorthStart;
	
	int northLength = 0;
	while(current!=edgeNorthEnd){
		northLength++;
		current = current->next;
	}
	current = edgeWestStart;

	double yStep = 2.0/((westLength+1)/2 + 1);
	double xStep = 2.0/((northLength+1)/2 + 1);
	double yPos = 1.0 - yStep;
	double xPos = -1.0 + xStep/2;

	while(current!=edgeWestEnd){
		if(!edgeMarker[current->fromChamber] && current->sigma==0 && current->toNext==2){
			(edges + edgeCounter)->from = vertexMarker[current->fromChamber];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[current->fromChamber][0]];
			(edges + edgeCounter)->x = -1;
			(edges + edgeCounter)->y = 0;
			graph->x[vertexMarker[current->fromChamber]] = xPos;
			graph->y[vertexMarker[current->fromChamber]] = yPos;
			graph->x[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - xPos;
			graph->y[vertexMarker[symbol->chambers[current->fromChamber][0]]] = yPos;
			yPos = yPos - yStep;
			edgeCounter++;
			markorbit(symbol, edgeMarker, current->fromChamber, 0, 2, 0);
		}
		current = current->next;
	}
	
	current = edgeNorthStart;
	xPos += xStep/2;
	yPos += yStep/2;

	while(current!=edgeNorthEnd){
		if(!edgeMarker[current->fromChamber] && current->sigma==0 && current->toNext==2){
			(edges + edgeCounter)->from = vertexMarker[current->fromChamber];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[current->fromChamber][0]];
			(edges + edgeCounter)->x = 0;
			(edges + edgeCounter)->y = -1;
			graph->x[vertexMarker[current->fromChamber]] = xPos;
			graph->y[vertexMarker[current->fromChamber]] = yPos;
			graph->x[vertexMarker[symbol->chambers[current->fromChamber][0]]] = xPos;
			graph->y[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - yPos;
			xPos = xPos + xStep;
			edgeCounter++;
			markorbit(symbol, edgeMarker, current->fromChamber, 0, 2, 0);
		}
		current = current->next;
	}
	
	for(i = 0; i<symbol->size; i++)
		if(!edgeMarker[i]){
			(edges + edgeCounter)->from = vertexMarker[i];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[i][0]];
			(edges + edgeCounter)->x = 0;
			(edges + edgeCounter)->y = 0;
			edgeCounter++;
			markorbit(symbol, edgeMarker, i, 0, 2, 0);
		}

	if(edgeCounter!=edgeCount)
		return 0;
	
	graph->edges = edges;
	graph->size = edgeCount;
	graph->order = vertexCount;

	//TODO: free edge of patch

	return 1;

}

	int getNumberOfTiles(DELANEY *symbol, DELANEY *patch, int chamber, int sigma1, int sigma2){
		int m, k;
		if(sigma1+sigma2==1)
			m = symbol->m[chamber][0];
		else if(sigma1+sigma2==3)
			m = symbol->m[chamber][1];
		else
			m = 2;

		int counter = 0;
		for(k=0; k<m; k++){
			if(patch->chambers[chamber][sigma1]==-1)
				counter++;
			if(patch->chambers[symbol->chambers[chamber][sigma1]][sigma2]==-1)
				counter++;
			chamber = symbol->chambers[symbol->chambers[chamber][sigma1]][sigma2];
		}
		return counter;
	}

int createPeriodicGraphFromHexagonalPatch(DELANEY *symbol, PeriodicGraph *graph, DELANEY *patch){
	int i, j, k;
	//find edge
	int startChamber = 0, startSigma = 0;
	while(startChamber<symbol->size && patch->chambers[startChamber][startSigma]!=-1)
		if(startSigma==2){
			startSigma=0;
			startChamber++;
		} else {
			startSigma++;
		}
	if(startChamber==symbol->size)
		return 0;

	int edgeChambers[symbol->size][symbol->size][3];
	
	for(i=0; i<symbol->size; i++)
		for(j=0; j<symbol->size; j++)
			for(k=0; k<3; k++)
				edgeChambers[i][j][k]=-1;
	int edgeSize = 0;
	
	struct EdgeElement edgeStart;
	edgeStart.fromChamber = startChamber;
	edgeStart.toChamber = symbol->chambers[startChamber][startSigma];
	edgeStart.sigma = startSigma;
	edgeStart.toPrev = (startSigma + 1)%3;
	edgeStart.toNext = (startSigma + 2)%3;
	edgeStart.prev = NULL;
	edgeStart.next = NULL;
	edgeChambers[startChamber][symbol->chambers[startChamber][startSigma]][startSigma]=edgeSize++;

	struct EdgeElement *last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toNext;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = sigmas[(i+1)%2];
		next->toNext = last->toPrev;
		next->prev = last;
		next->next = NULL;
		last->next = next;
		last = next;
	}
	
	struct EdgeElement *edgeThirdEnd = last->prev;
	edgeChambers[last->fromChamber][last->toChamber][last->sigma]=-1;
	last = &edgeStart;
	
	while(edgeChambers[last->toChamber][last->fromChamber][last->sigma]==-1){
		int sigmas[2];
		sigmas[0] = last->toPrev;
		sigmas[1] = last->sigma;
		int chamber = last->fromChamber;
		i=0;
		while(patch->chambers[chamber][sigmas[i]]!=-1){
			chamber = patch->chambers[chamber][sigmas[i]];
			i = (i+1)%2;
		}
		edgeChambers[chamber][symbol->chambers[chamber][sigmas[i]]][sigmas[i]] = edgeSize++;
		struct EdgeElement *next = (struct EdgeElement *) malloc(sizeof(struct EdgeElement));
		next->fromChamber = chamber;
		next->toChamber = symbol->chambers[chamber][sigmas[i]];
		next->sigma = sigmas[i];
		next->toPrev = last->toNext;
		next->toNext = sigmas[(i+1)%2];
		next->prev = NULL;
		next->next = last;
		last->prev = next;
		last = next;
	}
	
	struct EdgeElement *edgeFirstStart = last->next;
	struct EdgeElement *edgeFirstEnd = edgeFirstStart->next;
	while(getNumberOfTiles(symbol, patch, edgeFirstEnd->fromChamber, edgeFirstEnd->sigma, edgeFirstEnd->toNext)==2)
		edgeFirstEnd = edgeFirstEnd->next;
	
	struct EdgeElement *edgeSecondStart = edgeFirstEnd->next;
	struct EdgeElement *edgeSecondEnd = edgeSecondStart->next;
	while(getNumberOfTiles(symbol, patch, edgeSecondEnd->fromChamber, edgeSecondEnd->sigma, edgeSecondEnd->toNext)==2)
		edgeSecondEnd = edgeSecondEnd->next;

	struct EdgeElement *edgeThirdStart = edgeSecondEnd->next;

	//mark vertices
	int vertexMarker[symbol->size];
	int vertexCount = 0;
	for(i=0; i<symbol->size; i++)
		vertexMarker[i]=-1;
	
	for(i=0; i<symbol->size; i++)
		if(vertexMarker[i]==-1){
			vertexMarker[i]=vertexCount;
			vertexMarker[symbol->chambers[i][1]]=vertexCount;
			int next = symbol->chambers[symbol->chambers[i][1]][2];
			while(next!=i){
				vertexMarker[next]=vertexCount;
				vertexMarker[symbol->chambers[next][1]]=vertexCount;
				next = symbol->chambers[symbol->chambers[next][1]][2];	
			}
			graph->x[vertexCount]=0.0;
			graph->y[vertexCount]=0.0;
			vertexCount++;
		}
	
	int edgeMarker[symbol->size];
	int edgeCount = 0;
		
	for(i=0; i<symbol->size; i++)
		edgeMarker[i]=-1;
	
	for(i=0; i<symbol->size; i++)
		if(edgeMarker[i]==-1){
				edgeMarker[i]=edgeCount;
				edgeMarker[symbol->chambers[i][0]]=edgeCount;
				int next = symbol->chambers[symbol->chambers[i][0]][2];
				while(next!=i){
					edgeMarker[next]=edgeCount;
					edgeMarker[symbol->chambers[next][0]]=edgeCount;
					next = symbol->chambers[symbol->chambers[next][0]][2];	
				}
				edgeCount++;
		}
			
	int edgeCounter = 0;
	for(i=0; i<symbol->size; i++)
		edgeMarker[i]=0;
	
	//PGEdge *edges = (PGEdge *)malloc(edgeCount*sizeof(PGEdge));
	PGEdge edges[edgeCount];
	
	struct EdgeElement *current = edgeFirstStart;

	int firstLength = 0;
	while(current!=edgeFirstEnd){
		firstLength++;
		current = current->next;
	}

	current = edgeSecondStart;
	
	int secondLength = 0;
	while(current!=edgeSecondEnd){
		secondLength++;
		current = current->next;
	}

	current = edgeThirdStart;
	
	int thirdLength = 0;
	while(current!=edgeThirdEnd){
		thirdLength++;
		current = current->next;
	}

	current = edgeFirstStart;
	
	double yStep = 1.0/((firstLength+1)/2 + 1);
	double stepDiag = 0.5/((secondLength+1)/2 + 1);
	double xStep = 1.0/((thirdLength+1)/2 + 1);
	double yPos = 0.5 - yStep;
	double xPos = -1.0 + stepDiag/2;
	
	while(current!=edgeFirstEnd){
		if(!edgeMarker[current->fromChamber] && current->sigma==0 && current->toNext==2){
			(edges + edgeCounter)->from = vertexMarker[current->fromChamber];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[current->fromChamber][0]];
			(edges + edgeCounter)->x = -1;
			(edges + edgeCounter)->y = 0;
			graph->x[vertexMarker[current->fromChamber]] = xPos;
			graph->y[vertexMarker[current->fromChamber]] = yPos;
			graph->x[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - xPos;
			graph->y[vertexMarker[symbol->chambers[current->fromChamber][0]]] = yPos;
			yPos -= yStep;
			edgeCounter++;
			markorbit(symbol, edgeMarker, current->fromChamber, 0, 2, 0);
		}
		current = current->next;
	}
	
	current = edgeSecondStart;
	xPos += stepDiag/2;
	yPos += yStep/2;
	
	while(current!=edgeSecondEnd){
		if(!edgeMarker[current->fromChamber] && current->sigma==0 && current->toNext==2){
			(edges + edgeCounter)->from = vertexMarker[current->fromChamber];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[current->fromChamber][0]];
			(edges + edgeCounter)->x = -1;
			(edges + edgeCounter)->y = -1;
			graph->x[vertexMarker[current->fromChamber]] = xPos;
			graph->y[vertexMarker[current->fromChamber]] = yPos;
			graph->x[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - xPos;
			graph->y[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - yPos;
			yPos -= stepDiag;
			xPos += stepDiag;
			edgeCounter++;
			markorbit(symbol, edgeMarker, current->fromChamber, 0, 2, 0);
		}
		current = current->next;
	}
	
	current = edgeThirdStart;
	xPos += xStep/2;
	yPos += stepDiag/2;
	
	while(current!=edgeThirdEnd){
		if(!edgeMarker[current->fromChamber] && current->sigma==0 && current->toNext==2){
			(edges + edgeCounter)->from = vertexMarker[current->fromChamber];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[current->fromChamber][0]];
			(edges + edgeCounter)->x = 0;
			(edges + edgeCounter)->y = -1;
			graph->x[vertexMarker[current->fromChamber]] = xPos;
			graph->y[vertexMarker[current->fromChamber]] = yPos;
			graph->x[vertexMarker[symbol->chambers[current->fromChamber][0]]] = xPos;
			graph->y[vertexMarker[symbol->chambers[current->fromChamber][0]]] = - yPos;
			xPos += xStep;
			edgeCounter++;
			markorbit(symbol, edgeMarker, current->fromChamber, 0, 2, 0);
		}
		current = current->next;
	}
	
	for(i = 0; i<symbol->size; i++)
		if(!edgeMarker[i]){
			(edges + edgeCounter)->from = vertexMarker[i];
			(edges + edgeCounter)->to = vertexMarker[symbol->chambers[i][0]];
			(edges + edgeCounter)->x = 0;
			(edges + edgeCounter)->y = 0;
			edgeCounter++;
			markorbit(symbol, edgeMarker, i, 0, 2, 0);
		}

	if(edgeCounter!=edgeCount)
		return 0;
	
	graph->edges = edges;
	graph->size = edgeCount;
	graph->order = vertexCount;

	//TODO: free edge of patch

	return 1;

}

int createPeriodicGraph(DELANEY *symbol, PeriodicGraph *graph){
	if(!hasOnlyTranslation(symbol))
		return 0;

	//create patch
	DELANEY patch;
	{
		int sigmas[3];
		sigmas[0]=1;
		sigmas[1]=2;
		sigmas[2]=0;
		emptyDelaney(&patch, symbol->size);
		patch.m[0][0] = symbol->m[0][0];
		patch.m[0][1] = symbol->m[0][1];
		symbolDFS(symbol, &patch, sigmas, 0);
		closeOrbits(&patch);
	}

	//determine type of edge
	int i, j, k;
	int frequency[5];
	for(i=0; i<5; i++)
		frequency[i]=0;
	for(i=0; i<symbol->size; i++){
		for(j=0; j<3; j++){
			if(patch.chambers[i][j]==-1){
				int chamber, counter, m, sigma;
				
				chamber = i;
				counter = 0;
				sigma = (j + 1)%3;
				if(j+sigma==1)
					m = symbol->m[i][0];
				else if(j+sigma==3)
					m = symbol->m[i][1];
				else
					m = 2;
				for(k=0; k<m; k++){
					if(patch.chambers[chamber][j]==-1)
						counter++;
					if(patch.chambers[symbol->chambers[chamber][j]][sigma]==-1)
						counter++;
					chamber = symbol->chambers[symbol->chambers[chamber][j]][sigma];
				}
				if(counter != 2 && counter != 3 && counter != 4){
					perror("Illegal number of tiles at vertex.\n");
					return 0;
				}
				frequency[counter]++;

				chamber = i;
				counter = 0;
				sigma = (j + 2)%3;
				if(j+sigma==1)
					m = symbol->m[i][0];
				else if(j+sigma==3)
					m = symbol->m[i][1];
				else
					m = 2;
				for(k=0; k<m; k++){
					if(patch.chambers[chamber][j]==-1)
						counter++;
					if(patch.chambers[symbol->chambers[chamber][j]][sigma]==-1)
						counter++;
					chamber = symbol->chambers[symbol->chambers[chamber][j]][sigma];
				}
				frequency[counter]++;
			}
		}
	}

	if(frequency[3]==12 && frequency[4]==0){
		//if(!transformHexagonPatchToParallelogram(symbol, &patch))
		//	return 0;
		return createPeriodicGraphFromHexagonalPatch(symbol, graph, &patch);
		//hexagon
	} else if (frequency[3]==0 && frequency[4]==8){
		return createPeriodicGraphFromQuadrangularPatch(symbol, graph, &patch);
		//parallelogram
	} else {
		perror("Fundamental patch is neither a hexagon nor a quadrangle.\n");
		return 0;
		//error
	}

}

void exportPeriodicGraph(PeriodicGraph *graph, FILE *f){
	int i;
	fprintf(f, "%d|2 2|", graph->order);
	for(i = 1; i<graph->order; i++)
		fprintf(f, "%f %f;", graph->x[i-1], graph->y[i-1]);
	fprintf(f, "%f %f|", graph->x[graph->order-1], graph->y[graph->order-1]);
	
	for(i=0; i<graph->size-1;i++)
		fprintf(f, "%d %d %d %d;", (graph->edges + i)->from, (graph->edges + i)->to, (graph->edges + i)->x, (graph->edges + i)->y);
	i = graph->size-1;
	fprintf(f, "%d %d %d %d\n", (graph->edges + i)->from, (graph->edges + i)->to, (graph->edges + i)->x, (graph->edges + i)->y);
}