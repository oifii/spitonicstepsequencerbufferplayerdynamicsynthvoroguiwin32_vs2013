/*
* Copyright (c) 1994-2016 Stephane Poirier
*
* stephane.poirier@oifii.org
*
* Stephane Poirier
* 3532 rue Ste-Famille, #3
* Montreal, QC, H2X 2L1
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
//////////////////////////////////////////////////////////////////
// POINTSET.MC	 application mdl "c" (pour MicroStation)
//
// Developpe par Stephane Poirier pour travaux de recherche avec le
// Centre de Recherche en Geomatique, Univ. Laval, Quebec, G1K 7P4.
//
//	   OFFWORLD Research, Stephane Poirier
//	   675 Des Rocailles,
//	   Quebec, Que., G2J 1A9
//	   Canada
//
//
// Applications:
//
// -Visionnement des regions voronoi (donnees images vectorielles)
// -Visionnement de la triangulation (surface, position-elevation)
// -Visionnement de l'arbre de segmentation (de donnees matricielles
//  ou vectorielles).
//
//
// Description du code:
//
// -Construit une triangulation pour a partir d'un fichier de donnees
//  .xyi initiale.
// -Calcul et sauvegarde les centres des triangles (ces centres seront
//  les "vertices" des polygones voronoi).
// -Pour chaque points de donnees, trouver ses plus proche voisins et
//  generer les polygones voronoi.
//
//
//
// Algorithme de construction de la triangulation
// (obtenu de Redouane Mahmoudi, Arzew, Algerie)
//
// Pour un ensemble de points donne, cet algorithme:
//      -Determine le polygone convex entourant l'ensemble des points
//      -Genere une triangulation arbitraire.
//      -Optimise la triangulation (triangle switching) pour qu'elle
//	 corresponde au critere Delaunay.
//
//
//
// Fonctions de requete sur l'adjacence spatiales
// (par Stephane Poirier)
//
// -Fonction retournant le triangle dans lequel se trouve un point
//  donne (ordre 0).
// -Fonction retournant les triangles adjacents et entourant un
//  triangle donne (ordre 1).
// -Fonction retournant les triangles adjacents (par l'exterieur)
//  a tous les triangles trouves a l'ordre 1 (fonction ci-dessus).
//
// -Fonction retournant les points (ou objets) voisins connexes a
//  un point (ou objet) donne.
//////////////////////////////////////////////////////////////////



#include "stdafx.h"


#include <stdlib.h>
#include <malloc.h>

#include <stdio.h>
#include <math.h>
#include <string.h>
 
#include <assert.h>
#include <minmax.h>

#define COMPILING_POINTSET_CPP		1
#include "c_pointset.h"
//#include "oifiilib.h" //spi 2014


/* //spi 2014
#include "oifii_app.h"
#include "logdoc.h"
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char* pszStatChannelNames[POINTSET_DEFAULT_NUMBEROFSTAT+POINTSET_EXTRA_NUMBEROFSTAT] = 
{
	"VoroArea",
	"VoroDens",
	"AvgVoroArea",	//1NeiAvgVoroArea
	"AvgTreeInt",	//1NeiAvgIntensity
	"AvgTreeArea",	//1NeiAvgTreeArea
	"VarVoroArea",	//1NeiVarVoroArea
	"VarIntensity",	//1NeiVarIntensity
	"VarTreeArea",	//1NeiVarTreeArea
	"ClassTreeInt",
	"TreeInt",
	"TreeDimX",
	"TreeDimY",
	"TreeArea"
};


int jnd[3] = {1,2,0};
int jrd[3] = {2,0,1};
int ijk[6] = {0,1,2,0,1,2};




/////////////////////
// Pointset Constructor
/////////////////////

POINTSET* NewPointset(long maxnumberofelements)
{
    POINTSET* pPointset;

    pPointset = (POINTSET*) malloc(sizeof(POINTSET));
    if(pPointset==NULL)
    {
		printf("not able to allocate POINTSET structure\n");
		return NULL;
    }

    //
    // alloc array vt
    //
    pPointset->vt[0] = (int*) malloc(3*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)*sizeof(int));
    if(pPointset->vt[0]==NULL)
    {
		printf("not able to allocate POINTSET structure (vt)\n");
		free(pPointset);
		return NULL;
    }
    pPointset->vt[1] = (int*) &(pPointset->vt[0][1*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    pPointset->vt[2] = (int*) &(pPointset->vt[0][2*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    //
    // alloc array nt
    //
    pPointset->nt[0] = (int*) malloc(3*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)*sizeof(int));
    if(pPointset->nt[0]==NULL)
    {
		printf("not able to allocate POINTSET structure (nt)\n");
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
    pPointset->nt[1] = (int*) &(pPointset->nt[0][1*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    pPointset->nt[2] = (int*) &(pPointset->nt[0][2*(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    //
    // alloc ctx, cty
    //
    pPointset->ctx = (double*) malloc(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS*sizeof(double));
    if(pPointset->ctx==NULL)
    {
		printf("not able to allocate POINTSET structure (ctx)\n");
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
    pPointset->cty = (double*) malloc(maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS*sizeof(double));
    if(pPointset->cty==NULL)
    {
		printf("not able to allocate POINTSET structure (cty)\n");
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
    pPointset->px = (double*) malloc(maxnumberofelements*sizeof(double));
    if(pPointset->px==NULL)
    {
		printf("not able to allocate POINTSET structure (px)\n");
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
    pPointset->py = (double*) malloc(maxnumberofelements*sizeof(double));
    if(pPointset->py==NULL)
    {
		printf("not able to allocate POINTSET structure (py)\n");
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
	//spi, begin
    pPointset->controlratio = (float*) malloc(maxnumberofelements*sizeof(float));
    if(pPointset->controlratio==NULL)
    {
		printf("not able to allocate POINTSET structure (py)\n");
		free(pPointset->py);
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
	//spi, end
    pPointset->dcd = (int*) malloc(maxnumberofelements*sizeof(int));
    if(pPointset->dcd==NULL)
    {
		printf("not able to allocate POINTSET structure (dcd)\n");
		free(pPointset->py);
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return NULL;
    }
    
    
    //pPointset->pfVoronoiDensity = (float*) malloc(maxnumberofelements*sizeof(float));
    //if(pPointset->pfVoronoiDensity==NULL)
    //{
	//	printf("not able to allocate POINTSET structure (pfVoronoiDensity)\n"); 
	//	free(pPointset->dcd);
	//	free(pPointset->py);
	//	free(pPointset->px);
	//	free(pPointset->cty);
	//	free(pPointset->ctx);
	//	free(pPointset->nt[0]);
	//	free(pPointset->vt[0]);
	//	free(pPointset);
	//	return NULL;
    //}
    
    
    //
    // now, initialize all variables
    //
    pPointset->npts=0;
    pPointset->ntri=0;
    pPointset->xmin = MAXDBL;
    pPointset->ymin = MAXDBL;
    pPointset->xmax = MINDBL;
    pPointset->ymax = MINDBL;
    pPointset->maxnumberofelements = maxnumberofelements;

    pPointset->pfStatistics = NULL; /* should be allocated later by UI */
    pPointset->nStatPerPoint = 0;
    pPointset->nSizeStatInByte = 0;

    return pPointset;
}


