/*
 * twoverticesontorus.c
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

# define MAX_ORDER 64
# define MAX_DEGREE 8
# define END_MARK 99

typedef int GRAPH[MAX_ORDER+1][MAX_DEGREE+2];

typedef int ADJACENCY[MAX_ORDER+1];

struct EMB_EDGE {
	int index;
	int end1;
	int end2;
	int marker;
	
	struct EMB_EDGE *next;
	struct EMB_EDGE *previous;
	struct EMB_EDGE *inverse;
};

struct EMB_MULG {
	int order;

	struct EMB_EDGE vertices[MAX_ORDER];
	struct EMB_EDGE edges[MAX_ORDER*8];
};
 

/*************************************************************************/

void print_embedded_multigraph(mulg)

struct EMB_MULG *mulg;
{
	int i;
	fprintf(stderr,"|");
	for (i=0; i< mulg->order; i++)
		fprintf(stderr,"%2d|",i);
	fprintf(stderr,"\n|");
	for (i=0; i< mulg->order; i++)
		fprintf(stderr,"==|");
	fprintf(stderr,"\n");
	
	char temp[(mulg->order)*MAX_DEGREE];

	for(i=0; i<(mulg->order)*MAX_DEGREE; i++)
		*(temp+i)=' ';

	for (i=0; i< mulg->order; i++){
		int j=0;
		struct EMB_EDGE *edge;
		edge = (mulg->edges)+i*MAX_DEGREE;
		while(edge->next != (mulg->edges)+i*MAX_DEGREE){
			*(temp + i*MAX_DEGREE + j)=edge->index;
			j++;
			edge = edge->next;
		}

	}

	int j;
	for(i=0; i < MAX_DEGREE; i++) {
		fprintf(stderr,"|");
		for(j=0; j < mulg->order; j++) {
			fprintf(stderr, "%2d|", *(temp+i*j));
		}
		fprintf(stderr,"\n");
	}
	
}

/*************************************************************************/

void visit_face(struct EMB_EDGE *start){
		start -> marker = 1;
		struct EMB_EDGE *next;
		next = (*((*start).inverse)).previous;
		while(next != start){
			(*next).marker = 1;
			next = (*(*next).inverse).previous;
		}
}

/*************************************************************************/

int count_faces(struct EMB_MULG *emb_mulg){
	int i;
	struct EMB_EDGE *edge;
	
	/* set all markers to 0 */
	for(i=0; i<emb_mulg->order; i++){
		struct EMB_EDGE *start;
		start = &(emb_mulg->edges[i*MAX_DEGREE]);
		(*start).marker = 0;
		edge = (*start).next;
		while(edge != start){
			(*edge).marker = 0;
			edge = (*edge).next;
		}
	}
	
	/* visit all the faces */
	int counter = 0;
	for(i=0; i<emb_mulg->order; i++){
		struct EMB_EDGE *start = &(emb_mulg->edges[i*MAX_DEGREE]);
		/*(*start).marker = 0;*/
		edge = start;
		while(edge != (*start).previous){
			if((*edge).marker == 0){
				visit_face(edge);
				counter++;
			}
			edge = (*edge).next;
		}
	}
	
	return counter;
}

/*************************************************************************/
void create_multigraph_2(int a[], struct EMB_MULG *mulg){
	int i;
	mulg->order = 2;
	for(i = 0; i < MAX_DEGREE; i++){
		((mulg->edges)+i)->index = i;
		((mulg->edges)+ MAX_DEGREE + i)->index = i;
		
		((mulg->edges)+i)->next = ((mulg->edges)+((i+1)%MAX_DEGREE));
		((mulg->edges)+((i+1)%MAX_DEGREE))->previous = ((mulg->edges)+i);
		((mulg->edges)+i)->inverse = ((mulg->edges) + MAX_DEGREE + i);
		
		((mulg->edges)+ MAX_DEGREE + i)->inverse = ((mulg->edges)+i);
	}
	
	for(i = 0; i < MAX_DEGREE; i++){
		((mulg->edges)+ MAX_DEGREE + *(a+i))->next = ((mulg->edges)+ MAX_DEGREE + *(a+(i+1)%MAX_DEGREE));
		((mulg->edges)+ MAX_DEGREE + *(a+i))->previous = ((mulg->edges)+ MAX_DEGREE + *(a+(i+MAX_DEGREE-1)%MAX_DEGREE));
	}
}

/*************************************************************************/
int isCanonical_2(int a[]){
	int i;
	for(i = 1; i < 8; i++){
		int offset = 8 - a[i];
		int j = 0;
		while(j < 8 && a[j] == ((a[(i + j)%8]+offset)%8)) j++;
		if(j<8 && a[j] > (a[(i + j)%8]+offset)%8)
			return 0;
	}
	return 1;
}

int count_torus_embeddings_2(){
	int i, j, k, l, m, n, o;
	int degree = 8;
	int numberOfGraphs = 0;
	int numberOfPermutations = 0;
	int onTorus = 0;
	for(i = 1; i<degree; i++)
		for(j = 1; j<degree; j++) if(j!=i)
			for(k = 1; k < degree; k++) if(k!=i && k!=j)
				for(l = 1; l < degree; l++) if(l!=i && l!=j && l!=k)
					for(m = 1; m < degree; m++) if(m!=i && m!=j && m!=k && m!=l)
						for(n = 1; n < degree; n++) if(n!=i && n!=j && n!=k && n!=l && n!=m)
							for(o = 1; o < degree; o++) if(o!=i && o!=j && o!=k && o!=l && o!=m && o!=n){
								int numbers[8];
								numbers[0] = 0;
								numbers[1] = i;
								numbers[2] = j;
								numbers[3] = k;
								numbers[4] = l;
								numbers[5] = m;
								numbers[6] = n;
								numbers[7] = o;
								numberOfPermutations++;
								
								if(isCanonical_2(numbers)==1){
									numberOfGraphs++;
									struct EMB_MULG mulg;
									create_multigraph_2(numbers, &mulg);
									/*fprintf(stderr, "%d\n", count_faces(&mulg));*/
									/*print_embedded_multigraph(&mulg);*/
									if(count_faces(&mulg)==6)
										onTorus++;
								}
							}
	fprintf(stderr,"number of permutations: %d\n",numberOfPermutations);
	fprintf(stderr,"number of graphs: %d\n",numberOfGraphs);
	fprintf(stderr,"number of graphs on the torus: %d\n",onTorus);
	return -1;
}


/*************************************************************************/

int main() {

	count_torus_embeddings_2();
	return 0;
}
