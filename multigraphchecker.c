/*
 * multigraphcheker.c
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

struct EDGE {
	int i;
	int end1;
	int end2;
};

struct MULG {
	unsigned char order;
	ADJACENCY adj;
	struct EDGE edges[MAX_ORDER][MAX_DEGREE];
};

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
 
/****************************WRITE_GRAPH**********************************************/

void write_graph(GRAPH g, ADJACENCY adj) {

	int x,y;
	fprintf(stderr,"\n\n ");

	fprintf(stderr,"|%2d",g[0][0]);

	for(x=1; (x <= g[0][0])&&(x<=24); x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

	fprintf(stderr," ");

	for(x=0; (x <= g[0][0])&&(x<=24); x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

	for(x=0; x < MAX_DEGREE; x++)
	  {
	   fprintf(stderr," |  ");
	   for(y=1; (y<=g[0][0])&&(y<=24); y++)
		   if ((g[y][x] == END_MARK) || (x>=adj[y])) fprintf(stderr,"|  "); else fprintf(stderr,"|%2d",g[y][x]);
		   fprintf(stderr,"|\n");
	  }

	if (g[0][0]>24) 
	{
	fprintf(stderr,"\n");

	fprintf(stderr,"    ");

	for(x=25; (x <= g[0][0])&&(x<=48); x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

	fprintf(stderr,"    ");

	for(x=25; (x <= g[0][0])&&(x<=48); x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

	for(x=0; x < MAX_DEGREE; x++)
	  {
	   fprintf(stderr,"    ");
	   for(y=25; (y <= g[0][0])&&(y<=48); y++)
		   if (g[y][x] == END_MARK) fprintf(stderr,"|  "); else fprintf(stderr,"|%2d",g[y][x]);
		   fprintf(stderr,"|\n");
	  }
	}

	if (g[0][0]>48) 
	{
	fprintf(stderr,"\n");

	fprintf(stderr,"    ");

	for(x=49; x<=g[0][0]; x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

	fprintf(stderr,"    ");

	for(x=49; x <= g[0][0]; x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

	for(x=0; x < MAX_DEGREE; x++)
	  {
	   fprintf(stderr,"    ");
	   for(y=49; y<=g[0][0]; y++)
		   if (g[y][x] == END_MARK) fprintf(stderr,"|  "); else fprintf(stderr,"|%2d",g[y][x]);
		   fprintf(stderr,"|\n");
	  }
	}
	fflush(0);
}


/****************************ADD_EDGE************************/

void add_edge (GRAPH graph, ADJACENCY adj, unsigned char v, unsigned char w)
/* Fuegt die Kante (v,w) in den Graphen graph ein. Dabei wird aber davon */
/* ausgegangen, dass in adj die wirklich aktuellen werte fuer die */
/* Adjazenzen stehen. Die adjazenzen werden dann aktualisiert. */

{
graph[v][adj[v]]=w;
graph[w][adj[w]]=v;
adj[v]++;
adj[w]++;
}


/****************************DECODE************************/

void decode(code,graph,adj,length)

unsigned char *code;
GRAPH graph;
ADJACENCY adj;
int length;

{
	int i,j;
	int order;

	/* adj[] wird passend belegt UND jede liste graph[i] wird mit "leer" abgeschlossen ". */


	graph[0][0]=order=code[0];


	for (i=1; i<= order; i++)
		adj[i]=0;

	j=1;

	for (i=1; i<length; i++) {
		if (code[i]==0)
			j++;
		else
			add_edge(graph,adj,j,code[i]);
	}
	
	for (i=1; i<= length; i++)
		graph[i][adj[i]]=END_MARK;
}


/****************************READ_MULTICODE************************/

int read_multicode(unsigned char**code, int *length, FILE *fil) {

/* Liest den code und gibt EOF zurueck, wenn das Ende der Datei erreicht
   ist, 1 sonst. Der Speicher fuer den code wird alloziert, falls noetig,
   was entschieden wird anhand der lokalen Variablen maxknotenzahl */

	static int maxorder= -1;
	int codel, buffered=0;
	int zerocount;
	int order,a,b;
	unsigned char buffer;
	
	if ((order=getc(fil))==EOF) return EOF;	
	if (order==0) {
		fprintf(stderr,"Graph is too big.\n");
		exit(0);
	} 
	
	zerocount=0; codel=1;

	if (order=='>') /* could be header or 62 */ {
		buffered=1;
		a=getc(fil);
		if(a==0) zerocount++; 
		b=getc(fil);
		if(b==0) zerocount++; 
		/* next we read 3 tokens */
		if ((a=='>') && (b=='m')) /*this is a header*/{
			while ((buffer=getc(fil)) != '<');
			/* two to go: */
			buffer=getc(fil); 
			if (buffer!='<') {
				fprintf(stderr,"Problems with header -- single '<'\n");
				exit(1);
			}
			if ((order=getc(fil))==EOF)
				return EOF;
			/* no graph here */
		}
		/* else no header */
	}

	if (order > maxorder)
	{ if (*code) free(*code);
		*code=(unsigned char *)malloc((order*(order-1)/2+order)*sizeof(unsigned char));
		if (code==NULL) { fprintf(stderr,"Do not get memory for code\n"); exit(0); }
		maxorder=order;
	}

	(*code)[0]=order;
	if (buffered) {
		codel=3;
		(*code)[1]=a;
		(*code)[2]=b;
	}

	while (zerocount<order-1){
		if (((*code)[codel]=getc(fil))==0)
			zerocount++;
		codel++;
	}

	*length=codel;
	return 1;
}

/*************************************************************************/

void create_multigraph(mulg, graph, adj)

struct MULG * mulg;
GRAPH graph;
ADJACENCY adj;
{
	(*mulg).order = graph[0][0];
	
	int i,j;

	for (i=0; i< (*mulg).order; i++)
		(*mulg).adj[i]=0;

	
	int edgecount = 0;
	for(i=0; i < (*mulg).order; i++) {
		for(j=0; j < adj[i+1]; j++) {
			if(i < graph[i+1][j]-1) {
				(*mulg).edges[i][(*mulg).adj[i]].end1 = i;
				(*mulg).edges[i][(*mulg).adj[i]].end2 = graph[i+1][j]-1;
				(*mulg).edges[i][(*mulg).adj[i]].i = ++edgecount;
				(*mulg).edges[graph[i+1][j]-1][(*mulg).adj[graph[i+1][j]-1]].end1 = graph[i+1][j]-1;
				(*mulg).edges[graph[i+1][j]-1][(*mulg).adj[graph[i+1][j]-1]].end2 = i;
				(*mulg).edges[graph[i+1][j]-1][(*mulg).adj[graph[i+1][j]-1]].i = edgecount;
				(*mulg).adj[i]++;
				(*mulg).adj[graph[i+1][j]-1]++;
			} 
		}
	}
}

/*************************************************************************/

void print_multigraph(mulg)

struct MULG mulg;
{
	int i;
	fprintf(stderr,"|");
	for (i=0; i< mulg.order; i++)
		fprintf(stderr,"%2d|",i);
	fprintf(stderr,"\n|");
	for (i=0; i< mulg.order; i++)
		fprintf(stderr,"==|");
	fprintf(stderr,"\n");

	int j;
	for(i=0; i < MAX_DEGREE; i++) {
		fprintf(stderr,"|");
		for(j=0; j < mulg.order; j++) {
			if(i<mulg.adj[j]) {
				fprintf(stderr, "%2d|", mulg.edges[j][i].i);
			} else {
				fprintf(stderr, "  |");
			}
		}
		fprintf(stderr,"\n");
	}
	
}

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
									/*int eulerNumber = (2 - 2 + 8 - countFaces())/2;
									numberOfEmbeddedGraphs[eulerNumber]++;*/
								}
							}
	fprintf(stderr,"%d\n",numberOfGraphs);
	fprintf(stderr,"%d\n",numberOfPermutations);
	fprintf(stderr,"%d\n",onTorus);
	return -1;
}


/*************************************************************************/

int main() {

/*
	int count = 0;
	int length;
	unsigned char *code=NULL;
	GRAPH graph;
	ADJACENCY adj;
	struct MULG mulg;

	while(read_multicode(&code, &length, stdin) != EOF) {
		
		count++; 
		decode(code,graph,adj,length); 
		fprintf(stderr,"\nGraph Nr. %d",count);
		write_graph(graph, adj);
		create_multigraph(&mulg, graph, adj);
		print_multigraph(mulg);
	}
*/
	count_torus_embeddings_2();
	return 0;
}