LONG ReallocPointset(POINTSET* pPointset, long new_maxnumberofelements)
{
    if(pPointset==NULL) return FALSE;

	/*
    pPointset = (POINTSET*) realloc(pPointset, sizeof(POINTSET));
    if(pPointset==NULL)
    {
		printf("not able to reallocate POINTSET structure\n");
		return NULL;
    }
	*/

    //
    // alloc array vt
    //
    pPointset->vt[0] = (int*) realloc(pPointset->vt[0], 3*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)*sizeof(int));
    if(pPointset->vt[0]==NULL)
    {
		printf("not able to allocate POINTSET structure (vt)\n");
		free(pPointset);
		return FALSE;
    }
    pPointset->vt[1] = (int*) &(pPointset->vt[0][1*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    pPointset->vt[2] = (int*) &(pPointset->vt[0][2*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    //
    // alloc array nt
    //
    pPointset->nt[0] = (int*) realloc(pPointset->nt[0], 3*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)*sizeof(int));
    if(pPointset->nt[0]==NULL)
    {
		printf("not able to allocate POINTSET structure (nt)\n");
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
    pPointset->nt[1] = (int*) &(pPointset->nt[0][1*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    pPointset->nt[2] = (int*) &(pPointset->nt[0][2*(new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS)]);
    //
    // alloc ctx, cty
    //
    pPointset->ctx = (double*) realloc(pPointset->ctx, new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS*sizeof(double));
    if(pPointset->ctx==NULL)
    {
		printf("not able to allocate POINTSET structure (ctx)\n");
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
    pPointset->cty = (double*) realloc(pPointset->cty, new_maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS*sizeof(double));
    if(pPointset->cty==NULL)
    {
		printf("not able to allocate POINTSET structure (cty)\n");
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
    pPointset->px = (double*) realloc(pPointset->px, new_maxnumberofelements*sizeof(double));
    if(pPointset->px==NULL)
    {
		printf("not able to allocate POINTSET structure (px)\n");
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
    pPointset->py = (double*) realloc(pPointset->py, new_maxnumberofelements*sizeof(double));
    if(pPointset->py==NULL)
    {
		printf("not able to allocate POINTSET structure (py)\n");
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
	//spi, begin
    pPointset->controlratio = (float*) realloc(pPointset->controlratio, new_maxnumberofelements*sizeof(float));
    if(pPointset->controlratio==NULL)
    {
		printf("not able to allocate POINTSET structure (py)\n");
		free(pPointset->py);
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
	//spi, end
    pPointset->dcd = (int*) realloc(pPointset->dcd, new_maxnumberofelements*sizeof(int));
    if(pPointset->dcd==NULL)
    {
		printf("not able to allocate POINTSET structure (dcd)\n");
		free(pPointset->py);
		free(pPointset->px);
		free(pPointset->cty);
		free(pPointset->ctx);
		free(pPointset->nt[0]);
		free(pPointset->vt[0]);
		free(pPointset);
		return FALSE;
    }
    
    
    //
    // now, initialize all variables
    //
	/*
    pPointset->npts=0;
    pPointset->ntri=0;
    pPointset->xmin = MAXDBL;
    pPointset->ymin = MAXDBL;
    pPointset->xmax = MINDBL;
    pPointset->ymax = MINDBL;
	*/
    pPointset->maxnumberofelements = new_maxnumberofelements;

	/*
    pPointset->pfStatistics = NULL; // should be allocated later by pointset library user 
    pPointset->nStatPerPoint = 0;
    pPointset->nSizeStatInByte = 0;
	*/
    return TRUE;
}


//////////////////////
// Pointset Destructor
//////////////////////
void DeletePointset(POINTSET* pPointset)
{
	// free(pPointset->pfVoronoiDensity); 
    free(pPointset->dcd);
	//spi, begin
	free(pPointset->controlratio);
	//spi, end
    free(pPointset->py);
    free(pPointset->px);
    free(pPointset->cty);
    free(pPointset->ctx);
    free(pPointset->nt[0]);
    free(pPointset->vt[0]);
    if(pPointset->pfStatistics) DeletePointsetStatistics(pPointset);
    free(pPointset);

    pPointset = NULL;
    return;
}

POINTSET* NewPointsetCopy(POINTSET* pPointset)
{
	POINTSET* pNewPointset = NULL;
	if(pPointset==NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}
	//pNewPointset->maxnumberofelements = pPointset->maxnumberofelements;
	pNewPointset = NewPointset(pPointset->maxnumberofelements);
	if(pNewPointset==NULL) return NULL;
	ASSERT(pNewPointset->maxnumberofelements==pPointset->maxnumberofelements);

	pNewPointset->ntri = pPointset->ntri;
	int tri_size = pPointset->maxnumberofelements*POINTSET_NUMTRIANGLESOVERNUMPOINTS;
	int tri_sizeinbyte = 3*tri_size*sizeof(int);
	//vt
    memcpy(pNewPointset->vt[0], pPointset->vt[0], tri_sizeinbyte);
    pNewPointset->vt[1] = (int*) &(pNewPointset->vt[0][1*tri_size]);
    pNewPointset->vt[2] = (int*) &(pNewPointset->vt[0][2*tri_size]);
	//nt
    memcpy(pNewPointset->nt[0], pPointset->nt[0], tri_sizeinbyte);
    pNewPointset->nt[1] = (int*) &(pNewPointset->nt[0][1*tri_size]);
    pNewPointset->nt[2] = (int*) &(pNewPointset->nt[0][2*tri_size]);

	memcpy(pNewPointset->ctx, pPointset->ctx, sizeof(double)*pPointset->maxnumberofelements); 
	memcpy(pNewPointset->cty, pPointset->cty, sizeof(double)*pPointset->maxnumberofelements); 

	pNewPointset->npts = pPointset->npts;
	memcpy(pNewPointset->px , pPointset->px, sizeof(double)*pPointset->maxnumberofelements);
	memcpy(pNewPointset->py, pPointset->py, sizeof(double)*pPointset->maxnumberofelements);
	//spi, begin
	memcpy(pNewPointset->controlratio, pPointset->controlratio, sizeof(float)*pPointset->maxnumberofelements);
	//spi, end
	memcpy(pNewPointset->dcd, pPointset->dcd, sizeof(int)*pPointset->maxnumberofelements); 

	NewPointsetStatistics(pNewPointset, pPointset->maxnumberofelements, 4);
	memcpy(pNewPointset->pfStatistics, pPointset->pfStatistics, pPointset->nSizeStatInByte); 
	pNewPointset->nStatPerPoint = pPointset->nStatPerPoint;     
	pNewPointset->nSizeStatInByte = pPointset->nSizeStatInByte;   

	pNewPointset->xmin = pPointset->xmin;
	pNewPointset->xmax = pPointset->xmax;
	pNewPointset->ymin = pPointset->ymin;
	pNewPointset->ymax = pPointset->ymax;
	strcpy(pNewPointset->filename, pPointset->filename);
	pNewPointset->loptim = pPointset->loptim;
	return pNewPointset;
}

void TranslatePointset(POINTSET* pPointset, double x_offset, double y_offset)
{
	if(pPointset==NULL)
	{
		ASSERT(FALSE);
		return;
	}
	for(int i=0; i<pPointset->ntri; i++)
	{
		pPointset->ctx[i] += x_offset; 
		pPointset->cty[i] += y_offset; 
	}
	for(int i=0; i<pPointset->npts; i++)
	{
		pPointset->px[i] += x_offset;
		pPointset->py[i] += y_offset;
	}
	pPointset->xmin += x_offset;
	pPointset->xmax += x_offset;
	pPointset->ymin += y_offset;
	pPointset->ymax += y_offset;
}

LONG NewPointsetStatistics(POINTSET* pPointset, long maxnumberofelements, long statperelement)
{
	if(pPointset==NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	//2012july24, poirier, begin
	/*
	//for the time being, we do not track the number of stat indenpendently
	ASSERT(maxnumberofelements==pPointset->maxnumberofelements);
	*/
	if(pPointset->pfStatistics!=NULL) 
	{
		ASSERT(FALSE);
		return FALSE;
	}
	//2012july24, poirier, end

	// optional, allocate pPointset->pfStatistics 
	pPointset->nStatPerPoint = statperelement + POINTSET_EXTRA_NUMBEROFSTAT; // POINTSET_EXTRA_NUMBEROFSTAT, +1 for voroarea 
	pPointset->nSizeStatInByte = sizeof(double)	* maxnumberofelements *(pPointset->nStatPerPoint); // +1 for voroarea 

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// allocate twice as much for list of merge segments 	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	pPointset->nSizeStatInByte = pPointset->nSizeStatInByte * 2;				    				
	pPointset->pfStatistics = (double*) malloc(pPointset->nSizeStatInByte); 
	if( pPointset->pfStatistics == NULL )
	{
	    return FALSE; //error, return safely
	}

	return TRUE;
}

LONG ReallocPointsetStatistics(POINTSET* pPointset, long new_maxnumberofelements)
{
	if(pPointset==NULL) return FALSE;

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// allocate twice as much for list of merge segments 	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	pPointset->nSizeStatInByte = pPointset->nStatPerPoint * new_maxnumberofelements * sizeof(double) * 2;
    pPointset->pfStatistics = (double*) realloc(pPointset->pfStatistics, pPointset->nSizeStatInByte);
	if(pPointset->pfStatistics == NULL) 
	{
		ASSERT(FALSE); //reallocation error, pfStatistics is now corrupted
		return FALSE;
	}
	return TRUE;
}

void DeletePointsetStatistics(POINTSET* pPointset)
{
	ASSERT(pPointset!=NULL && pPointset->pfStatistics!=NULL);
	free(pPointset->pfStatistics);
	pPointset->pfStatistics = NULL;
	return;
}

double* GetPointsetPointerToStatistics(POINTSET* pPointset, long ielement)
{	
	double* pfStatistics;
	ASSERT(ielement>-1 && ielement<pPointset->maxnumberofelements);
	
	//long i_baseoffset = ielement*pPointset->nStatPerPoint*sizeof(double);
	//pfStatistics = pPointset->pfStatistics + i_baseoffset;
	long i_baseindex = ielement*(pPointset->nStatPerPoint);
	pfStatistics = &(pPointset->pfStatistics[i_baseindex]);
	return pfStatistics;
}





void BuildTriangleNetwork(POINTSET* pPointset)
{
    int	   	nptf,nptfs,nptfi;
    int	   	i,k,l,nl;
	//int 	ii;
    int     it;
    int     nrej1;
    int     nrej2;
    double  strip;
    double  dxstrip,dystrip;
    double  px1,px2,py1,py2,aux;
    double  m,q,x0;
    //float h;

    // were outside the main scope, module globals 
    int vtm[3],ntm[3],ntr[3];

    pPointset->loptim=0;
    strip=0.05;

    dxstrip = strip*(pPointset->xmax-pPointset->xmin);
    dystrip = strip*(pPointset->ymax-pPointset->ymin);
    px1=pPointset->xmin-dxstrip;
    py1=pPointset->ymin-dystrip;
    aux=pPointset->npts;
    nl= 1 + (int)pow(aux,0.25);
    px2=(pPointset->xmax-pPointset->xmin+2*dxstrip)/nl;
    py2=(pPointset->ymax-pPointset->ymin+2*dystrip)/nl;
    for (i=0;i<pPointset->npts;i++)
    {
		int nx,ny,k1,k2;
		nx=1+(int)((pPointset->px[i]-px1)/px2);
		ny=1+(int)((pPointset->py[i]-py1)/py2);
		k1=(ny%2 == 0) ? 1 : -1;
		k2=(1-k1)/2;
		pPointset->vt[0][i]=nl*ny-k1*nx-k2*(nl+1)+1;
		pPointset->vt[1][i]=i;
		pPointset->vt[2][i]=i;
    }

	//poirier, sept 2001, begin
	/*
    printf(" tri ...1\n");
	*/
#ifdef _DEBUG	
	/* //spi 2014
	CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
	CString myString;
	myString.Format(" first sort ...\r\n");
	pLogDocument->AddText(myString);
	*/
#endif //_DEBUG
	//poirier, sept 2001, end
    fsort(pPointset,pPointset->vt[2],0,i-1,xcmp);

	//poirier, sept 2001, begin
	/*
    printf(" tri ...2\n");
	*/
    fsort(pPointset,pPointset->vt[1],0,i-1,bcmp);
#ifdef _DEBUG
	/* //spi 2014
	myString.Format(" second sort ...\r\n");
	pLogDocument->AddText(myString);
	*/
#endif //_DEBUG
	//poirier, sept 2001, end

    /////////////////////////////////////////////////////////////////////
    //Creation de la frontiere convexe autour de tous les points d'entree
    /////////////////////////////////////////////////////////////////////
    printf(" creation de la frontiere convexe ...\n");
    x0=pPointset->px[pPointset->vt[2][0]];
    q=pPointset->py[pPointset->vt[2][0]];
    m=(pPointset->py[pPointset->vt[2][pPointset->npts-1]]-q)/(pPointset->px[pPointset->vt[2][pPointset->npts-1]]-x0);
    nptfi=1;
    nptfs=pPointset->npts;
    pPointset->nt[0][0]=pPointset->vt[2][0];
    for (i=1; i<pPointset->npts; i++)
    {
		int ind;
		ind=pPointset->vt[2][i];
		if ((pPointset->py[ind]-q)>=m*(pPointset->px[ind]-x0))
		{
			nptfs--;
			pPointset->nt[0][nptfs]=ind;
		}
		else
		{
			pPointset->nt[0][nptfi]=ind;
			nptfi++;
		}
    }
    nptf=1;
    for (i=2; i<pPointset->npts; i++)
    {
		//poirier, march 27 1997, with new data set we encounter the condition where nptf=0
		//and ckturn() cannot be evaluated because of second parameter, nptf = 0
		//while((ckturn(pPointset,pPointset->nt[0],nptf,i) > 0) && (nptf >= 1))
		while((nptf >= 1) && (ckturn(pPointset,pPointset->nt[0],nptf,i) > 0))
		{
			nptf--;
		}
		nptf++;
		pPointset->nt[0][nptf]=pPointset->nt[0][i];
    }
    while((ckturn(pPointset,pPointset->nt[0],nptf,0) > 0) && (nptf >= 1)) nptf--;
    for (i=0; i<=nptf; i++)
    {
		int ind;
		ind=pPointset->nt[0][i];
		pPointset->dcd[i]=ind;
		pPointset->vt[0][ind]=0;
    }
    k=nptf;
    for (i=0; i<pPointset->npts; i++)
    {
		int ind;
		ind=pPointset->vt[1][i];
		if (pPointset->vt[0][ind] != 0)
		{
			k++;
			pPointset->dcd[k]=ind;
		}
    }
    pPointset->ntri=nptf-2;
    for (i=0; i<=pPointset->ntri; i++)
    {
		pPointset->vt[0][i]=pPointset->dcd[0];
		pPointset->vt[1][i]=pPointset->dcd[i+1];
		pPointset->vt[2][i]=pPointset->dcd[i+2];
		pPointset->nt[0][i]=i+1;
		pPointset->nt[1][i]=i-1;
		pPointset->nt[2][i]=-1;
		optim(pPointset,i,2);
    }
    pPointset->nt[0][pPointset->ntri]=-1;
    printf("debut de la triangulation ...\n");
    it=0;
    nrej1=0;
    nrej2=0;
    for (i=nptf+1; i<pPointset->npts; i++)
    {
		int it1;
		nl=pPointset->dcd[i];
		it1=FindTriContainingVertex(pPointset,nl,it);
		if (it1 >=0)
		{
			 it=it1;
			 ntr[0]=it;
			 ntr[1]=pPointset->ntri+1;
			 ntr[2]=pPointset->ntri+2;
			 for (k=0; k<3;k++)
			 {
				vtm[k]=pPointset->vt[k][it];
				ntm[k]=pPointset->nt[k][it];
			 }
			 for (k=0; k<3; k++)
			 {
				int ntrk,knd,krd;
				ntrk=ntr[k];
				knd=jnd[k];
				krd=jrd[k];
				pPointset->vt[0][ntrk]=nl;
				pPointset->vt[1][ntrk]=vtm[k];
				pPointset->vt[2][ntrk]=vtm[knd];
				pPointset->nt[0][ntrk]=ntr[knd];
				pPointset->nt[1][ntrk]=ntr[krd];
				pPointset->nt[2][ntrk]=ntm[knd];
			 }
			 for ( k=1; k<3; k++)
			 {
				int ind;
				ind=pPointset->nt[2][ntr[k]];
				if (ind >=0)
				{
					for (l=0; l<3; l++)
					{
						if (pPointset->nt[l][ind] == ntr[0])
						{
							pPointset->nt[l][ind]=ntr[k];
						}
					}
				}
			 }
			 pPointset->ntri+=2;
			 optim(pPointset,ntr[0],0);
			 optim(pPointset,ntr[1],0);
			 optim(pPointset,ntr[2],0);
		}
		else
		{
			nrej1++;
			if (it1==-2) nrej2++;
		}
    }
	//poirier, march 27 1997
#ifdef _DEBUG
	/* */
	//CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
	//CString myString;
	/* //spi 2014
	myString.Format("nombre de points qui coincident =  %d\r\n",nrej2);
	pLogDocument->AddText(myString);
	myString.Format("nombre de points rejetes        =  %d\r\n",nrej1);
	pLogDocument->AddText(myString);
	myString.Format("nombre de triangles construits  =  %d\r\n",pPointset->ntri+1);
	pLogDocument->AddText(myString);
	*/
	/* */
#endif //_DEBUG

	/*
    printf("nombre de points qui coincident =  %d\n",nrej2);
    printf("nombre de points rejetes        =  %d\n",nrej1);
    printf("nombre de triangles construits  =  %d\n",pPointset->ntri+1);
	*/
    return;
}





void save_triangle_to_file(POINTSET* pPointset)
{
    int i,k;
    FILE* trstrm;
    FILE* trstrm2;
    FILE* ctrstrm;

    /////////////////////////////////////////////////////////
    //Creation de fichiers de sortie avec differentes donnees.
    /////////////////////////////////////////////////////////
    if ((trstrm=fopen("debug_vertri.dat","w")) == NULL)
    {
		printf("trimake: ne peut ouvrir le fichier de sortie '%s'\n","vertri.dat");
		ASSERT(FALSE);
		exit(1);
    }
    if ((trstrm2=fopen("debug_coortri.dat","w")) == NULL)
    {
		printf("trimake: ne peut ouvrir le fichier de sortie '%s'\n","coortri.dat");
		ASSERT(FALSE);
		exit(1);
    }
    if ((ctrstrm=fopen("debug_centre.dat","w")) == NULL)
    {
		printf("trimake: ne peut ouvrir le fichier de sortie '%s'\n","centre.dat");
		ASSERT(FALSE);
		exit(1);
    }
    printf("stockage des resultats ...\n");


    for (i=0; i<=pPointset->ntri; i++)
    {
	 //
	 //Pour chacun des triangles, sauvegarde: v1,v2,v3,t1,t2,t3
	 //
	 // printf(" \ntriangle n0:  %d\n",i); 
	 fprintf(trstrm," %d",i);
	 for (k=0; k<3; k++)
	 {
	    fprintf(trstrm," %d",pPointset->vt[k][i]);// printf(" %d",pPointset->vt[k][i]);
	 }
	 fprintf(trstrm," %d",pPointset->nt[0][i]);//printf(" %d",pPointset->nt[1][i]); 
	 fprintf(trstrm," %d",pPointset->nt[1][i]);//printf(" %d",pPointset->nt[0][i]); 
	 fprintf(trstrm," %d",pPointset->nt[2][i]);//printf(" %d",pPointset->nt[2][i]); 

	 //
	 //Pour chacun des triangles, sauvegarder:
	 //                  v1.x,v1.y,v2.x,v2.y,v3.x,v3.y
	 //
	 fprintf(trstrm2," %d",i);
	 for (k=0; k<3; k++)
	 {
	    fprintf(trstrm2," %f %f",pPointset->px[pPointset->vt[k][i]],pPointset->py[pPointset->vt[k][i]]);
	 }
	 // fine de ligne 
	 fputc('\n',trstrm);
	 fputc('\n',trstrm2);

	 //
	 //Sauvegarder chacun des centre dans un fichier
	 //
	 fprintf(ctrstrm,"%d  %f %f\n", i, pPointset->ctx[i], pPointset->cty[i]);
    }
    fclose(trstrm);
    fclose(trstrm2);
    fclose(ctrstrm);
    return;
}




void save_neighbor_and_center_to_file(POINTSET* pPointset)
{
    int ivertex,j;
    FILE* strm;

    int numtrifound, itriseed;
    int p_arraytri[200];
    int p_arrayneighbor[200];

    // the triangle seed must exist 
    itriseed = 0;

    ///////////////////////////////
    //Creation du fichier de sortie
    ///////////////////////////////
    if ((strm=fopen("neighbor.dat","w")) == NULL)
    {
		printf("trimake: ne peut ouvrir le fichier de sortie '%s'\n","neighbor.dat");
		ASSERT(FALSE);
		exit(1);
    }

    for (ivertex=0; ivertex<pPointset->npts; ivertex++)
    {
	 //
	 //Pour chacun des points ...
	 //

	 if( FindAllTriSurroundingVertex( pPointset,
					  ivertex,
					  &itriseed,
					  &numtrifound,
					  p_arraytri,
					  p_arrayneighbor) )
	 {
	     fprintf(strm," %d  %d",ivertex, numtrifound);
	     for(j=0; j<numtrifound; j++)
	     {
		 fprintf(strm," %d",p_arrayneighbor[j]);
	     }
	     fprintf(strm, " / ");
	     for(j=0; j<numtrifound; j++)
	     {
		 fprintf(strm," %d",p_arraytri[j]);
	     }
	     fprintf(strm, "\n");
	 }
    }
    fclose(strm);
    return;
}



void save_2nd_order_neighbors_from_memory(POINTSET* pPointset)
{
    FILE* strm;
    int j,itri,iorder,numvertexfound;
    int arrayneighbor[200];

    ///////////////////////////////
    //Creation du fichier de sortie
    ///////////////////////////////
    iorder = 2;
    if ((strm=fopen("2ndorder.dat","w")) == NULL)
    {
		printf("trimake: ne peut ouvrir le fichier de sortie '%s'\n","2ndorder.dat");
		ASSERT(FALSE);
		exit(1);
    }

    for(itri=0; itri<=pPointset->ntri; itri++)
    {
		numvertexfound = 0;
	if ( FindAllNeighborSurroundingTri( pPointset,
					    itri,
					    iorder,
					    &numvertexfound,
					    arrayneighbor) )
	{
	     fprintf(strm, "itri=%d, n=%d,  ",itri,numvertexfound);
	     for(j=0; j<numvertexfound; j++)
	     {
		 fprintf(strm, " %d", arrayneighbor[j]);
	     }
	     fprintf(strm, "\n");
	}
    }
    fclose(strm);
    return;
}



//////////////////////////////////////////
//  swapv - echange deux positions memoire
//////////////////////////////////////////
void swapv(int* pv,
		   int i1,
		   int i2)
{
    int pt;
    pt=pv[i1];
    pv[i1]=pv[i2];
    pv[i2]=pt;
    return;
}


////////////////////////////////////////////////////////////////
// ckturn - verifie si trois sommets sont:
//					  alignes      (0)
//					  anti-horaire (-1)
//					  horaire      (1)
//					  coincident   (2)
////////////////////////////////////////////////////////////////
int ckturn(POINTSET* pPointset,
		   int* pv,
		   int i2,
		   int i3)
{
   double aux;
   int i1;
   i1=i2-1;
   aux=(pPointset->py[pv[i2]]-pPointset->py[pv[i1]])*(pPointset->px[pv[i3]]-pPointset->px[pv[i1]])-
       (pPointset->py[pv[i3]]-pPointset->py[pv[i1]])*(pPointset->px[pv[i2]]-pPointset->px[pv[i1]]);
   if (aux > DBERR) return 1;
   if (aux < -DBERR) return -1;
   aux=(pPointset->px[pv[i3]]-pPointset->px[pv[i2]])*(pPointset->px[pv[i2]]-pPointset->px[pv[i1]])+
       (pPointset->py[pv[i3]]-pPointset->py[pv[i2]])*(pPointset->py[pv[i2]]-pPointset->py[pv[i1]]);
   if (aux <= -DBERR) return 1;
   if (aux >= DBERR) return 0;
   return 2;
}




/////////////////////////////////////////////////////////////////////////
//  cklop - verifie si quatre points satisfont l'optimisation local
//	     cklop = 1 si cela est vrai
//	     cklop = 0 sinon
/////////////////////////////////////////////////////////////////////////
int cklop(POINTSET* pPointset,
		  int i1,
		  int i2,
		  int i3,
		  int i4)
{
     double x2sq,y2sq;
     double a,b,c,d,e,f,xc,yc,rsq;
     y2sq=pow(pPointset->py[i2],2);
     x2sq=pow(pPointset->px[i2],2);
     a=2*(pPointset->px[i2]-pPointset->px[i1]);
     b=2*(pPointset->py[i2]-pPointset->py[i1]);
     c=y2sq-pow(pPointset->py[i1],2)+x2sq-pow(pPointset->px[i1],2);
     d=2*(pPointset->px[i3]-pPointset->px[i2]);
     e=2*(pPointset->py[i3]-pPointset->py[i2]);
     f=pow(pPointset->py[i3],2)-y2sq+pow(pPointset->px[i3],2)-x2sq;
     x2sq=d*b-e*a;
     rsq=pow(x2sq,2);
     if (rsq > 0.)
      {
       xc=(f*b-c*e)/x2sq;
       yc=(d*c-f*a)/x2sq;
       rsq=pow((xc-pPointset->px[i1]),2)+pow((yc-pPointset->py[i1]),2);
       if ((pow((pPointset->py[i4]-yc),2)+pow((pPointset->px[i4]-xc),2)) > rsq) return 1;
      }
     return 0;
}



/////////////////////////////////////////////////////////////////////////
//   optim - optimise deux triangle, procedure d'optimisation locale
/////////////////////////////////////////////////////////////////////////
int optim(POINTSET* pPointset,
		  int it,
		  int k1)
{
     //int i9;
     int nea;
     int i,i1,k2,k3;
     if (pPointset->loptim == MAXOPT) return TRUE;
     k2=jnd[k1];
     k3=jrd[k1];
     nea=pPointset->nt[k3][it];
     if (nea == -1) return TRUE;
     pPointset->loptim ++;
     for (i=0; i<3; i++) if(pPointset->nt[i][nea] == it ) break;
     if (i == 3)
     {
		printf(" erreur dans optim \n");
		ASSERT(FALSE);
		exit(0);
		//2012august19, poirier, begin
		//return FALSE;
		//2012august19, poirier, end
     }
     i1=jnd[i];
     if (cklop(pPointset,pPointset->vt[k3][it],pPointset->vt[k1][it],pPointset->vt[k2][it],pPointset->vt[i1][nea]) == 0)
     {
	swaptr(pPointset,it,nea,k1,i);
	optim(pPointset,it,k1);
	optim(pPointset,nea,i);
     }
     pPointset->loptim --;
     return TRUE;
}




//////////////////////////////////////////////////////////
// swaptr - retourne les deux diagonales d'un quadrilatere
//////////////////////////////////////////////////////////
int swaptr(POINTSET* pPointset,
		   int t1,
		   int t2,
		   int k1,
		   int k2)
{
    int save,i,ind1,ind2;
    pPointset->vt[jrd[k1]][t1]=pPointset->vt[jnd[k2]][t2];
    pPointset->vt[k2][t2]=pPointset->vt[k1][t1];
    save=pPointset->nt[k1][t1];
    pPointset->nt[k1][t1]=t2;
    pPointset->nt[jrd[k1]][t1]=pPointset->nt[jnd[k2]][t2];
    pPointset->nt[jnd[k2]][t2]=t1;
    pPointset->nt[k2][t2]=save;
    ind1=pPointset->nt[jrd[k1]][t1];
    ind2=pPointset->nt[k2][t2];
    for ( i=0; i<3; i++)
    {
      if (ind1 != -1 && pPointset->nt[i][ind1] == t2) pPointset->nt[i][ind1]=t1;
      if (ind2 != -1 && pPointset->nt[i][ind2] == t1) pPointset->nt[i][ind2]=t2;
    }
     return TRUE;
}



//////////////////////////////////////////////////////////////
//   xcmp - routine appelee par fsort
//	   elle compare deux abscisses
//	   si elles sont egales elle compare les ordonnees
//////////////////////////////////////////////////////////////
int xcmp(POINTSET* pPointset,
		 int* i1,
		 int* i2)
{
   double aux;
   aux=pPointset->px[*i1]-pPointset->px[*i2];
   if (aux > 0.) return 1;
   if (aux < 0. ) return -1;
   if (aux == 0.)
   {
     aux=pPointset->py[*i1]-pPointset->py[*i2];
     if (aux > 0.) return 1;
     if (aux < 0.) return -1;
   }
   return 0;
}



////////////////////////////////////////////////
//   bcmp - routine appelee par fsort
//	  elle compare deux nombres binaires
////////////////////////////////////////////////
int bcmp(POINTSET* pPointset,
		 int* i1,
		 int* i2)
{
    return pPointset->vt[0][*i1]-pPointset->vt[0][*i2];
}



////////////////////////////////////
// fsort - routine de tri
////////////////////////////////////
void fsort(POINTSET* pPointset,
		   int* pv,
		   int i1,
		   int i2,
		   int (*fnz)(POINTSET*, int*, int*))
{
  int i,j,k;
  i=i1;
  j=i2;
  k=0;
  while (i != j)
   {
    if (fnz(pPointset,&(pv[i]),&(pv[j])) > 0)
     {
      swapv(pv,i,j);
      k++;
     }
    if (k % 2 == 0) i++;
    else j--;
   }
  i--;j++;
  if (i2-j >0) fsort(pPointset,pv,j,i2,fnz);
  if (i-i1 >0) fsort(pPointset,pv,i1,i,fnz);
  return;
}
 // fin de trimake 




///////////////////////////////////////////////////////////////
// calcul_centre_triangle():
//
// fonction ajoutee pour visualisation regions Voronoi
//
// Cette fonction calcul le centre de chacun des triangles d'une
// liste donnees.  Par centre, nous referons au centre du cercle
// circonscrit au triangle (ou intersection des mediatrices du tri).
//
//			 	 ...
//		     v1______v2
//		     . \     / .
//		     .	\   /  .
//		      .	 \ /  .
//			 	. v3.
//
//
//   Reference:  datei (module) voronoi.c, Gerhard Albers,
//		 function center(i,j,k), 23 sept 1991 (public domain).
//
//   Remerciement particulier:	Francois Anton, pour discussion
//				au sujet du calcul des centres.
///////////////////////////////////////////////////////////////
void ComputeAllTriangleCenters(POINTSET* pPointset)
{
    for(int idtri=0; idtri<=pPointset->ntri; idtri++)
    {
		ComputeTriangleCenter(pPointset, idtri);
	}
}

void ComputeTriangleCenter(POINTSET* pPointset, int idtri)
{
    double x1,y1,x2,y2,x3,y3;
    double x12,y12,x13,y13;
    double det,detinv,r12sq,r13sq;
    int i=idtri;

    // uses pPointset->vt[3][pPointset->ntri] for v1,v2, v3 
    //	    pPointset->px[pPointset->npts] for v.x	      
    //	    pPointset->py[pPointset->npts] for v.y	      
    //	    pPointset->ctx[pPointset->ntri] for x centre	 
    //	    pPointset->cty[pPointset->ntri] for y centre	 

    //for(i=0; i<=pPointset->ntri; i++)
    //{
		x3 = pPointset->px[pPointset->vt[0][i]];
		x1 = pPointset->px[pPointset->vt[1][i]];
		x2 = pPointset->px[pPointset->vt[2][i]];
		y3 = pPointset->py[pPointset->vt[0][i]];
		y1 = pPointset->py[pPointset->vt[1][i]];
		y2 = pPointset->py[pPointset->vt[2][i]];
	
		x12 = x2 - x1;
		y12 = y2 - y1;
		x13 = x3 - x1;
		y13 = y3 - y1;
	
		det = x12*y13 - x13*y12;
		if( fabs(det) > DBERR )
		{
		    detinv = 0.5 / det;
		    r12sq = x12*x12 + y12*y12;
		    r13sq = x13*x13 + y13*y13;
		    pPointset->ctx[i] = x1 + detinv*(r12sq*y13 - r13sq*y12);
		    pPointset->cty[i] = y1 + detinv*(r13sq*x12 - r12sq*x13);
		}
		else
		{
		    //mdlDialog_openAlert("trois points trop rapproches");
			printf("trois points trop rapproches\n");
			ASSERT(FALSE);
		    exit(1);
		}
    //}
    return;
}



//////////////////////////////////////////////////////////////////////////
// FindTriContainingVertex(int ivertex, int itriseed)
//
//    Cherche le triangle qui contient le sommet ivertex.  Cette
//    routine est utilisee lors de la construction de la triangulation
//    arbitraire (pre-Delaunay). On construit ainsi la triangulation
//    arbitraire en ajoutant sequentiellement, un a un, les points
//    d'entree (qui seront les sommets des triangles).  A chaque fois,
//    qu'un nouveau point est ajoute a la triangulation partielle
//    existante, on verifie si ce dernier se trouve a l'interieur
//    d'un triangle deja construit ou a l'exterieur (dans les deux cas,
//    la mise a jour de la nouvelle triangulation est une operation
//    "trivale").
//
//
//    ivertex: est l'index du sommet d'interet.
//    itriseed: est l'index du triangle qui sera utilise pour
//		debuter la recherche.
//
//    retourne:  l'index du triangle si succes.
//		 si error: -1 si aucun triangle trouve dans toute la liste.
//			   -2 si le sommet coincide avec un sommet existant.
//////////////////////////////////////////////////////////////////////////
int FindTriContainingVertex(POINTSET* pPointset,
			    int ivertex,
			    int itriseed)
{
     int i,aux,newtr;
     newtr=itriseed;
     do
     {
		 aux=0;
		 for (i=0; i<=2; i++)
		 {
		   double area,x21,x31,y21,y31;
		   int i1,ind1,ind2;
		   i1=jnd[i];
		   ind1=pPointset->vt[i][newtr];
		   ind2=pPointset->vt[i1][newtr];
		   x31=(pPointset->px[ivertex]-pPointset->px[ind1]);
		   x21=(pPointset->px[ind2]-pPointset->px[ind1]);
		   y31=(pPointset->py[ivertex]-pPointset->py[ind1]);
		   y21=(pPointset->py[ind2]-pPointset->py[ind1]);
		   area=y31*x21-y21*x31;
		   if (area <0.)
		   {
		     newtr=pPointset->nt[i1][newtr];
		     if (newtr == -1) return newtr;
		     aux=1;
		     break;
		   }
		   if (area == 0.) if (x31 == 0. && y31 == 0.) return -2;
		 }
     } while (aux ==1);
     return newtr;
}





//////////////////////////////////////////////////////////////////////////
// FindTriContainingPoint(double xa, double ya, int* p_itriseed)
//
//    Cherche le triangle qui contient le point (xa,ya).
//
//
//    xa: coordonnee x du point d'interet.
//    ya: coordonnee y du point d'interet.
//    p_itriseed: est le pointeur a l'index du triangle qui sera
//		  utilise pour debuter la recherche.  ce pointeur
//		  est egalement utilise pour sauvegarder le dernier
//		  triangle trouve.
//
//
//    retourne:  l'index du triangle si succes.
//		 si error: -1 si aucun triangle trouve dans toute la liste.
//			   -2 si (xa,ya) coincide avec un sommet existant.
//			   -99 si erreur avec le pointeur p_itriseed.
//
//		     itriseed
//
//		     v1 --- v2
//		      \    /
//			 	v3
//
//
//
//
//		 .
//		 (xa,ya)
//
//
//////////////////////////////////////////////////////////////////////////
int FindTriContainingPoint(POINTSET* pPointset,
						   double xa,
						   double ya,
						   int* p_itriseed)
{
    int i,aux,newtr;

    // validate input pointer 
    if(p_itriseed == NULL) 
	{
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindTriContainingPoint(), p_itriseed is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
	}

	//poirier, august 1999, begin
	if(p_itriseed[0]>pPointset->ntri)
	{
		/* //spi 2014
		CLogDocument* pLogDoc = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		pLogDoc->AddText("warning, itriseed reseted to zero\r\n");
		*/
		p_itriseed[0] = 0;
	}
	//poirier, august 1999, end
    newtr=p_itriseed[0];
    do
    {
		aux=0;
		for (i=0; i<=2; i++)
		{
			double area,x21,x31,y21,y31;
			int i1,ind1,ind2;
			i1=jnd[i];
			ind1=pPointset->vt[i][newtr];
			ind2=pPointset->vt[i1][newtr];
			x31=(xa-pPointset->px[ind1]);
			x21=(pPointset->px[ind2]-pPointset->px[ind1]);
			y31=(ya-pPointset->py[ind1]);
			y21=(pPointset->py[ind2]-pPointset->py[ind1]);
			area=y31*x21-y21*x31;
			if (area <0.)
			{
				if(pPointset->nt[i1][newtr] == -1)
				{
					// store the last triangle found before going
					//   outside the convex frontier 
					p_itriseed[0] = newtr;
					return -1;
				}
				newtr=pPointset->nt[i1][newtr];
				aux=1;
				break;
			}
			else if (area == 0.)
			{
				if( x31 == 0. && y31 == 0.)
				{
					p_itriseed[0] = newtr;
					return -2;
				}
			}
		} //end of for()
    } while (aux ==1);
    p_itriseed[0] = newtr;
    return newtr;
}

//this version collects all triangle (_CAT) involved in the (n)log(n) search path
//this version has also been modified so the search path stays along a straight line (seed triangle center - destination point)
int FindTriContainingPoint_CAT(	POINTSET* pPointset,
								double xa,
								double ya,
								int* p_itriseed,
								int* p_numtrifound,
								int* p_arraytri)
{
	if(pPointset==NULL || p_itriseed==NULL || p_numtrifound==NULL || p_arraytri==NULL)
	{
		ASSERT(FALSE);
		return -99;
	}
    int i,aux,newtr;

    // validate input pointer 
    if(p_itriseed == NULL) 
	{
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindTriContainingPoint_CAT(), p_itriseed is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
	}

	//poirier, august 1999, begin
	if(p_itriseed[0]>pPointset->ntri)
	{
		/* //spi 2014
		CLogDocument* pLogDoc = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		pLogDoc->AddText("warning, itriseed reseted to zero in FindTriContainingPoint_CAT()\r\n");
		*/
		p_itriseed[0] = 0;
	}
	//poirier, august 1999, end
	p_numtrifound[0] = 0;
    newtr=p_itriseed[0];
	//poirier, oct 2001, begin
	int itri = 0;
	p_numtrifound[0]++;
	p_arraytri[itri] = newtr; //collect all triangle on the search path
	//to force the search path to stay on the imaginary line (seed triangle center xc, yc - destination xa, ya)
	double xc = (pPointset->px[pPointset->vt[0][newtr]] + pPointset->px[pPointset->vt[1][newtr]] + pPointset->px[pPointset->vt[2][newtr]]) / 3.;
	double yc = (pPointset->py[pPointset->vt[0][newtr]] + pPointset->py[pPointset->vt[1][newtr]] + pPointset->py[pPointset->vt[2][newtr]]) / 3.;
	//poirier, oct 2001, end
    do
    {
		aux=0;
		double area[3],x21[3],x31[3],y21[3],y31[3];
		int i_adj[3],idvertex1[3],idvertex2[3];
		for (i=0; i<=2; i++)
		{
			i_adj[i]=jnd[i]; //jnd[] = {1,2,0} to pick adjacent index (counter-clockwise)
			idvertex1[i] = pPointset->vt[i][newtr];
			idvertex2[i] = pPointset->vt[i_adj[i]][newtr];
			x31[i] = (xa-pPointset->px[idvertex1[i]]);
			x21[i] = (pPointset->px[idvertex2[i]]-pPointset->px[idvertex1[i]]);
			y31[i] = (ya-pPointset->py[idvertex1[i]]);
			y21[i] = (pPointset->py[idvertex2[i]]-pPointset->py[idvertex1[i]]);
			area[i]=y31[i]*x21[i]-y21[i]*x31[i];
		} //end of for()
		int i_smallest=0;
		double f_area_smallest=0.0;
		BOOL bIntersect[3];
		for(i=0; i<=2; i++)
		{
			
			bIntersect[i] = LineSegmentsIntersect(	pPointset->px[idvertex1[i]], 
													pPointset->py[idvertex1[i]], 
													pPointset->px[idvertex2[i]], 
													pPointset->py[idvertex2[i]], //line segment a
													xc, yc, xa, ya); //line segment b
			
			//pick smallest area (largest negative area)
			if(area[i]<0.)
			{
				if(area[i]<f_area_smallest)
				{
					i_smallest = i;
					f_area_smallest = area[i];
				}
			}
		}
		i=i_smallest;
		//pick smallest area triangle that is also intersecting imaginary axis (directional line) 
		f_area_smallest = 0.0;
		int i_smallest_and_intersecting = -1;
		for(i=0; i<=2; i++)
		{
			if(area[i]<f_area_smallest && bIntersect[i]==TRUE)
			{
				i_smallest_and_intersecting = i;
				f_area_smallest = area[i];
			}
		}
		if(i_smallest_and_intersecting<0) i = i_smallest;
		  else i=i_smallest_and_intersecting;
		/*
		#ifdef _DEBUG
				CString myString;
				myString.Format("area picked: %f (from %f %f %f) (%d %d %d)\r\n", area[i], area[0], area[1], area[2], bIntersect[0], bIntersect[1], bIntersect[2]);
				((COIIIApp*)AfxGetApp())->GetLogDocument()->AddText(myString);
		#endif //_DEBUG
		*/

		//ASSERT(area[i]==f_area_smallest);
		if (area[i]==0.)
		{
			//case on the vertex
			if( x31[i]==0. && y31[i]==0.)
			{
				p_itriseed[0] = newtr;
				return -2;
			}
		}
		else if (area[i] <0.)
		{
			if(pPointset->nt[i_adj[i]][newtr] == -1)
			{
				// store the last triangle found before going
				//   outside the convex frontier 
				p_itriseed[0] = newtr;
				return -1;
			}
			newtr=pPointset->nt[i_adj[i]][newtr];
			aux=1;
			//poirier, oct 2001, begin
			itri++;
			p_numtrifound[0]++;
			p_arraytri[itri] = newtr; //collect all triangle on the search path
			//poirier, oct 2001, end
			continue;
		}
		else
		{
			//all areas greater than zero
			//ASSERT(FALSE);
		}
    } while (aux ==1);
    p_itriseed[0] = newtr;
    return newtr;
}

//this version collects all triangle (_CAT) and vertex (_AV) involved in the (n)log(n) search path
//this version has also been modified so the search path stays along a straight line (seed triangle center - destination point)
//
//
//  xc,yc  .      .          .   .          .
//             .      .         .     .          .  xa,ya
//
//triangle vertices collected sequentially as new triangle is found, independently of
//the side of the line (xc,yc) (xa,ya) where a given vertex lies (default vertex sort)
int FindTriContainingPoint_CATAV(	POINTSET* pPointset,
									double xa,
									double ya,
									int* p_itriseed,
									int* p_numtrifound,
									int* p_arraytri,
									int* p_numvertexfound,
									int* p_arrayvertex)
{
    // validate input pointer 
	if(pPointset==NULL || p_itriseed==NULL 
		|| p_numtrifound==NULL || p_arraytri==NULL
		|| p_numvertexfound==NULL || p_arrayvertex==NULL)
	{
		ASSERT(FALSE);
		return -99;
	}
    if(p_itriseed == NULL) 
	{
		/* spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindTriContainingPoint_CATAV(), p_itriseed is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
	}
	if(p_itriseed[0]>pPointset->ntri)
	{
		/* //spi 2014
		CLogDocument* pLogDoc = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		pLogDoc->AddText("warning, itriseed reseted to zero in FindTriContainingPoint_CATAV()\r\n");
		*/
		p_itriseed[0] = 0;
	}


    int i,aux,newtr;
	p_numtrifound[0] = 0;
    newtr=p_itriseed[0];
	//to collect all triangles on the search path
	int itri = 0;
	p_numtrifound[0]++;
	p_arraytri[itri] = newtr; //first triangle id
	//to collect all vertex on the search path
	int prev_idvertex[3]; //always store last triangle's vertex to avoid adding same vertex twice
	prev_idvertex[0] = pPointset->vt[0][newtr];
	prev_idvertex[1] = pPointset->vt[1][newtr];
	prev_idvertex[2] = pPointset->vt[2][newtr];
	int ivertex = 0;
	p_numvertexfound[0] = 0;
	p_numvertexfound[0]++;
	p_arrayvertex[ivertex] = prev_idvertex[0]; //first vertex id
	ivertex++;
	p_numvertexfound[0]++;
	p_arrayvertex[ivertex] = prev_idvertex[1]; //second vertex id
	ivertex++;
	p_numvertexfound[0]++;
	p_arrayvertex[ivertex] = prev_idvertex[2]; //third vertex id

	//to force the search path to stay on the imaginary line (seed triangle center xc, yc - destination xa, ya)
	double xc = (pPointset->px[pPointset->vt[0][newtr]] + pPointset->px[pPointset->vt[1][newtr]] + pPointset->px[pPointset->vt[2][newtr]]) / 3.;
	double yc = (pPointset->py[pPointset->vt[0][newtr]] + pPointset->py[pPointset->vt[1][newtr]] + pPointset->py[pPointset->vt[2][newtr]]) / 3.;

    do
    {
		aux=0;
		double area[3],x21[3],x31[3],y21[3],y31[3];
		int i_adj[3],idvertex1[3],idvertex2[3];
		for (i=0; i<=2; i++)
		{
			i_adj[i]=jnd[i]; //jnd[] = {1,2,0} to pick adjacent index (counter-clockwise)
			idvertex1[i] = pPointset->vt[i][newtr];
			idvertex2[i] = pPointset->vt[i_adj[i]][newtr];
			x31[i] = (xa-pPointset->px[idvertex1[i]]);
			x21[i] = (pPointset->px[idvertex2[i]]-pPointset->px[idvertex1[i]]);
			y31[i] = (ya-pPointset->py[idvertex1[i]]);
			y21[i] = (pPointset->py[idvertex2[i]]-pPointset->py[idvertex1[i]]);
			area[i]=y31[i]*x21[i]-y21[i]*x31[i];
		} //end of for()
		int i_smallest=0;
		double f_area_smallest=0.0;
		BOOL bIntersect[3];
		for(i=0; i<=2; i++)
		{
			
			bIntersect[i] = LineSegmentsIntersect(	pPointset->px[idvertex1[i]], 
													pPointset->py[idvertex1[i]], 
													pPointset->px[idvertex2[i]], 
													pPointset->py[idvertex2[i]], //line segment a
													xc, yc, xa, ya); //line segment b
			
			//pick smallest area (largest negative area)
			if(area[i]<0.)
			{
				if(area[i]<f_area_smallest)
				{
					i_smallest = i;
					f_area_smallest = area[i];
				}
			}
		}
		i=i_smallest;
		//pick smallest area triangle that is also intersecting imaginary axis (directional line) 
		f_area_smallest = 0.0;
		int i_smallest_and_intersecting = -1;
		for(i=0; i<=2; i++)
		{
			if(area[i]<f_area_smallest && bIntersect[i]==TRUE)
			{
				i_smallest_and_intersecting = i;
				f_area_smallest = area[i];
			}
		}
		if(i_smallest_and_intersecting<0) i = i_smallest;
		  else i=i_smallest_and_intersecting;
		/*
		#ifdef _DEBUG
				CString myString;
				myString.Format("area picked: %f (from %f %f %f) (%d %d %d)\r\n", area[i], area[0], area[1], area[2], bIntersect[0], bIntersect[1], bIntersect[2]);
				((COIIIApp*)AfxGetApp())->GetLogDocument()->AddText(myString);
		#endif //_DEBUG
		*/

		//ASSERT(area[i]==f_area_smallest);
		if (area[i]==0.)
		{
			//case on the vertex
			if( x31[i]==0. && y31[i]==0.)
			{
				p_itriseed[0] = newtr;
				return -2;
			}
		}
		else if (area[i] <0.)
		{
			if(pPointset->nt[i_adj[i]][newtr] == -1)
			{
				// store the last triangle found before going
				//   outside the convex frontier 
				p_itriseed[0] = newtr;
				return -1;
			}
			newtr = pPointset->nt[i_adj[i]][newtr];
			aux=1;
			//poirier, oct 2001, begin
			//collect all triangle on the search path
			itri++;
			p_numtrifound[0]++;
			p_arraytri[itri] = newtr; 
			//collect all vertex on the search path
			ivertex++;
			p_numvertexfound[0]++;
			for(int j=0; j<=2; j++)
			{
				//for each new triangle found, there is only one new vertex to add
				if(pPointset->vt[j][newtr] != prev_idvertex[0]
					&& pPointset->vt[j][newtr] != prev_idvertex[1]
					&& pPointset->vt[j][newtr] != prev_idvertex[2])
				{
					p_arrayvertex[ivertex] = pPointset->vt[j][newtr]; //add new vertex id
				}
			}
			prev_idvertex[0] = pPointset->vt[0][newtr];
			prev_idvertex[1] = pPointset->vt[1][newtr];
			prev_idvertex[2] = pPointset->vt[2][newtr];
			//poirier, oct 2001, end
			continue;
		}
		else
		{
			//all areas greater than zero
			//ASSERT(FALSE);
		}
    } while (aux ==1);
    p_itriseed[0] = newtr;
    return newtr;
}




//this version collects all triangle (_CAT) and vertex (_AV) involved in the (n)log(n) search path
//this version has also been modified so the search path stays along a straight line (seed triangle center - destination point)
//
//
//  xc,yc  .      .          .   .          .
//             .      .         .     .          .  xa,ya
//
//triangle vertices collected sequentially as new triangle is found and
//sorted between 2 vertex sets, depending upon which side of the line
//defined by (xc,yc) (xa,ya) a given vertex lies
int FindTriContainingPoint_CATAV(	POINTSET* pPointset,
									double xa,
									double ya,
									int* p_itriseed,
									int* p_numtrifound,
									int* p_arraytri,
									int* p_numvertexfound1,
									int* p_arrayvertex1,
									int* p_numvertexfound2,
									int* p_arrayvertex2)
{
    // validate input pointer 
	if(pPointset==NULL || p_itriseed==NULL 
		|| p_numtrifound==NULL || p_arraytri==NULL
		|| p_numvertexfound1==NULL || p_arrayvertex1==NULL
		|| p_numvertexfound2==NULL || p_arrayvertex2==NULL)
	{
		ASSERT(FALSE);
		return -99;
	}
    if(p_itriseed == NULL) 
	{
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindTriContainingPoint_CATAV(), p_itriseed is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
	}
	if(p_itriseed[0]>pPointset->ntri)
	{
		/* //spi 2014
		CLogDocument* pLogDoc = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		pLogDoc->AddText("warning, itriseed reseted to zero in FindTriContainingPoint_CATAV()\r\n");
		*/
		p_itriseed[0] = 0;
	}


    int i,aux,newtr;
	p_numtrifound[0] = 0;
    newtr=p_itriseed[0];
	//to collect all triangles on the search path
	int itri = 0;
	p_numtrifound[0]++;
	p_arraytri[itri] = newtr; //first triangle id

	//to force the search path to stay on the imaginary line (seed triangle center xc, yc - destination xa, ya)
	double xc = (pPointset->px[pPointset->vt[0][newtr]] + pPointset->px[pPointset->vt[1][newtr]] + pPointset->px[pPointset->vt[2][newtr]]) / 3.;
	double yc = (pPointset->py[pPointset->vt[0][newtr]] + pPointset->py[pPointset->vt[1][newtr]] + pPointset->py[pPointset->vt[2][newtr]]) / 3.;

	double m, b;
	if(LineMB(xc, yc, xa, ya, &m, &b)!=TRUE) //y = mx + b
	{
		ASSERT(FALSE);
	}

	//to collect all vertex on the search path
	int idvertex;
	p_numvertexfound1[0]=0;
	p_numvertexfound2[0]=0;
	int ivertex1 = 0; //index to first p_arrayvertex1[]
	int ivertex2 = 0; //index to second p_arrayvertex2[]
	int prev_idvertex[3]; //always store last triangle's vertex to avoid adding same vertex twice
	//for the first 3 vertices
	for(int j=0; j<=2; j++)
	{
		idvertex = pPointset->vt[j][newtr];
		prev_idvertex[j] = idvertex;
		if(IsPointUpLineMB(	pPointset->px[idvertex], //x
							pPointset->py[idvertex], //y
							&m, &b)) //lineMB, y = mx + b
		{
			p_numvertexfound1[0]++;
			p_arrayvertex1[ivertex1] = idvertex; 
			ivertex1++;
		}
		else
		{
			p_numvertexfound2[0]++;
			p_arrayvertex2[ivertex2] = idvertex; 
			ivertex2++;
		}
	}

    do
    {
		aux=0;
		double area[3],x21[3],x31[3],y21[3],y31[3];
		int i_adj[3],idvertex1[3],idvertex2[3];
		for (i=0; i<=2; i++)
		{
			i_adj[i]=jnd[i]; //jnd[] = {1,2,0} to pick adjacent index (counter-clockwise)
			idvertex1[i] = pPointset->vt[i][newtr];
			idvertex2[i] = pPointset->vt[i_adj[i]][newtr];
			x31[i] = (xa-pPointset->px[idvertex1[i]]);
			x21[i] = (pPointset->px[idvertex2[i]]-pPointset->px[idvertex1[i]]);
			y31[i] = (ya-pPointset->py[idvertex1[i]]);
			y21[i] = (pPointset->py[idvertex2[i]]-pPointset->py[idvertex1[i]]);
			area[i]=y31[i]*x21[i]-y21[i]*x31[i];
		} //end of for()
		int i_smallest=0;
		double f_area_smallest=0.0;
		BOOL bIntersect[3];
		for(i=0; i<=2; i++)
		{
			
			bIntersect[i] = LineSegmentsIntersect(	pPointset->px[idvertex1[i]], 
													pPointset->py[idvertex1[i]], 
													pPointset->px[idvertex2[i]], 
													pPointset->py[idvertex2[i]], //line segment a
													xc, yc, xa, ya); //line segment b
			
			//pick smallest area (largest negative area)
			if(area[i]<0.)
			{
				if(area[i]<f_area_smallest)
				{
					i_smallest = i;
					f_area_smallest = area[i];
				}
			}
		}
		i=i_smallest;
		//pick smallest area triangle that is also intersecting imaginary axis (directional line) 
		f_area_smallest = 0.0;
		int i_smallest_and_intersecting = -1;
		for(i=0; i<=2; i++)
		{
			if(area[i]<f_area_smallest && bIntersect[i]==TRUE)
			{
				i_smallest_and_intersecting = i;
				f_area_smallest = area[i];
			}
		}
		if(i_smallest_and_intersecting<0) i = i_smallest;
		  else i=i_smallest_and_intersecting;
		/*
		#ifdef _DEBUG
				CString myString;
				myString.Format("area picked: %f (from %f %f %f) (%d %d %d)\r\n", area[i], area[0], area[1], area[2], bIntersect[0], bIntersect[1], bIntersect[2]);
				((COIIIApp*)AfxGetApp())->GetLogDocument()->AddText(myString);
		#endif //_DEBUG
		*/

		//ASSERT(area[i]==f_area_smallest);
		if (area[i]==0.)
		{
			//case on the vertex
			if( x31[i]==0. && y31[i]==0.)
			{
				p_itriseed[0] = newtr;
				return -2;
			}
		}
		else if (area[i] <0.)
		{
			if(pPointset->nt[i_adj[i]][newtr] == -1)
			{
				// store the last triangle found before going
				//   outside the convex frontier 
				p_itriseed[0] = newtr;
				return -1;
			}
			newtr = pPointset->nt[i_adj[i]][newtr];
			aux=1;
			//poirier, oct 2001, begin
			//collect all triangle on the search path
			itri++;
			p_numtrifound[0]++;
			p_arraytri[itri] = newtr; 
			//collect all vertex on the search path
			for(int j=0; j<=2; j++)
			{
				//for each new triangle found, there is only one new vertex to add
				idvertex = pPointset->vt[j][newtr];
				if(idvertex != prev_idvertex[0]
					&& idvertex != prev_idvertex[1]
					&& idvertex != prev_idvertex[2])
				{
					if(IsPointUpLineMB(	pPointset->px[idvertex], //x
										pPointset->py[idvertex], //y
										&m, &b)) //lineMB, y = mx + b
					{
						p_numvertexfound1[0]++;
						p_arrayvertex1[ivertex1] = idvertex; 
						ivertex1++;
					}
					else
					{
						p_numvertexfound2[0]++;
						p_arrayvertex2[ivertex2] = idvertex; 
						ivertex2++;
					}

				}
			}
			prev_idvertex[0] = pPointset->vt[0][newtr];
			prev_idvertex[1] = pPointset->vt[1][newtr];
			prev_idvertex[2] = pPointset->vt[2][newtr];
			//poirier, oct 2001, end
			continue;
		}
		else
		{
			//all areas greater than zero
			//ASSERT(FALSE);
		}
    } while (aux ==1);
    p_itriseed[0] = newtr;
    return newtr;
}

///////////////////////////////////////////////////////////////////////////
// FindNearestNeighbor(double xa, double ya, int* p_itriseed)
//
//    Cherche le point voisin le plus pres de (xa,ya).
//
//
//    xa: coordonnee x du point d'interet.
//    ya: coordonnee y du point d'interet.
//    itriseed*: est l'adresse de l'index du triangle qui sera utilise pour
//		 debuter la recherche.	cette fonction retourne egalement
//		 l'index du dernier triangle trouve dans p_itriseed.
//
//    retourne:  l'index du point voisin.
//		 si error: -1 si (xa,ya) est a l'exterieur de la frontiere
//			      convexe qui contient tout les triangles.
//			   -2 si (xa,ya) correspond a un sommet existant.
//			   -99 si p_itriseed est NULL
///////////////////////////////////////////////////////////////////////////
int FindNearestNeighbor(POINTSET* pPointset,
			double xa,
			double ya,
			int* p_itriseed)
{
    int i,itrianglefound,index, inearest;
    double r2, r2min;

    // validate input pointer 
    if(p_itriseed == NULL) 
	{
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindNearestNeighbor(), p_itriseed is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
	}

    itrianglefound = FindTriContainingPoint(pPointset,xa,ya,p_itriseed);
    if( itrianglefound >= 0)
    {
		for(i=0; i<3; i++)
		{
		    index = pPointset->vt[i][itrianglefound];
		    r2 = (xa-pPointset->px[index])*(xa-pPointset->px[index])+(ya-pPointset->py[index])*(ya-pPointset->py[index]);
		    if(i==0 || r2<r2min)
		    {
				r2min = r2;				
				inearest = index;
		    }
		}
		p_itriseed[0] = itrianglefound;
		return inearest;
    }
    else if(itrianglefound == -2)
    {
		//
		// Le point (xa,ya) correspond a un sommet existant. 
		//

		/*
		ASSERT(FALSE); //we should identify which vertex the input point lies on
		return -2;
		*/
		int idtriseed = p_itriseed[0];
		for(i=0; i<3; i++)
		{
		    index = pPointset->vt[i][idtriseed];
		    if( (xa-pPointset->px[index])==0.0 && (ya-pPointset->py[index])==0.0 )
			{
				inearest = index;
				return inearest;
			}
		}
		ASSERT(FALSE); //we did not identify which vertex the input point lies on 
		return -2; //should not occur
    }
    else if(itrianglefound==-1)
    {
		// Erreur, le point d'entree doit etre a l'exterieur
		//   de la frontiere convexe. 
		return -1;
    }
	//development-time error, should not end up here
	return -1;
}


///////////////////////////////////////////////////////////////////
// A triangle is said to be invalid if :
//
// 1) has -1 for triangle index (this is the case for outside tri).
// 2) has its center outside the convexe envelope bounding rect.
//
// Usage:
//
// This function should be used before building voronoi region, in
// this case the vertex should not be surrounded by invalid tri.
//
// This function is indirectly used when getting the neighbor of
// any input data point.  To get proper neighbor, only valid tri
// should be used.
///////////////////////////////////////////////////////////////////
int InvalidTri( POINTSET* pPointset,
		int iadjtri)
{
    // case 1 
    if( iadjtri == -1 ) return TRUE;

	//2012august09, poirier, begin
	/*
    // case 2 
    if( pPointset->ctx[iadjtri] < pPointset->xmin ||
	pPointset->ctx[iadjtri] > pPointset->xmax ||
	pPointset->cty[iadjtri] < pPointset->ymin ||
	pPointset->cty[iadjtri] > pPointset->ymax ) 
	{
		return TRUE;
	}
	*/
	//2012august09, poirier, end
	return FALSE;
}



////////////////////////////////////////////////////////////////
// FindAllTriSurroundingVertex(POINTSET* pPointset,
//			       int  ivertex,
//			       int* p_itriseed,
//			       int* p_numtrifound,
//			       int* p_arraytri,
//			       int* p_arrayneighbor);
//
//
// -cette fonction permet de trouver les triangles entourant un
//  point donne.  par la suite, on pourra obtenir les centres
//  de ces triangles directement (en utilisant ct[indextriangle].
// -cette fonction retourne egalement, les plus proches voisins.
//
//  ivertex: est l'index au point (ou sommet) d'interet.
//  p_itriseed: est l'adresse de l'index du triangle qui sera
//		utilise pour debuter la recherche. cette fonction
//		retourne egalement l'index du dernier triangle
//		trouve dans p_itriseed.
//
//  p_numtrifound: est l'adresse de la variable int dans laquelle
//		   sera retournee le nombre de triangles trouves.
//
//  p_arraytri: est l'adresse du vecteur (array) dans lequel
//		seront retournes les index des triangles trouves.
//
//  p_arrayneighbor: est l'adresse du vecteur (array) dans lequel
//		     seront retournes les index des sommets voisins.
//
//  retourne: 1 (TRUE) si SUCCES
//	      0 (FALSE) si au moins un triangle non valide
//	     <0 si ERREUR
////////////////////////////////////////////////////////////////
int FindAllTriSurroundingVertex( POINTSET* pPointset,
				 int  ivertex,
				 int* p_itriseed,
				 int* p_numtrifound,
				 int* p_arraytri,
				 int* p_arrayneighbor )
{
    //char text[255];
    int i, itri, ifirstfoundtri, ifirstadjtri, iadjtri;
    int ineighbor;
	//int ifirstneighbor;

    if( p_itriseed == NULL || p_numtrifound == NULL
	|| p_arraytri == NULL || p_arrayneighbor == NULL)
    {
		// erreur, pointeur NULL 
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindAllTriSurroundingVertex(), p_itriseed or p_numtrifound or p_arraytri or p_arrayneighbor is NULL\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
    }


    ifirstfoundtri = FindTriContainingPoint( pPointset,
					     pPointset->px[ivertex],
					     pPointset->py[ivertex],
					     p_itriseed );
    //
    // -should get -2 because the input point is a vertex, and a vertex
    //	is never inside a triangle, it is a part of a triangle.	the
    //	only time a vertex "could" be inside a existing triangle is when
    //	we just inserted this new vertex and the triangulation needs
    //	update.
    // -the triangle that has the input point has one of its vertices
    //	should be in p_itriseed[0].
    //
    if(ifirstfoundtri != -2)
    {
		//mdlDialog_openAlert("Fatal Error, vertex found inside an existing triangle");
		//printf("Fatal Error, vertex found inside an existing triangle\n");
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindAllTriSurroundingVertex(), vertex found inside an existing triangle\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
    }
    ifirstfoundtri = p_itriseed[0];
    if(ifirstfoundtri == -1)
    {
		// this should pe impossible, first tri found cannot be -1 
		//mdlDialog_openAlert("Fatal Error, cannot return surrounding triangles for boundary vertices");
		//printf("Fatal Error, cannot return surrounding triangles for boundary vertices\n");
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("Error in FindAllTriSurroundingVertex(), cannot return surrounding triangles for boundary vertices\r\n");
		pLogDocument->AddText(myString);
		*/
		return -99;
    }
    if( InvalidTri(pPointset, ifirstfoundtri) )
    {
		//
		// if first found tri invalid, keep searching  until a valid tri
		// is found (counter-clockwise search)
		// assumption: there is always at least one valid tri for each
		//	       data point.
		//
		iadjtri = ifirstfoundtri;
		do
		{
		    i=0;
		    // find the index corresponding to ivertex 
		    while(i<3 && ivertex!=pPointset->vt[i][iadjtri]) i++;
		    // temporary error check 
		    if(i==3)
		    {
				//sprintf(text,"Fatal Error, ivertex %d not found in iadjtri %d", ivertex, iadjtri );
				//mdlDialog_openAlert(text);
				//printf("%s\n", text);
				/* //spi 2014
				CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
				CString myString;
				myString.Format("Error in FindAllTriSurroundingVertex(), ivertex %d not found in iadjtri %d\r\n", ivertex, iadjtri);
				pLogDocument->AddText(myString);
				*/
				return -99;
		    }
		    // finds the 2nd neighbors counter-clockwise 
		    ineighbor = pPointset->vt[ijk[i+2]][iadjtri];
		    iadjtri = GetAdjTri(pPointset,ivertex, ineighbor, iadjtri);

		    if( iadjtri==ifirstfoundtri )
		    {
				//mdlDialog_openAlert("Fatal Error, cannot find one valid tri for vertex (ccw search)");
				//printf("Fatal Error, cannot find one valid tri for vertex (ccw search)\n");
				/* //spi 2014
				CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
				CString myString;
				myString.Format("Error in FindAllTriSurroundingVertex(), cannot find one valid tri for vertex (ccw search)\r\n");
				pLogDocument->AddText(myString);
				*/
				return -99;
		    }
		    if( iadjtri == -1 ) break;
		} while( InvalidTri(pPointset, iadjtri) );

		if( iadjtri == -1 )
		{
		    //
		    // if the boundary was hit before identifying a first valid
		    // triangle in the ccw search, perform a clock-wise search.
		    //
		    iadjtri = ifirstfoundtri;
		    do
		    {
				i=0;
				// find the index corresponding to ivertex 
				while(i<3 && ivertex!=pPointset->vt[i][iadjtri]) i++;
				// temporary error check 
				if(i==3)
				{
					//sprintf(text,"Fatal Error, ivertex %d not found in iadjtri %d", ivertex, iadjtri );
					//mdlDialog_openAlert(text);
					//printf("%s\n", text);
					/* //spi 2014
					CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
					CString myString;
					myString.Format("Error in FindAllTriSurroundingVertex(), ivertex %d not found in iadjtri %d\r\n", ivertex, iadjtri);
					pLogDocument->AddText(myString);
					*/
					return -99;
				}
				// finds the 1st neighbors counter-clockwise 
				ineighbor = pPointset->vt[ijk[i+1]][iadjtri];
				// get the clockwise adjacent triangle 
				iadjtri = GetAdjTri(pPointset,ivertex, ineighbor, iadjtri);

				if( iadjtri==ifirstfoundtri )
				{
					//mdlDialog_openAlert("Fatal Error, cannot find one valid tri for vertex (ccw + cw search)");
					//printf("Fatal Error, cannot find one valid tri for vertex (ccw + cw search)\n");
					/* //spi 2014
#ifdef _DEBUG
					CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
					CString myString;
					myString.Format("Error in FindAllTriSurroundingVertex(), cannot find one valid tri for vertex (ccw + cw search)\r\n");
					pLogDocument->AddText(myString);
#endif
					*/
					return -99;
				}
				if( iadjtri == -1 ) break;
		    } while( InvalidTri(pPointset, iadjtri) );

		    // if we hit the boundary while doing the clockwise search 
		    if( iadjtri == -1 )
		    {
				//mdlDialog_openAlert("Fatal Error, cannot find one valid tri for vertex (ccw + cw search)");
				//printf("Fatal Error, cannot find one valid tri for vertex (ccw + cw search)\n");
				/* //spi 2014
#ifdef _DEBUG
				CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
				CString myString;
				myString.Format("Error in FindAllTriSurroundingVertex(), cannot find one valid tri for vertex (ccw + cw search)\r\n");
				pLogDocument->AddText(myString);
#endif// _DEBUG
				*/
				return -99;
		    }
		}

	//
	// now, ifirstfoundtri is garanted to be valid
	//
	ifirstfoundtri = iadjtri;
    }



    ///////////////////////////////////////////////////////////
    // now, find all the other triangles adjacent to ivertex. 
    // if a invalid triangle is found, set p_itriseed to the  
    // last valid triangle found in counter-clockwise order.  
    ///////////////////////////////////////////////////////////
    ifirstadjtri = ifirstfoundtri;
    iadjtri = ifirstfoundtri;
    itri = -1;
    do
    {
		itri++; // index for the output array 


		i=0;
		// find the index corresponding to ivertex 
		while(i<3 && ivertex!=pPointset->vt[i][iadjtri]) i++;
		// temporary error check 
		if(i==3)
		{
			//sprintf(text,"Fatal Error, ivertex %d not found in iadjtri %d", ivertex, iadjtri );
			//mdlDialog_openAlert(text);
			//printf("%s\n", text);
			/* //spi 2014
			CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
			CString myString;
			myString.Format("Error in FindAllTriSurroundingVertex(), ivertex %d not found in iadjtri %d\r\n", ivertex, iadjtri);
			pLogDocument->AddText(myString);
			*/
			return -99;
		}
		// finds the 2nd neighbors counter-clockwise 
		ineighbor = pPointset->vt[ijk[i+2]][iadjtri];
		// printf("found: ineighbor %d, iadjtri %d\n",ineighbor,iadjtri); 


		p_arrayneighbor[itri] = ineighbor;
		p_arraytri[itri] = iadjtri;

		// printf("searching next: ivertex %d, ineighbor %d, in tri %d", ivertex,ineighbor,iadjtri); 
		iadjtri = GetAdjTri(pPointset,ivertex, ineighbor, iadjtri);
		// printf(", iadjtri returned ->%d\n",iadjtri);

		if(iadjtri == -1 || InvalidTri(pPointset, iadjtri) )
		{
	    
		    //In searching for next counter-clockwise adjacent triangle we
		    //get an invalid triangle (index=-1 or tri center outside
		    //convexe envelope bounding rect).
		    //
		    //Set triseed to the last valid found triangle and return FALSE.
		    //Using the same input vertex (ivertex) and this returned triseed,
		    //one can find valid triangles and neighbors with function:
		    //FindAllValidTriAndNeighborSurroundingVertex().  This function
		    //will return, in the CLOCK-WISE order, the array of valid
		    //adjacent triangle as well as the array of proper neighbors.
		    //Warning, in this later case, the number of adjacent triangle
		    //will be generaly different than the number of neighbor found.
	    
		    p_itriseed[0] = p_arraytri[itri];
		    return FALSE;
		}
		if(iadjtri == -99)
		{
		    //sprintf(text, "Fatal Error, inconsistant vertices pair (%d,%d) for triangle %d", ivertex,ineighbor,p_arraytri[itri]);
		    //mdlDialog_openAlert(text);
			//printf("%s\n", text);
			/* //spi 2014
			CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
			CString myString;
			myString.Format("Error in FindAllTriSurroundingVertex(), inconsistant vertices pair (%d,%d) for triangle %d\r\n", ivertex,ineighbor,p_arraytri[itri]);
			pLogDocument->AddText(myString);
			*/
		    return FALSE;
		}
    } while(iadjtri != ifirstadjtri);

    p_numtrifound[0] = itri+1; // itri is an array index 

    return TRUE;
}


//
// ComputeAllVoronoiDensities(POINTSET* pPointset)
//  
// -the voronoi density is the inverse of the voronoi area,
//  we have identified this quantity to be very usefull in
//  the segmentation process.
//
// -we cannot compute voronoi density for vertex having one
//  or more of their adjacent triangles part of the boundary,
//  for those vertex we will set  voronoi density to -9999.99.
//
//
int ComputeVoronoiAreaForAllVertex(POINTSET* pPointset)
{
    for (int idvertex=0; idvertex<(pPointset->npts); idvertex++)
    {
		ComputeVoronoiAreaForVertex(pPointset, idvertex);
	}
	return TRUE;
}

int ComputeVoronoiAreaForVertex(POINTSET* pPointset, int idvertex)
{                        
    int ivertex=idvertex;
	int j;
    int numtrifound, numneighborfound, itriseed;
    int p_arraytri[200];
    int p_arrayneighbor[200];    
    double fVertexX, fVertexY, fCenterTriX, fCenterTriY, fPrevCenterTriX, fPrevCenterTriY;       
    double fTriangleArea, fVoronoiArea, fVoronoiDensity;
    
	/*
    // for debugging 
    FILE* pFile;
    FILE* pFile2;  
    char pszFilename[255];
    char* pCharFound;
    strcpy(pszFilename, "voroarea.dat");
    if( (pFile = fopen(pszFilename, "w")) == NULL )
    {
		printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
		ASSERT(FALSE);
		exit(1);
    }           
    pCharFound = strchr(pszFilename, '.'); 
    if(pCharFound==NULL)    
    {
    	printf("Error, in ComputeVoronoiAreaForAllVertex()\n");
    }
    strcpy(pCharFound, ".gph");
    if( (pFile2 = fopen(pszFilename, "w")) == NULL )
    {
		printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
		ASSERT(FALSE);
		exit(1);
    }
	fprintf(pFile, "ivertex,\tvoroarea,\tvorodensity\n");	
	*/
    
    //the triangle seed must be an existing triangle index 
    itriseed = 0;
    //for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    //{
        
		if(FindAllValidTriSurroundingVertex(pPointset,
					      					ivertex,
					      					&itriseed,
					      					&numtrifound,
					      					p_arraytri,
					      					&numneighborfound,
					      					p_arrayneighbor ) == TRUE)
		{
			// 
			// this vertex is fully surrounded by neighbors, it is not a boundary vertex,
			// therefore, we can compute safely the voronoi area (and voronoi density)
			//
			
			// closing the polygon made out of the adjacent triangle centers, last triangle center will also be the first one   
			p_arraytri[numtrifound] = p_arraytri[0];
			                                       
			fVoronoiArea = 0.0; 
			fVertexX = pPointset->px[ivertex];
			fVertexY = pPointset->py[ivertex];
			
			fCenterTriX = pPointset->ctx[p_arraytri[numtrifound]];
			fCenterTriY = pPointset->cty[p_arraytri[numtrifound]];
			// in order to compute the area of a polygon, we decompose it in triangles and sum their areas 
						                                                                       
		    for(j=1; j<numtrifound+1; j++) // now visiting tri in counter-clockwise order 
		    // for(j=numtrifound-1; j>-1; j--) 
		    {          
		    	fPrevCenterTriX = fCenterTriX;
		    	fPrevCenterTriY = fCenterTriY;		    	
		    	fCenterTriX = pPointset->ctx[p_arraytri[j]]; 
		    	fCenterTriY = pPointset->cty[p_arraytri[j]];
		    	
		    	fTriangleArea = (fPrevCenterTriX*fVertexY + fPrevCenterTriY*fCenterTriX + fCenterTriY*fVertexX 
		    					- fVertexY*fCenterTriX - fPrevCenterTriY*fVertexX - fPrevCenterTriX*fCenterTriY);
		    	 
		    	//fVoronoiArea = fVoronoiArea + ((fPrevCenterTriX*fCenterTriY - fCenterTriX*fPrevCenterTriY)
		    	//							-  (fVertexX*fCenterTriY - fCenterTriX*fVertexY)
		    	//							+  (fVertexX*fPrevCenterTriY - fPrevCenterTriX*fCenterTriY));
		    								
			    if(fTriangleArea<0) fTriangleArea = -0.5 * fTriangleArea;
			      else fTriangleArea = 0.5 * fTriangleArea; 
			    fVoronoiArea = fVoronoiArea + fTriangleArea;  
		    } 
		    
		}
		else
		{   
			// cannot evaluate voronoi area, but set it to -9999.99 
			fVoronoiArea = POINTSET_TAGFORINVALIDVOROAREA; // -9999.99  
		}
	    // we store the voronoi area as the first statistic   
	    pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VOROAREA] = fVoronoiArea;
	    
	    // we store the voronoi density (1/voronoiarea) as the second statistic  
	    if(fVoronoiArea<0) fVoronoiDensity = POINTSET_TAGFORINVALIDVOROAREA; //was 0.0f // if was dummy value -9999.99, set density to 0.0 
	    // amplifying vorodensity, of factor 1000 so it can be fit in window by microstation, numbers become big enough 
	      else fVoronoiDensity = 1.0/fVoronoiArea; 
	    pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VORODENSITY] = fVoronoiDensity;
	    
		/*
		// for debugging only  
		fprintf(pFile, "%i,\t%f,\t%f\n", ivertex, fVoronoiArea, fVoronoiDensity);	
		fprintf(pFile2, "%i\t%f\t%f\n", ivertex, fVoronoiArea, fVoronoiDensity);	
		*/
	//}  
	/*
	// for debugging 
	fclose(pFile);
	fclose(pFile2);
	*/
	return TRUE;
}


//
// ComputeNeighboringVoronoiAreaForAllVertex(POINTSET* pPointset)
//  
// -the neighboring voronoi area is the average voronoi area of
//  all surrounding neighbors.
//
// -neighboring voronoi area for vertex that have a voronoi area
//  not computable, will be set to the same value, -9999.99.
//
// -the neighbors having a dummy value for voronoi area, -9999.99,
//  will not be used in the average.
//
// -should be called after ComputeVoronoiAreaForAllVertex().
//
//
//Note:  poirier, dec 96, not used anymore, see ComputeLocalAverage() below
int ComputeNeighboringVoronoiDensityForAllVertex(POINTSET* pPointset)
{                        
    int ivertex, j, ivertexneighbor;
    int numtrifound, numneighborfound, itriseed, numvalidneighbor;
    int p_arraytri[200];
    int p_arrayneighbor[200];    
    double fNeighboringVoronoiDensity, fVoronoiDensity, fVoronoiArea;

    // for debugging 
    FILE* pFile;
    FILE* pFile2;  
    char pszFilename[255];
    char* pCharFound;
    strcpy(pszFilename, "voronei.dat");
    if( (pFile = fopen(pszFilename, "w")) == NULL )
    {
		printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
		ASSERT(FALSE);
		exit(1);
    }           
    pCharFound = strchr(pszFilename, '.'); 
    if(pCharFound==NULL)    
    {
    	printf("Error, in ComputeVoronoiAreaForAllVertex()\n");
    }
    strcpy(pCharFound, ".gph");
    if( (pFile2 = fopen(pszFilename, "w")) == NULL )
    {
		printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
		ASSERT(FALSE);
		exit(1);
    }
	fprintf(pFile, "ivertex,\tvoroarea,\tvorodensity\n");	
    
    // the triangle seed must be an existing triangle index 
    itriseed = 0;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
		// for debugging only  
		fprintf(pFile, "%i,  ", ivertex);
        
		if(FindAllValidTriSurroundingVertex(pPointset,
					      					ivertex,
					      					&itriseed,
					      					&numtrifound,
					      					p_arraytri,
					      					&numneighborfound,
					      					p_arrayneighbor ) == TRUE)
		{
			// 
			// this vertex is fully surrounded by neighbors, it is not a boundary vertex,
			// therefore, we can compute safely the neighboing voronoi density. 
			//
			// BUT do not forget to exclude the any neighbor having a dummy voronoi density
			//
			
			fNeighboringVoronoiDensity = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VORODENSITY]; /* also use itself */
			numvalidneighbor = numneighborfound + 1; // 1 for itself 
			
			// for debugging only  
			fprintf(pFile, "%f, ", fNeighboringVoronoiDensity);
				
		    for(j=0; j<numneighborfound; j++)
		    {   
		    	ivertexneighbor = p_arrayneighbor[j];
		    	fVoronoiArea = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VOROAREA];       
		    	fVoronoiDensity = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VORODENSITY];       
		    	if(fVoronoiArea<0.0)
		    	{
		    		numvalidneighbor--;
		    	}
		    	else
		    	{
		    		fNeighboringVoronoiDensity = fNeighboringVoronoiDensity + fVoronoiDensity;
					// for debugging only  
					fprintf(pFile, "%f, ", fVoronoiDensity);	
		    	}	
		    } 
		    if(numvalidneighbor==1)
		    {
		    	printf("error, numvalidneighbor==0 in ComputeNeighboringVoronoiDensityForAllVertex()\n");
				ASSERT(FALSE);
		    	exit(0);
		    }
		    else 
		    {
		    	fNeighboringVoronoiDensity = fNeighboringVoronoiDensity/numvalidneighbor;
		    }	
		    
		}
		else
		{   
			// voronoi area should be -9999.99, voronoi density 0.0 and neighboring voronoi density 0.0 
			fNeighboringVoronoiDensity = POINTSET_TAGFORINVALIDVOROAREA;  //was 0.0f
		}
	    // we store the neighboring voronoi density in the statistics   
		pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA] = fNeighboringVoronoiDensity;
		
		// for debugging only  
		fprintf(pFile, "\taverage: %f\n", fNeighboringVoronoiDensity);	
		fprintf(pFile2, "%i\t%f\n", ivertex, fNeighboringVoronoiDensity);	
	}     
	fclose(pFile);
	fclose(pFile2);
	return TRUE;
}



//  
// int ApplyAverageFilterToAllVertex(	POINTSET* pPointset, 
//										int iInputStatOffset, 
//										int iOutputStatOffset, 
//										int iNearestNeighborOrder)
//
// It is a finite responce spatial filtering of type average
//  
// -iInputStatOffset will determine the statistic to average, filter input.
//
// -iOutputStatOffset will determine where to store the averaged value, filter output.
//                                 
// -iNearestNeighborOrder will determine the number of layer of nearest neighbor to
//  use in the filtering process (i.e. 0: output will equal input, 1: average with the
//  nearest neighbors, 2: average with both the 1st and the 2nd nearest neighbors layers).
// 
// -WARNING: if iInputStatOffset correspond to voronoidensity, the neighbors having a dummy value 
//			 for voronoi area (-9999.99) will not be used in the average.
//
// -WARNING: if used with iNearestNeighborOrder greater than 1, this function should be called after 
//  		 calling ComputeVoronoiAreaForAllVertex() because ...
// 
// -WARNING: this function does not support iNearestNeighborOrder greater than one, as of April 20th 1995.

int ApplyAverageFilterToAllVertex(	POINTSET* pPointset, 
									int iInputStatOffset, 
									int iOutputStatOffset, 
									int iNearestNeighborOrder)
{                        
    int ivertex, j, ivertexneighbor;
    int numtrifound, numneighborfound, itriseed, numvalidneighbor;
    int p_arraytri[200];
    int p_arrayneighbor[200];    
    double fOutputValue, fInputValue, fVoronoiArea;

    // for debugging
    //FILE* pFile;
    //FILE* pFile2;  
    //char pszFilename[255];
    //char* pCharFound;
    //strcpy(pszFilename, "voronei.dat");
    //if( (pFile = fopen(pszFilename, "w")) == NULL )
    //{
	//	printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
	//	exit(1);
    //}           
    //pCharFound = strchr(pszFilename, '.'); 
    //if(pCharFound==NULL)    
    //{
    //	printf("Error, in ComputeVoronoiAreaForAllVertex()\n");
    //}
    //strcpy(pCharFound, ".gph");
    //if( (pFile2 = fopen(pszFilename, "w")) == NULL )
    //{
	//	printf("Error in ComputeVoronoiAreaForAllVertex(), cannot open file %s\n", pszFilename);
	//	exit(1);
    //}
	//fprintf(pFile, "ivertex,\tvoroarea,\tvorodensity\n");	
    //
    
    // validate  
    if(iInputStatOffset<0 || iInputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iInputStatOffset param in ApplyAverageFilterToAllVertex()");
		ASSERT(FALSE);
    	exit(0);
    }           
    if( iOutputStatOffset<0 || iOutputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iOutputStatOffset param in ApplyAverageFilterToAllVertex()");
		ASSERT(FALSE);
    	exit(0);
    }
    if(iOutputStatOffset==iInputStatOffset)
    {   
    	printf("error iOutputStatOffset==iInputStatOffset in ApplyAverageFilterToAllVertex()");
		ASSERT(FALSE);
    	exit(0);
    }
    
    // the triangle seed must be an existing triangle index
    itriseed = 0;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
		// for debugging only 
		//fprintf(pFile, "%i,  ", ivertex);
		//
        
        // output value is initialize to the vertex value 
		fOutputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iInputStatOffset]; //also use itself 
        
		if(FindAllValidTriSurroundingVertex(pPointset,
					      					ivertex,
					      					&itriseed,
					      					&numtrifound,
					      					p_arraytri,
					      					&numneighborfound,
					      					p_arrayneighbor ) == TRUE)
		{
			// 
			// this vertex is fully surrounded by neighbors, it is not a boundary vertex,
			// therefore, we can compute safely the neighboing voronoi density. 
			//
			// BUT do not forget to exclude the any neighbor having a dummy voronoi density
			//
			numvalidneighbor = numneighborfound + 1; // 1 for itself 
			
			// for debugging only 
			//fprintf(pFile, "%f, ", fNeighboringVoronoiDensity);
			//	
		    for(j=0; j<numneighborfound; j++)
		    {   
		    	ivertexneighbor = p_arrayneighbor[j];     
		    				
		    	fInputValue = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+iInputStatOffset];       
		    	if(iInputStatOffset==POINTSET_OFFSETSTAT_VORODENSITY || 
		    	   iInputStatOffset==POINTSET_OFFSETSTAT_VOROAREA  ||
				   iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
				   iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
		    	{   
		    		// if voronoi area invalid, do not sum it in the average 
		    		fVoronoiArea = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VOROAREA];       
			    	if(fVoronoiArea<0.0)
			    	{
			    		numvalidneighbor--;
			    	}
			    	else
			    	{
			    		fOutputValue = fOutputValue + fInputValue;
			    	}	
		    	
		    	}
		    	else
		    	{   
		    		// if not dealing with voronoi area or density, fInputValue is always valid 
		    		fOutputValue = fOutputValue + fInputValue;
		    	}
		    	  
		    } 
			//poirier, march 27 1997
			//let it go when numvalidneighbor = 1, it occurs with new data set
		    //if(numvalidneighbor<=1)
		    if(numvalidneighbor<1)
		    {   
		    	// development-time error, would occur if substracting too many neighbors 
		    	printf("error, numvalidneighbor==0 in ApplyAverageFilterToAllVertex()\n");
				ASSERT(FALSE);
		    	exit(0);
		    }
		    else 
		    {
		    	fOutputValue = fOutputValue/(double)numvalidneighbor;
		    }	
		    
		}
	    // we store the neighboring averaged value in the statistics   
		pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iOutputStatOffset] = fOutputValue;
		
		// for debugging only 
		//fprintf(pFile, "\taverage: %f\n", fNeighboringVoronoiDensity);	
		//fprintf(pFile2, "%i\t%f\n", ivertex, fNeighboringVoronoiDensity);	
		//
	}
	// for debugging only     
	//fclose(pFile);
	//fclose(pFile2);           
	//
	return TRUE;
}

int ComputeLocalAverage(POINTSET* pPointset,
						int iInputStatOffset,
						int iOutputStatOffset, 
						int iNearestNeighborOrder)
{
	return ApplyAverageFilterToAllVertex(pPointset, 
										 iInputStatOffset, 
										 iOutputStatOffset, 
										 iNearestNeighborOrder);
}

//WARNING, this function must be called after ComputeLocalAverage()
int ComputeLocalVariance(POINTSET* pPointset,
						 int iInputStatOffset,
						 int iInputAverageStatOffset,//stat offset of previously computed average (in relation with iInputStatOffset)
						 int iOutputStatOffset, //stat offset for variance
						 int iNearestNeighborOrder)
{
    int ivertex, j, ivertexneighbor;
    int numtrifound, numneighborfound, itriseed, numvalidneighbor;
    int p_arraytri[200];
    int p_arrayneighbor[200];    
    double fOutputValue, fInputValue, fVoronoiArea, fAverage, fDiff;

    
    // validate  
    if(iInputStatOffset<0 || iInputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iInputStatOffset param in ComputeLocalVariance()");
		ASSERT(FALSE);
    	exit(0);
    }           
    if(iInputAverageStatOffset<0 || iInputAverageStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iInputAverageStatOffset param in ComputeLocalVariance()");
		ASSERT(FALSE);
    	exit(0);
    }           
    if( iOutputStatOffset<0 || iOutputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iOutputStatOffset param in ComputeLocalVariance()");
		ASSERT(FALSE);
    	exit(0);
    }
    if(iOutputStatOffset==iInputStatOffset)
    {   
    	printf("error iOutputStatOffset==iInputStatOffset in ComputeLocalVariance()");
		ASSERT(FALSE);
    	exit(0);
    }
    
    // the triangle seed must be an existing triangle index
    itriseed = 0;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
		// for debugging only 
		//fprintf(pFile, "%i,  ", ivertex);
		//
        
        // output value is initialize to the vertex value 
		fOutputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iInputStatOffset]; //also use itself 
        fAverage = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iInputAverageStatOffset]; 
		fOutputValue = (fOutputValue - fAverage)*(fOutputValue - fAverage);
		if(FindAllValidTriSurroundingVertex(pPointset,
					      					ivertex,
					      					&itriseed,
					      					&numtrifound,
					      					p_arraytri,
					      					&numneighborfound,
					      					p_arrayneighbor ) == TRUE)
		{
			// 
			// this vertex is fully surrounded by neighbors, it is not a boundary vertex,
			// therefore, we can compute safely the neighboing voronoi density. 
			//
			// BUT do not forget to exclude the any neighbor having a dummy voronoi density
			//
			numvalidneighbor = numneighborfound + 1; // 1 for itself 
			
			// for debugging only 
			//fprintf(pFile, "%f, ", fNeighboringVoronoiDensity);
			//	
		    for(j=0; j<numneighborfound; j++)
		    {   
		    	ivertexneighbor = p_arrayneighbor[j];     		    				
		    	fInputValue = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+iInputStatOffset];       
				fDiff = fAverage-fInputValue;
		    	if(iInputStatOffset==POINTSET_OFFSETSTAT_VORODENSITY || 
		    	   iInputStatOffset==POINTSET_OFFSETSTAT_VOROAREA )
		    	{   
		    		// if voronoi area invalid, do not sum it in the average 
		    		fVoronoiArea = pPointset->pfStatistics[ivertexneighbor*(pPointset->nStatPerPoint)+POINTSET_OFFSETSTAT_VOROAREA];       
			    	if(fVoronoiArea<0.0)
			    	{
			    		numvalidneighbor--;
			    	}
			    	else
			    	{
			    		fOutputValue += fDiff*fDiff;
			    	}	
		    	
		    	}
		    	else
		    	{   
		    		// if not dealing with voronoi area or density, fInputValue is always valid 
		    		fOutputValue += fDiff*fDiff; 
		    	}
		    	  
		    } 
			//poirier, march 27 1997
			//let it go when numvalidneighbor = 1, it occurs with new data set
		    //if(numvalidneighbor<=1)
		    if(numvalidneighbor<1)
		    {   
		    	// development-time error, would occur if substracting too many neighbors 
		    	printf("error, numvalidneighbor==0 in ApplyAverageFilterToAllVertex()\n");
				ASSERT(FALSE);
		    	exit(0);
		    }
		    else 
		    {
		    	fOutputValue = fOutputValue/((double)numvalidneighbor);	//we might underestimate the variance, but OK for now
		    }			    
		}
		// we store the computed value in the statistics   
		fOutputValue = sqrt(fOutputValue);
		pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iOutputStatOffset] = fOutputValue;		
	}
	return TRUE;
}

int InitializeClassToZero(	POINTSET* pPointset,
							int iStatOffset)
{
	for(int ivertex=0; ivertex<pPointset->npts; ivertex++)
	{
		pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iStatOffset] = 0.0;		
	}
	return TRUE;
}

//finds the minimum and the maximum value of the statistic channel specified by iStatOffset,
//it stores the result in the pGlobalStatisticsInfo[iStatOffset] structure
int FindGlobalMinMax(POINTSET* pPointset, int iStatOffset)
{
    int ivertex;
    double fInputValue, fMin, fMax;

	ASSERT(iStatOffset<POINTSET_MAX_TOTALNUMBEROFSTAT);

    // validate  
    if(iStatOffset<0 || iStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iStatOffset param in FindGlobalMinMax()");
    	//exit(0);
		ASSERT(FALSE);
		return FALSE;
    }           

	//find minimum and maximum of all input channel values 
	fMin = MAXDBL;
	fMax = MINDBL;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
        // get input value 
		fInputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iStatOffset]; //also use itself 
		//exclude negative voronoi area values
		if(!((iStatOffset==POINTSET_OFFSETSTAT_VOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_VORODENSITY ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
			  && fInputValue<0.0f))
		{
			// check if smaller than min or larger than max
			if(fInputValue<fMin) fMin = fInputValue;
			if(fInputValue>fMax) fMax = fInputValue;
		}
	}
	pPointset->pGlobalStatisticsInfo[iStatOffset].fMin = fMin;
	pPointset->pGlobalStatisticsInfo[iStatOffset].fMax = fMax;
	return TRUE;
}

int ComputeGlobalAverage(POINTSET* pPointset, 
						 int iStatOffset)
{
    int ivertex;
    double fInputValue, fAverage;

	ASSERT(iStatOffset<POINTSET_MAX_TOTALNUMBEROFSTAT);

    // validate  
    if(iStatOffset<0 || iStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iStatOffset param in ComputeGlobalAverage()");
    	//exit(0);
		ASSERT(FALSE);
		return FALSE;
    }           

	//compute sum of all input channel values 
	fAverage = 0.0;
	int ii=0;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
        // get input value 
		fInputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iStatOffset]; //also use itself 
		//exclude negative voronoi area values
		if(!((iStatOffset==POINTSET_OFFSETSTAT_VOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_VORODENSITY ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
			  && fInputValue<0.0f))
		{
			//sum
			fAverage += fInputValue;
			ii++;
		}
	}
	pPointset->pGlobalStatisticsInfo[iStatOffset].fAverage = fAverage/((double)ii);
	return TRUE;
}

//average MUST be computed prior to call this function
int ComputeGlobalVariance(POINTSET* pPointset, 
						  int iStatOffset)
{
    int ivertex;
    double fInputValue, fAverage, fVariance, fDiff;

	ASSERT(iStatOffset<POINTSET_MAX_TOTALNUMBEROFSTAT);

    // validate  
    if(iStatOffset<0 || iStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iStatOffset param in ComputeGlobalAverage()");
    	//exit(0);
		ASSERT(FALSE);
		return FALSE;
    }           

	//compute sum of all input channel values 
	fAverage = pPointset->pGlobalStatisticsInfo[iStatOffset].fAverage;
	fVariance = 0.0;
	int ii=0;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
        // get input value 
		fInputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iStatOffset]; //also use itself 
		if(!((iStatOffset==POINTSET_OFFSETSTAT_VOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_VORODENSITY ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
			  iStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
			  && fInputValue<0.0f))
		{
			//diff
			fDiff = fInputValue - fAverage;
			//sum squared diff
			fVariance += fDiff*fDiff;
			ii++;
		}
	}
	pPointset->pGlobalStatisticsInfo[iStatOffset].fVariance = sqrt(fVariance/((double)ii));
	return TRUE;
}

int UpdateGlobalStatisticsInfo(POINTSET* pPointset, 
							   int iModifiedStatOffset)
{
	FindGlobalMinMax(pPointset, iModifiedStatOffset);
	ComputeGlobalAverage(pPointset, iModifiedStatOffset);
	ComputeGlobalVariance(pPointset, iModifiedStatOffset);
	return TRUE;
}

//NormalizeAllVertex(	POINTSET* pPointset, 
//						int iInputStatOffset, 
//						int iOutputStatOffset, 
//						double fMaxOutputValue)
//
//this function perform a rescaling of a given statistics channel
//
//-it assumes input statistics range between 0 and a maximum value
//-output statistics will be rescaled to range between 0 and the 
// supplied fMaxOutputValue
//
//-WARNING, overwriting input statistics channel is allowed by setting
// iInputStatOffset equal to iOutputStatOffset.  of course,
// this is a destructive operation.
//
int NormalizeAllVertex(	POINTSET* pPointset, 
						int iInputStatOffset, 
						int iOutputStatOffset,
						double fMinOutputValue,
						double fMaxOutputValue)
{                        
    int ivertex;
    double fOutputValue, fInputValue, fNormalizationFactor, fNormalizationOffset;
    double fMaxInputValue, fMinInputValue;

    // validate  
    if(iInputStatOffset<0 || iInputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iInputStatOffset param in ApplyAverageFilterToAllVertex()");
		ASSERT(FALSE);
    	exit(0);
    }           
    if( iOutputStatOffset<0 || iOutputStatOffset>=pPointset->nStatPerPoint)
    {   
    	printf("error with iOutputStatOffset param in ApplyAverageFilterToAllVertex()");
 		ASSERT(FALSE);
   		exit(0);
    }

	//find input channel maximum and minimum value 
	fMaxInputValue = MINDBL;
	fMinInputValue = MAXDBL;
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
        // get input value 
		fInputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iInputStatOffset]; //also use itself 
		//exclude negative voronoi area values
		if(!((iInputStatOffset==POINTSET_OFFSETSTAT_VOROAREA ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_VORODENSITY ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
			  && fInputValue<0.0f))
		{
			if(fInputValue>fMaxInputValue) fMaxInputValue = fInputValue;
			if(fInputValue<fMinInputValue) fMinInputValue = fInputValue;
		}
	}
	pPointset->pGlobalStatisticsInfo[iInputStatOffset].fMin = fMinInputValue;
	pPointset->pGlobalStatisticsInfo[iInputStatOffset].fMax = fMaxInputValue;

	//compute normalization offset
	fNormalizationOffset = fMinOutputValue - fMinInputValue;
	//compute normalization factor
	fNormalizationFactor = (fMaxOutputValue-fMinOutputValue)/(fMaxInputValue-fMinInputValue);

	//normalize input channel, store result in output channel
    for (ivertex=0; ivertex<(pPointset->npts); ivertex++)
    {
        //get input value 
		fInputValue = pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iInputStatOffset]; //also use itself 
		//exclude negative voronoi area values
		if(!((iInputStatOffset==POINTSET_OFFSETSTAT_VOROAREA ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_VORODENSITY ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA ||
			  iInputStatOffset==POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA)
			  && fInputValue<0.0f))
		{
			//remap output value
			fOutputValue = (fInputValue - fMinInputValue) * fNormalizationFactor + fMinOutputValue;  
		}
		else
		{
			fOutputValue = fInputValue;
		}
		//set output value 
		pPointset->pfStatistics[ivertex*(pPointset->nStatPerPoint)+iOutputStatOffset] = fOutputValue;	
	}

	//update infostatistics structure for modified stat channel
	UpdateGlobalStatisticsInfo(pPointset, iOutputStatOffset);

	return TRUE;
}


int CheckNeighborConsistensyForAllVertex(POINTSET* pPointset)
{

    // the triangle seed must be an existing triangle index
    int itriseed = 0;
	int returnednumneighborfound;
	int p_returnedarrayneighbor[200];
    for (int ivertex=0; ivertex<(pPointset->npts); ivertex++)
	{
		FindAllConsistentNeighborSurroundingVertex( pPointset,
													ivertex,
													&itriseed,
													&returnednumneighborfound,
													p_returnedarrayneighbor );
	}
	return TRUE;
}

//poirier, march 27 1997, new function
//
//this function MUST be called after voronoi area are computed for all valid vertex
//
//based on a comparison of voronoi area, voronoi shape and relative position with
//the voronoi region between ivertex and its neighbors, 
//this function rejects the neighboring vertex that are not
//estimated to be part of the same neighborhood
int FindAllConsistentNeighborSurroundingVertex( POINTSET* pPointset,
												int  ivertex,
												int* p_itriseed,
												int* p_returnednumneighborfound,
												int* p_returnedarrayneighbor )
{
    //char text[255];
    int i, result; //itri, inei
    //int iccwmostvalidtri, iadjtri;
    //int ineighbor;
	//int ifirstadjtri, ifirstneighbor; 

	int numtrifound;
	int numneighborfound;
	int p_arraytri[200];
	int p_arrayneighbor[200];

    if( p_itriseed == NULL || p_arraytri == NULL || p_arrayneighbor == NULL)
    {
		// erreur, pointeur NULL 
		ASSERT(FALSE);
		return -99;
    }

	result = FindAllValidTriSurroundingVertex(	pPointset,
												ivertex,
												p_itriseed,
												&numtrifound,
												p_arraytri,
												&numneighborfound,
												p_arrayneighbor );
	ASSERT(numtrifound<200);

	//1) exclude neighbors that are much futher away from the others
	double dfRMin = MAXDBL;
	double dfRMax = MINDBL;
	double dfRPrevMax = MINDBL;
	double dfRPrevPrevMax = MINDBL;
	/* //poirier, dec 98
	int ivertexMin, ivertexMax;
	*/
	int ivertexMin = -1;
	int ivertexMax = -1;
	int ivertexPrevMax = -1;
	int ivertexPrevPrevMax = -1;
	double dfX, dfY, dfR;
	double dfRAverage = 0.0;
	BOOL bRemoveMax = FALSE;
	BOOL bRemovePrevMax = FALSE;
	BOOL bRemovePrevPrevMax = FALSE;
	p_returnednumneighborfound[0] = numneighborfound;
	if(numneighborfound>3)
	{
		for(i=0; i<numneighborfound; i++)
		{
			int ineivertex = p_arrayneighbor[i];
			dfX = pPointset->px[ineivertex]-pPointset->px[ivertex];
			dfY = pPointset->py[ineivertex]-pPointset->py[ivertex];
			dfR = sqrt(dfX*dfX+dfY*dfY);
			if(dfR<dfRMin) 
			{
				dfRMin = dfR;
				ivertexMin = ineivertex;
			}
			if(dfR>dfRMax) 
			{
				//prev prev max
				dfRPrevPrevMax = dfRPrevMax;
				ivertexPrevPrevMax = ivertexPrevMax;
				//prev max
				dfRPrevMax = dfRMax;
				ivertexPrevMax = ivertexMax;
				//new max
				dfRMax = dfR;
				ivertexMax = ineivertex;
			}
			dfRAverage += dfR;

		}
		dfRAverage = dfRAverage/(double)numneighborfound;
		//check if max should be removed
		if(dfRMax>2.0*dfRAverage)
		{
			bRemoveMax = TRUE;
			//check if prev max should be removed
			dfRAverage = (dfRAverage*(double)numneighborfound - dfRMax)/(double)(numneighborfound-1);
			if(dfRPrevMax>2.0*dfRAverage)
			{
				bRemovePrevMax = TRUE;
				//check if prev prev max should be removed
				dfRAverage = (dfRAverage*(double)(numneighborfound-1) - dfRPrevMax)/(double)(numneighborfound-2);
				if(dfRPrevPrevMax>2.0*dfRAverage)
				{
					bRemovePrevPrevMax = TRUE;
				}
			}
		}
		//remove vertex and create new vertex list
		int ii=-1;
		for(i=0; i<numneighborfound; i++)
		{
			int ineivertex = p_arrayneighbor[i];
			if( (bRemoveMax && ineivertex==ivertexMax)
				|| (bRemovePrevMax && ineivertex==ivertexPrevMax)
				|| (bRemovePrevPrevMax && ineivertex==ivertexPrevPrevMax))
			{
				//exclude
				p_returnednumneighborfound[0]--;
			}
			else
			{
				//include
				ii++;
				p_returnedarrayneighbor[ii] = ineivertex;
			}
		}
		ASSERT((ii+1)==p_returnednumneighborfound[0]);
	}
#ifdef _DEBUG	
	if(bRemoveMax || bRemovePrevMax || bRemovePrevPrevMax)
	{
		int iNumRemoved = bRemoveMax + bRemovePrevMax + bRemovePrevPrevMax;
		/* //spi 2014
		CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
		CString myString;
		myString.Format("%d neighbor(s) removed for vertex %d\r\n",iNumRemoved, ivertex);
		pLogDocument->AddText(myString);
		*/
	}
#endif //_DEBUG
	return TRUE;
}

////////////////////////////////////////////////////////////////
// FindAllValidTriSurroundingVertex(POINTSET* pPointset,
//				    				int	 ivertex,
//				    				int* p_itriseed,
//				    				int* p_numtrifound,
//				    				int* p_arraytri,
//				    				int* p_numneighborfound,
//				    				int* p_arrayneighbor);
//
//
// -cette fonction permet de ne trouver que les triangles valides
//  entourant un point donne.
// -cette fonction retourne egalement les voisins correspondant aux
//  triangles valides.
// -contrairement a la fonction FindAllTriSurroundingVertex(), cette
//  fonction retrouve les triangles et voisins dans le SENS HORAIRE.
//  Le premier triangle trouve sera le dernier triangle valide identifie
//  par recherche effectuee en sens anti-horaire.  Cette fonction pour-
//  suivra une recherche en sens horaire jusqu'a ce qu'elle tombe sur
//  un triangle invalide.
// -chaque point (vertex) possede au moins un triangle valide, donc au
//  minimum deux voisins.
//
//  WARNING: Cette fonction peut etre utilisee pour construire les
//	     regions (ou polygones) voronoi seulement si elle retourne
//	     TRUE (return 1) ce qui garantie que les triangles et voisins
//	     retrouves entourent completement le point d'interet (ivertex).
//	     Si la fonction retourne FALSE (return 0) la region voronoi
//	     ne peut etre construite, mais la liste de voisins est valide.
//
//
//
//  ivertex: est l'index au point (ou sommet) d'interet.
//
//  p_itriseed: est l'adresse de l'index du triangle qui sera
//		utilise pour debuter la recherche. cette fonction
//		retourne egalement l'index du dernier triangle
//		trouve dans p_itriseed.
//
//  p_numtrifound: est l'adresse de la variable int dans laquelle
//		   sera retournee le nombre de triangles trouves.
//
//  p_arraytri: est l'adresse du vecteur (array) dans lequel
//		seront retournes les index des triangles trouves.
//
//  p_numneighborfound: est l'adresse de la variable int dans laquelle
//			sera retournee le nombre de voisins trouves.
//
//  p_arrayneighbor: est l'adresse du vecteur (array) dans lequel
//		     seront retournes les index des sommets voisins.
//
//  retourne: 1 (TRUE) si SUCCES
//	      0 (FALSE) si tri n'entourent pas completement ivertex
//	     <0 si ERREUR
////////////////////////////////////////////////////////////////
int FindAllValidTriSurroundingVertex( POINTSET* pPointset,
				      int  ivertex,
				      int* p_itriseed,
				      int* p_numtrifound,
				      int* p_arraytri,
				      int* p_numneighborfound,
				      int* p_arrayneighbor )
{
    char text[255];
    int i, itri, inei, result;
    int iccwmostvalidtri, iadjtri;
    int ineighbor;
	//int ifirstadjtri, ifirstneighbor; 

    if( p_itriseed == NULL || p_numtrifound == NULL
	|| p_arraytri == NULL || p_numneighborfound == NULL
	|| p_arrayneighbor == NULL)
    {
		// erreur, pointeur NULL 
		ASSERT(FALSE);
		return -99;
    }

    result = FindAllTriSurroundingVertex( pPointset,
					  ivertex,
					  p_itriseed,
					  p_numtrifound,
					  p_arraytri,
					  p_arrayneighbor );
    if( result == TRUE)
    {
		// SUCCESS, vertex is fully surrounded by valid tri 
		p_numneighborfound[0] = p_numtrifound[0];
		return TRUE;
    }
    else if( result == FALSE )
    {
		// WARNING, some triangle not valid.  Voronoi region cannot
		//	    be constructed but neighbor list is valid.
		//
		// Since the function FindAllTriSurroundingVertex sets
		// p_itriseed[0] to the last valid tri in the ccw search,
		// all we have to do here is a clockwise search around vertex
		// starting with p_itriseed[0] tri.
		//
		iccwmostvalidtri = p_itriseed[0];
		if( InvalidTri(pPointset, iccwmostvalidtri) )
		{
		    //mdlDialog_openAlert("Fatal Error, no valid tri to start the cw search");
			printf("Fatal Error, no valid tri to start the cw search\n");
			ASSERT(FALSE);
		    return -99;
		}
	
		iadjtri = iccwmostvalidtri;
		itri = -1; // index for the output triangle array 
		inei = -1; // index for the output neighbor array 
	
		i=0;
		// find the index corresponding to ivertex 
		while(i<3 && ivertex!=pPointset->vt[i][iadjtri]) i++;
		// temporary error check 
		if(i==3)
		{
		    sprintf(text,"Fatal Error, ivertex %d not found in iadjtri %d", ivertex, iadjtri );
		    //mdlDialog_openAlert(text);
			printf("%s\n", text);
			ASSERT(FALSE);
		    return -99;
		}
		// finds the 1st neighbor
		ineighbor = pPointset->vt[ijk[i+2]][iadjtri];
		inei++;
		p_arrayneighbor[inei] = ineighbor;
	
		do
		{
		    itri++; // index for the output triangle array 
		    inei++; // index for the output neighbor array 
	
		    i=0;
		    // find the index corresponding to ivertex 
		    while(i<3 && ivertex!=pPointset->vt[i][iadjtri]) i++;
		    // temporary error check 
		    if(i==3)
		    {
				sprintf(text,"Fatal Error, ivertex %d not found in iadjtri %d", ivertex, iadjtri );
				//mdlDialog_openAlert(text);
				printf("%s\n", text);
				ASSERT(FALSE);
				return -99;
		    }
		    // finds the 1st neighbors counter-clockwise 
		    ineighbor = pPointset->vt[ijk[i+1]][iadjtri];
	
		    p_arrayneighbor[inei] = ineighbor;
		    p_arraytri[itri] = iadjtri;
	
		    // get the clockwise adjacent triangle 
		    iadjtri = GetAdjTri(pPointset,ivertex, ineighbor, iadjtri);
		    if( iadjtri==iccwmostvalidtri )
		    {
				//mdlDialog_openAlert("Fatal Error, inconsistency, should have found an invalid tri");
				printf("Fatal Error, inconsistency, should have found an invalid tri\n");
				ASSERT(FALSE);
				return -99;
		    }
		} while( !InvalidTri(pPointset, iadjtri) );
	
		p_numtrifound[0] = itri+1; // itri is a triangle array index 
		p_numneighborfound[0] = inei+1; // inei is a neighbor array index 
		return FALSE;
    }
    else
    {
		/*
		// fatal error
		ASSERT(FALSE);
		*/
		/* //spi 2014
		#ifdef _DEBUG
			CString myText;
			myText.Format("Warning, assert(false) in pointset.cpp, FindAllValidTriSurroundingVertex() calling FindAllTriSurroundingVertex() that returns %d\r\n", result);
			CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
			pLogDocument->AddText(myText);
		#endif
			*/
		return -99;
    }
}




////////////////////////////////////////////////////////////////
// FindAllNeighborSurroundingTri( POINTSET* pPointset,
//				  int  itri,
//				  int  iorder,
//				  int* p_numvertexfound,
//				  int* p_arrayneighbor );
//
//
// -Cette fonction permet de trouver tous les points voisins entourant
//  un triangle donne.	En specifiant l'ordre 0, 1 , 2 ou etc. on
//  retrouve soit les sommets du triangle de depart (ordre 0), les
//  sommets obtenu a l'odre 0 plus ceux formant tous les triangles
//  entourant le triangle de depart (ordre 1), les sommets obtenus a
//  l'ordre 0 et 1 ainsi que tous les sommets entourant les triangles
//  retrouve a l'ordre 1 (ordre 2).
//
// -Cette fonction est utilisee lors du calcul du vol des aires voronoi
//  dans le cas ou un nouveau point est ajoute.  Pour obtenir le vol
//  des aires voronoi lors de l'insertion d'un nouveau point, il faut
//  identifier les sous-ensemble de point a l'interieur duquel la
//  triangulation devra etre reconstruite.  Par exemple, si on insere
//  un point (x,y):
//
//  1) on trouve le triangle, itri, contenant ce point (x,y).
//  2) on appelle cette fonction en utilisant iorder = 2.
//  3) ensuite avec la liste des sommets obtenue, une triangulation locale
//     peut etre reconstruite.
//  4) on peut ainsi comparer les regions voronoi avant et apres insertion
//     du point.
//
//
//  itri: est l'index au triangle d'interet.
//  iorder: est l'ordre d'adjacence des triangles:
//		- 0 pour les sommets du triangle itri seulement.
//		- 1 pour les sommets plus-proche voisins de chacun
//		  des 3 sommets de itri.
//		- 2 pour les sommets plus-proche voisins de chacun
//		  des n sommets trouves a l'ordre 1.
//
//  p_numvertexfound: est le nombre de sommets trouves.
//
//  p_arrayneighbor: est l'adresse du vecteur (array) dans lequel
//		     seront retournes les index des sommets trouves.
//
//  retourne: 1 (TRUE) si SUCCES
//	      0 (FALSE) si erreur
////////////////////////////////////////////////////////////////
int FindAllNeighborSurroundingTri( POINTSET* pPointset,
				   int	itri,
				   int	iorder,
				   int* p_final_numfound,
				   int* final_arrayneighbor)
{
    int i,itemp,j,k,ivertex,v1,ii;
    int itriseed, istart, iend;
    int temp_arraytri[200];
    int temp_arrayneighbor[100];
    int temp_numfound;

    if( itri < 0 || itri>pPointset->ntri || iorder<0 || iorder>2
	|| p_final_numfound == NULL || final_arrayneighbor == NULL)
    {
		// erreur 
		printf(" erreur validation\n");
		ASSERT(FALSE);
		return FALSE;
    }
    /////////////////////////////////////////////////////////
    // now, find all the other triangles adjacent to itri.
    // that is:	for each one of itri's vertex, find their
    // surrounding neighbors using function,
    //		     FindAllTriSurroundingVertex()
    /////////////////////////////////////////////////////////

    // first, find vertex for order 0 
    p_final_numfound[0] = 0;
    for (ivertex=0; ivertex<3; ivertex++)
    {
		final_arrayneighbor[ivertex] = pPointset->vt[ivertex][itri];
		p_final_numfound[0]++;
    }
    if(iorder == 0) return TRUE;


    // second, find vertex for order 1 and then 2 
    istart = 0;
    iend = p_final_numfound[0]; // should be 3 
    for(ii= 0; ii<2; ii++)
    {
		for (i=istart; i<iend; i++)
		{
			itriseed = itri;
			ivertex = final_arrayneighbor[i];
			temp_numfound = 0;
			if( FindAllTriSurroundingVertex( pPointset,
							 ivertex,
							 &itriseed,
							 &temp_numfound,
							 temp_arraytri,
							 temp_arrayneighbor ) )
			{
				//
				// -if success, add tri and vertex in the final arrays
				// -do not duplicate tri or vertex
				//
				k = p_final_numfound[0];
				for (itemp=0; itemp<temp_numfound; itemp++)
				{
					v1 = temp_arrayneighbor[itemp];
					j = 0;
					while(j<p_final_numfound[0] && final_arrayneighbor[j]!=v1) j++;
					if ( j == p_final_numfound[0] )
					{
						// printf("%d, v=%d\n",k,v1); 
						final_arrayneighbor[k] = v1;
						k++;
					}
				}
				p_final_numfound[0] = k;
				printf("itri: %d, ivertex: %d, numfound: %d\n",itri,ivertex,k);
			}
			else
			{
				//
				//cannot find ii+1 order neighbor
				//printf("ivertex: %d, erreur triangle exterieur trouve\n", ivertex);
				//
				printf("<<ERROR>> itri: %d, ivertex: %d\n",itri,ivertex);
				p_final_numfound[0] = 0;
				return FALSE;
			}
		}
		if(iorder == 1) return TRUE;
		istart = 3;
		iend = p_final_numfound[0];
    }
    return 1;
}

//similar to FindAllNeighborSurroundingTri, but uses a vertex as input
//
//this function collects the neighboring vertices
//the vertices are ordered in a spiral like trace around input vertex
//the input iOrder parameter specifies the size of the neighborhood
//iOrder=0, final list is the input vertex alone
//iOrder=1, final list is the input vertex plus the nearest neighbors (if no edge found)
//iOrder=2, final list is 
int FindAllNeighborSurroundingVertex(	POINTSET* pPointset,
										int	iVertex,
										int	iOrder,
										int* p_iTriSeed,
										int* p_final_numfound,
										int* final_arrayneighbor)
{
    int i,itemp,j,k,v1,ii, ivertex;
    int itriseed, istart, iend;
    int temp_arraytri[200];
    int temp_arrayneighbor[100];
    int temp_numfound;

    if( iVertex < 0 || iVertex>pPointset->npts || iOrder<0 || iOrder>POINTSET_MAXIORDER
	|| p_final_numfound==NULL || final_arrayneighbor==NULL || p_iTriSeed==NULL)
    {
		// erreur 
		printf(" erreur validation\n");
		ASSERT(FALSE);
		p_final_numfound[0] = 0;
		return FALSE;
    }

    // first, store input vertex for order 0 
    p_final_numfound[0] = 0;
	final_arrayneighbor[0] = iVertex;
	p_final_numfound[0]++;
    if(iOrder == 0) return TRUE;


    // second, find vertices for order 1 to iorder
	//
	//important, collect vertices in expanding spiral like trace
	//			 discontinuities in the trace will indicate candidate for exclusion from neighborhood
	itriseed = p_iTriSeed[0];
    istart = 0;
    iend = p_final_numfound[0]; // should be 1 
    for(ii= 0; ii<iOrder; ii++)  
    {
		//for each of the previous order vertices
		for (i=istart; i<iend; i++)	
		{
			ivertex = final_arrayneighbor[i];
			temp_numfound = 0;
			if( FindAllTriSurroundingVertex( pPointset,
							 ivertex,
							 &itriseed,
							 &temp_numfound,
							 temp_arraytri,
							 temp_arrayneighbor ) )
			{
				//
				// -if success, add vertex in the final arrays
				// -do not duplicate vertex
				//
				k = p_final_numfound[0]; //k is the id of the next vertex to be inserted
				int ivertexLastInserted = final_arrayneighbor[k-1];
				/*
				for (itemp=0; itemp<temp_numfound; itemp++)
				{				
					//for each new vertex found
					v1 = temp_arrayneighbor[itemp];
					//assure vertex not already found in the previous orders
					j = 0;
					while(j<p_final_numfound[0] && final_arrayneighbor[j]!=v1) j++;
					//if new vertex not found, add it to the final list
					if (j==p_final_numfound[0] )
					{
						// printf("%d, v=%d\n",k,v1); 
						final_arrayneighbor[k] = v1;
						k++;
					}
				}
				*/
				//if iOrder>1, find index of the last inserted vertex in the new list, start adding if not already in the previous list
				//else iOrder==1, add all vertices
				if(iOrder>1 && ii!=0) //ii=0 is the first order
				{
					//look in the new list of vertices, find index of the last vertex inserted in the previous iorder
					j = 0;
					while(j<temp_numfound && temp_arrayneighbor[j]!=ivertexLastInserted) j++;
					if (j==temp_numfound )
					{
						//if last inserted vertex not found
						ASSERT(FALSE);
						p_final_numfound[0] = 0;
						return FALSE;
					}
					//starting at the next vertex position, add all vertices not already into the final_arrayneighbor
					int iitemp = j+1; //starting at the next vertex position
					for (itemp=0; itemp<temp_numfound; itemp++)
					{				
						if(iitemp==temp_numfound) iitemp=0; //cyclic rotation
						//for each new vertex found
						v1 = temp_arrayneighbor[iitemp];
						//assure vertex not already found in the previous orders
						j = 0;
						while(j<p_final_numfound[0] && final_arrayneighbor[j]!=v1) j++;
						//if new vertex not found
						if (j==p_final_numfound[0] )
						{
							//add vertex to the final list
							final_arrayneighbor[k] = v1;
							k++;
						}
						else
						{
							//if vertex found, stop adding (all the other vertices should also be in the list)
							break; //go out of this for(itemp=0; itemp<temp_numfound; itemp++) loop
						}

						iitemp++;
					}
				}
				else
				{
					ASSERT(iOrder==1 || (iOrder>1 && ii==0) );
					//add all vertices
					for(itemp=0; itemp<temp_numfound; itemp++)
					{
						final_arrayneighbor[k] = temp_arrayneighbor[itemp];
						k++;
					}
				}
				p_final_numfound[0] = k;
				/*
				#ifdef _DEBUG
					CString myText;
					myText.Format("ivertex: %d, numfound: %d\r\n",ivertex,k);
					CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
					pLogDocument->AddText(myText);
				#endif
				*/
			}
			else
			{
				//
				//cannot find ii+1 order neighbor
				//printf("ivertex: %d, erreur triangle exterieur trouve\n", ivertex);
				//
				/* //spi 2014
				#ifdef _DEBUG
					CString myText;
					myText.Format("Warning, external triangle found for ivertex: %d\r\n",ivertex);
					CLogDocument* pLogDocument = ((COIIIApp*)AfxGetApp())->GetLogDocument();
					pLogDocument->AddText(myText);
				#endif
					*/
				//-the border has been hit
				//-stop adding vertices to the final list now
				//-return now
				//ASSERT(FALSE);
				p_final_numfound[0] = 0;
				return FALSE;
			}
		}
		istart = iend;
		iend = p_final_numfound[0];
    }
    return TRUE;
}



////////////////////////////////////////////////////////////////
// GetAdjTri(int ivertex1, int ivertex2, int itri)
//
//
// -cette fonction permet de trouver un des 3 triangles adjacents au
//  au triangle specifie par l'index itri.  le triangle choisi
//  est celui qui est oppose au cote specifie par la paire de points
//  ivertex1 et ivertex2.
//
//  ivertex1: index au premier point (ou sommet) de la paire d'interet.
//  ivertex2: index au second point (ou sommet) de la paire d'interet.
//  itri: est l'index du triangle pour lequel on cherche le adjtri.
//
//  retourne: l'index du triangle adjacent si SUCCES.
//	      -1 si erreur, signifie que ivertex1 et ivertex2 sont
//		 directement situes sur la frontiere convexe.
//	      -99 si erreur,
////////////////////////////////////////////////////////////////
int GetAdjTri(POINTSET* pPointset,
	      int ivertex1,
	      int ivertex2,
	      int itri)
{
    if( (ivertex1==pPointset->vt[0][itri]) && (ivertex2==pPointset->vt[1][itri]) )
      return ((int) pPointset->nt[1][itri]);
    if( (ivertex1==pPointset->vt[1][itri]) && (ivertex2==pPointset->vt[0][itri]) )
      return ((int) pPointset->nt[1][itri]);
    if( (ivertex1==pPointset->vt[0][itri]) && (ivertex2==pPointset->vt[2][itri]) )
      return ((int) pPointset->nt[0][itri]);
    if( (ivertex1==pPointset->vt[2][itri]) && (ivertex2==pPointset->vt[0][itri]) )
      return ((int) pPointset->nt[0][itri]);
    if( (ivertex1==pPointset->vt[1][itri]) && (ivertex2==pPointset->vt[2][itri]) )
      return ((int) pPointset->nt[2][itri]);
    if( (ivertex1==pPointset->vt[2][itri]) && (ivertex2==pPointset->vt[1][itri]) )
      return ((int) pPointset->nt[2][itri]);
    return -99;
}


int LineSegmentsIntersect(	double x1, double y1, double x2, double y2, //line segment a
							double x3, double y3, double x4, double y4) //line segment b
{
	//Line a
	//P1 (x1,y1)
	//P2 (x2,y2)
	//
	//Line b
	//P3 (x3,y3)
	//P4 (x4,y4)
	//
	//Point a (intersection express with respect to Line a)
	//Pa = P1 + Ua * (P2 - P1) 
	//
	//Point b (intersection express with respect to Line b)
	//Pb = P3 + Ub * (P4 - P3) 
	//
	//Solving thse 2 equations where Pa = Pb (both equation represent the same intersection point)
	//x1 + Ua * (x2 - x1) = x3 + Ub * (x4 - x3)
	//y1 + Ua * (y2 - y1) = y3 + Ub * (y4 - y3)
	//Ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / D
	//Ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / D
	//D = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1)

	double Ua_numerator = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3));
	double Ub_numerator = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3));
	double U_denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	if( (Ua_numerator==0. || Ub_numerator==0.) && U_denominator==0. )
	{
		return FALSE; //lines are coincident
	}
	else if(U_denominator==0.) 
	{
		return FALSE; //lines are parallel
	}
	
	double Ua = Ua_numerator/U_denominator;
	double Ub = Ub_numerator/U_denominator;

	/* //this x and y could be anywhere along the lines (Line a and Line b)
	double x = x1 + Ua * (x2-x1);
	double y = y1 + Ua * (y2-y1);
	*/
	//x and y will be garanteed to be within line segments (Line Segment a and Line Segment b)
	//if Ua is between 0 and 1 (x,y is garanteed to be within Line Segment a - P1,P2 bound)
	//if Ub is between 0 and 1 (x,y is garanteed to be within Line Segment b - P3,P4 bound)
	if( (Ua>=0. && Ua<=1.) && (Ub>=0 && Ub<=1.))
	{
		return TRUE; //intersect within bounds	
	}

	return FALSE; //don't intersect within line segment boundary
}

//this function computes m (the slope) and b (origin ordinate) for the
//line defined by the two points (x1,y1 and x2,y2). 
int LineMB(	double x1, double y1, double x2, double y2, //line segment a
			double* m, double* b) //y = mx + b
{
	ASSERT(m!=NULL && b!=NULL);
	if( (x2-x1)==0.) 
	{
		/*
		m[0] = 0.;
		b[0] = 0.;
		return FALSE;
		*/
		x1 = x1 + 0.00000000000001;
	}

	//Line a
	//P1 (x1,y1)
	//P2 (x2,y2)
	//
	
	//slope m
	m[0] = (y2-y1)/(x2-x1);
	b[0] = y1 - m[0]*x1;
	//double bb = (y2 - m[0]*x2);
	//ASSERT(bb==b[0]);
	return TRUE;
}

int IsPointUpLineMB(double x, double y, //point
					double* m, double* b) //lineMB, y = mx + b
{
	ASSERT(m!=NULL && b!=NULL);
	double det = y - m[0]*x - b[0];
	
	if(det==0.) return FALSE;
	else if(det>0.) return TRUE;
	else return FALSE;

	return TRUE;
}

//2012august11, poirier, begin
//see line-line-intersection.rtf
int LineSegmentsIntersect(	double x1, double y1, double x2, double y2, //line segment 1
							double x3, double y3, double x4, double y4, //line segment 2
							double* pfX, double* pfY) 
{
	if(pfX==NULL || pfY==NULL)
	{
		ASSERT(FALSE);
		return 0;
	}
	double A1=y2-y1;
	double B1=x1-x2;
	double C1=A1*x1+B1*y1;

	double A2=y4-y3;
	double B2=x3-x4;
	double C2=A2*x3+B2*y3;

    double det = A1*B2 - A2*B1;
    if(det == 0)
	{
        //Lines are parallel
		return -1;
    }
	else
	{
        *pfX = (B2*C1 - B1*C2)/det;
        *pfY = (A1*C2 - A2*C1)/det;
		//This gives the location of the intersection of two lines, but we need intersection of the line segments
		//return 1;
    }
	//make sure that the point found is on both of the line segments (must keep testing for line segments)
	if((min(x1,x2) <= *pfX) && (*pfX <= max(x1,x2)))
	{
		if((min(y1,y2) <= *pfY) && (*pfY <= max(y1,y2)))
		{
			if((min(x3,x4) <= *pfX) && (*pfX <= max(x3,x4)))
			{
				if((min(y3,y4) <= *pfY) && (*pfY <= max(y3,y4)))
				{
					//*pfX is valid
					//*pfY is valid
					//line segments do intersect 
					return 1;
				}
			}
		}
	}
	*pfX = 0.0; //*pfX is not valid
	*pfY = 0.0; //*pfY is not valid
	return 0; //line segments do not intersect
}
//2012august11, poirier, end

#undef COMPILING_POINTSET_CPP

