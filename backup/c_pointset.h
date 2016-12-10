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
/////////////////////////////////////////////////////////////
// POINTSET.H
//
// the original oiii pointset library enabling to store and 
// retreive point primitive as well as computing the TIN for
// the set.
//
// it is a c library, fully stand alone, can be exported to
// other plateforms and compilers.
//
// as of sept 2001, this libray is directly used by the gui's
// COW2Doc and do not depend upon the above c++ COWPointset class 
//
// TIN level code rewritten and augmented to provide voronoi
// representation. 
//
// copyright offworld technologies corporation 2001
/////////////////////////////////////////////////////////////
#ifndef _POINTSET_H
#define _POINTSET_H


#define OIFIILIB_API
#ifndef LONG
#define LONG long
#endif
#ifndef ASSERT
#define ASSERT assert
#endif
#ifndef BOOL
#define BOOL bool
#endif
//
// pointset library macros
//
#ifndef NULL
#define NULL	((void*) 0)
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

//#define MAXPTS	  10100      // nbre max de points    
//#define MAXTRI    2*MAXPTS   // nbre max de triangles 

#define MAXOPT    600        // niveau max d'optimisation 
#define DBERR	  1.e-15     // precision des calculs 
#define MAXDBL	  1.e+15
#define MINDBL	  1.e-15;

#define POINTSET_TAGFORINVALIDVOROAREA			-9999.99                      
#define POINTSET_NUMTRIANGLESOVERNUMPOINTS		2
#define POINTSET_INITIAL_NUMBEROFPOINTS			4000
#define POINTSET_ADDITIONAL_NUMBEROFPOINTS		1000

#define POINTSET_MAXIORDER		5	//used in neighborhood function
                      
#define POINTSET_MAX_TOTALNUMBEROFSTAT			20	// maximum including POINTSET_EXTRA_NUMBEROFSTAT +  number of stat specified by user 
#define POINTSET_DEFAULT_NUMBEROFSTAT			4	// 
#define POINTSET_EXTRA_NUMBEROFSTAT				9 //8	// used in dlog.mc and 
#define POINTSET_OFFSETSTAT_INTENSITY			9 //8
                                                 
// inserted into statistics                                                  
#define POINTSET_OFFSETSTAT_CLASSTREEINTENSITY		8
#define POINTSET_OFFSETSTAT_NEIGHVARIANCETREEAREA	7
#define POINTSET_OFFSETSTAT_NEIGHVARIANCEINTENSITY	6
#define POINTSET_OFFSETSTAT_NEIGHVARIANCEVOROAREA	5
#define POINTSET_OFFSETSTAT_NEIGHAVERAGETREEAREA	4
#define POINTSET_OFFSETSTAT_NEIGHAVERAGEINTENSITY	3
#define POINTSET_OFFSETSTAT_NEIGHAVERAGEVOROAREA	2
#define POINTSET_OFFSETSTAT_VORODENSITY				1
#define POINTSET_OFFSETSTAT_VOROAREA				0

// the following added with POINTSET_EXTRA_NUMBEROFSTAT should give the actual offset in the statistics 
#define POINTSET_XYZI_INTENSITY		0
#define POINTSET_XYZI_TREEDIMX		1
#define POINTSET_XYZI_TREEDIMY		2
#define POINTSET_XYZI_TREEAREA		3


//allocated in pointset.cpp, should be consistent with POINTSET_DEFAULT_NUMBEROFSTAT + POINTSET_EXTRA_NUMBEROFSTAT
#ifndef COMPILING_POINTSET_CPP
	extern char* pszStatChannelNames[];
	extern int jnd[]; //jnd[3] = {1,2,0};
	extern int jrd[]; //jrd[3] = {2,0,1};
	extern int ijk[]; //ijk[6] = {0,1,2,0,1,2};
#endif



//these values should be consistent with the statistics channels at all time.
//for instance, they are automatically updated when NormalizeAllVertex() is called. 
//when modified by other mecanism, UpdateGlobalStatisticsInfo() should be called.
typedef struct OIFIILIB_API tagGLOBALSTATISTICSINFO
{
	double fMin;
	double fMax; 
	double fAverage;
	double fVariance; 
} GLOBALSTATISTICSINFO;

typedef struct OIFIILIB_API tagPOINTSET
{

/////////////////////////////////////////////////////////////////
//
//                      .
//                  .     .
//				.     t2    .	  Given, a triangle t with
//           v1 . . . . . . v2    vertices are: v1, v2, v3.
//          . .           . .     The neighboring triangles
//         .   .   T   .   .      can be specified as t1,
//	  		. t1	.   .	  .	  t2 and t3.
//	 		.  .  . v3   t3 .
//                 .    .         Ex: v3, has opposite side v1-v2
//                  .  .              and corresponding neighboring
//                   .                triangle t3.
//
// In the following vt[i][j], with i = 0,1, 2
//                                 j = 0 to max number of triangle
//       j, is an index specifying a given triangle.
//	 i, is an index specifying either v1, v2 or v3.
//       vt[i][j], is an index specifying a given point
//		   (index for px[] and py[] arrays). these
//		   vertices are always stored in counter-clockwise
//		   order.
//
// In the following nt[i][j], with i = 0,1, 2
//                                 j = 0 to max number of triangle
//       j, is an index specifying a given triangle.
//	 i, is an index specifying either t1, t2 or t3.
//       nt[i][j], is an index specifying a given neighboring triangle
//		   (index for vt[][] array).  these triangles are always
//		   stored in counter-clockwise order.  Ex: t3 is always
//		   the neighboring triangle to t and opposite to v2-v3
//		   sides.  see the preceding illustration for vt[i][t]
//		   and nt[i][t] relationship.  See also the function
//		   GetAdjTri(vi, vj, t).
/////////////////////////////////////////////////////////////////

int ntri;	 	// number of triangles 
int* vt[3];	 	// int vt[3][MAXTRI];	triangle vertices:  v1, v2, v3	
int* nt[3];	 	// int nt[3][MAXTRI];	triangle neighbors: t1, t2, t3	
double* ctx; 	// double ctx[MAXTRI];	triangle centers x		
double* cty;	// double cty[MAXTRI];	triangle centers y		


/////////////////////////////////////////////////////////////////////
//
// Since each input data points are also
// vertices of one or many triangles, we
// do not have to duplicate coordinates.
//
// In the following px[i] and py[i], with i = 0 to max number of points
//       i, is an index specifying a given input point.
//       px[i], x float coordinate of point i.
//       py[i], y float coordinate of point i.
/////////////////////////////////////////////////////////////////////

int npts;
double* px;     			// double px[MAXPTS]; 
double* py;     			// double py[MAXPTS]; 
int* dcd;      				// int dcd[MAXPTS];  
//float* pfVoronoiDensity;  // 1/voronoi area   
//spi, begin
float* controlratio;
//spi, end

//an INFOSTATISTICS structure is allocated for each channel of statistics
GLOBALSTATISTICSINFO pGlobalStatisticsInfo[POINTSET_MAX_TOTALNUMBEROFSTAT];

double* pfStatistics;   // this pointer should be allocated when reading the input file 
int nStatPerPoint;     // number of statistics per point 
int nSizeStatInByte;   // total number of bytes required by pfStatistics 


///////////////////////////////////////////////////////////
// rectangle containing all points. initialize when reading
// the input file.  should be maintained up to date.
///////////////////////////////////////////////////////////

double xmin;
double xmax;
double ymin;
double ymax;


//////////////////////////////////////////////
// maximum number of elements in this pointset
//////////////////////////////////////////////

long maxnumberofelements;


//////////////////////////////////////////////////////////////
// The input filename will be store in the PointSet structure.
// This PointSet structure will contain all pointers to tri-
// angle lists, triangle centers lists, etc..
//////////////////////////////////////////////////////////////

char filename[255];


/////////////////////////////////////////////////////////
//not sure if should be here or declared as optim() local
/////////////////////////////////////////////////////////

int loptim;

} POINTSET;



//////////////////////////////////////
// POINTSET constructor and destructor
//////////////////////////////////////

OIFIILIB_API POINTSET* NewPointset(long maxnumberofelements);
//struct tagPOINTSET* OIFIILIB_API NewPointset(long maxnumberofelements); //spi 2014

OIFIILIB_API void DeletePointset(POINTSET* pPointset);

OIFIILIB_API LONG ReallocPointset(POINTSET*, long new_maxnumberofelements);

OIFIILIB_API POINTSET* NewPointsetCopy(POINTSET* pPointset);
OIFIILIB_API  void TranslatePointset(POINTSET* pPointset, double x_offset, double y_offset);


OIFIILIB_API LONG NewPointsetStatistics(POINTSET* pPointset, long maxnumberofelements, long statperelement);
OIFIILIB_API LONG ReallocPointsetStatistics(POINTSET* pPointset, long new_maxnumberofelements);
OIFIILIB_API void DeletePointsetStatistics(POINTSET* pPointset);

OIFIILIB_API double* GetPointsetPointerToStatistics(POINTSET* pPointset, long ielement);




///////////////////////////////////////////////////////
// Public function called to construct triangle network
///////////////////////////////////////////////////////

void OIFIILIB_API BuildTriangleNetwork(POINTSET* pPointset);



/////////////////////////////////////////////////////////////
// Public functions called in responce to various queries on
// spatial adjacencies.  These functions assumes the triangle
// network has been previously constructed.
/////////////////////////////////////////////////////////////

void OIFIILIB_API ComputeAllTriangleCenters(POINTSET* pPointset);
void OIFIILIB_API ComputeTriangleCenter(POINTSET* pPointset, int idtri);

int OIFIILIB_API FindTriContainingVertex(POINTSET* pPointset,
							int ivertex,
							int itriseed);

int OIFIILIB_API FindTriContainingPoint(POINTSET* pPointset,
						   double xa,
						   double ya,
						   int* p_itriseed);

int OIFIILIB_API FindTriContainingPoint_CAT(	POINTSET* pPointset,
								double xa,
								double ya,
								int* p_itriseed,
								int* p_numtrifound,
								int* p_arraytri);

int OIFIILIB_API FindTriContainingPoint_CATAV(	POINTSET* pPointset,
									double xa,
									double ya,
									int* p_itriseed,
									int* p_numtrifound,
									int* p_arraytri,
									int* p_numvertexfound,
									int* p_arrayvertex);

int OIFIILIB_API FindTriContainingPoint_CATAV(	POINTSET* pPointset,
									double xa,
									double ya,
									int* p_itriseed,
									int* p_numtrifound,
									int* p_arraytri,
									int* p_numvertexfound1,
									int* p_arrayvertex1,
									int* p_numvertexfound2,
									int* p_arrayvertex2);

int OIFIILIB_API FindNearestNeighbor(POINTSET* pPointset,
						double xa,
						double ya,
						int* p_itriseed);

int OIFIILIB_API InvalidTri( POINTSET* pPointset,
				int itri);

//poirier, march 27 1997, see below
int OIFIILIB_API CheckNeighborConsistensyForAllVertex(POINTSET* pPointset);
//poirier, march 27 1997, use this function to guaranty spatial consistency
//between all returned neighbors.  for instance, the neighbors that are
//found to be much further away from input ivertex are rejected.
//
//this function relies on voronoi area and shape calculation, so it
//must be called after those calculation.
//
//this function should be used when computing neighboring average values
//as well as for computing variance values and initializing neighbor
//lists before segmentation. 
int OIFIILIB_API FindAllConsistentNeighborSurroundingVertex( POINTSET* pPointset,
												int  ivertex,
												int* p_itriseed,
												int* p_numneighborfound,
												int* p_arrayneighbor );

int OIFIILIB_API FindAllTriSurroundingVertex( POINTSET* pPointset,
				 				 int  ivertex,
								 int* p_itriseed,
								 int* p_numtrifound,
								 int* p_arraytri,
								 int* p_arrayneighbor );

int OIFIILIB_API FindAllValidTriSurroundingVertex( POINTSET* pPointset,
				      				  int  ivertex,
								      int* p_itriseed,
								      int* p_numtrifound,
								      int* p_arraytri,
								      int* p_numneighborfound,
								      int* p_arrayneighbor );

int OIFIILIB_API FindAllNeighborSurroundingTri( POINTSET* pPointset,
								   int	itri,
								   int	iorder,
								   int* p_final_numfound,
								   int* final_arrayneighbor);

int OIFIILIB_API FindAllNeighborSurroundingVertex(	POINTSET* pPointset,
										int	iVertex,
										int	iOrder,
										int* p_iTriSeed,
										int* p_final_numfound,
										int* final_arrayneighbor);


int OIFIILIB_API GetAdjTri(POINTSET* pPointset,
			  int ivertex1,
			  int ivertex2,
			  int itri);

int OIFIILIB_API ComputeVoronoiAreaForAllVertex(POINTSET* pPointset);
int OIFIILIB_API ComputeVoronoiAreaForVertex(POINTSET* pPointset, int idvertex);

int OIFIILIB_API ComputeNeighboringVoronoiDensityForAllVertex(POINTSET* pPointset);

// this function is a generalized ComputeNeighboringVoronoiDensityForAllVertex() that
//   can be applied to any statistics present in the pPointset->pfStatistics array                 
int OIFIILIB_API ApplyAverageFilterToAllVertex(	POINTSET* pPointset, 
									int iInputStatOffset, 
									int iOutputStatOffset, 
									int iNearestNeighborOrder);

int OIFIILIB_API ComputeLocalAverage(POINTSET* pPointset,
						int iInputStatOffset,
						int iOutputStatOffset, //stat offset for average
						int iNearestNeighborOrder);

int OIFIILIB_API ComputeLocalVariance(POINTSET* pPointset,
						 int iInputStatOffset,
						 int iInputAverageStatOffset,//stat offset of previously computed average (in relation with iInputStatOffset)
						 int iOutputStatOffset, //stat offset for variance
						 int iNearestNeighborOrder);

int OIFIILIB_API InitializeClassToZero(	POINTSET* pPointset,
							int iStatOffset);

int OIFIILIB_API FindGlobalMinMax(POINTSET* pPointset, 
					 int iModifiedStatOffset);

int OIFIILIB_API ComputeGlobalAverage(POINTSET* pPointset, 
						 int iStatOffset);

int OIFIILIB_API ComputeGlobalVariance(POINTSET* pPointset, 
						  int iStatOffset);

int OIFIILIB_API UpdateGlobalStatisticsInfo(	POINTSET* pPointset,
								int iModifiedStatOffset);

int OIFIILIB_API NormalizeAllVertex(	POINTSET* pPointset, 
						int iInputStatOffset, 
						int iOutputStatOffset,
						double fMinOutputValue,
						double fMaxOutputValue);
                

///////////////////////////////////////////////////////////////////
// Public functions called to save various info to disk (debug use)
///////////////////////////////////////////////////////////////////

void OIFIILIB_API save_triangle_to_file(POINTSET* pPointset);
void OIFIILIB_API save_neighbor_and_center_to_file(POINTSET* pPointset);
void OIFIILIB_API save_2nd_order_neighbors_from_memory(POINTSET* pPointset);




///////////////////////////////////////////////////
// Private functions used by BuildTriangleNetwork()
///////////////////////////////////////////////////

void swapv(int* pv,
	   int i1,
	   int i2);

int ckturn(POINTSET* pPointset,
	   int* pv,
	   int i2,
	   int i3);

int cklop(POINTSET* pPointset,
	  int i1,
	  int i2,
	  int i3,
	  int i4);

int optim(POINTSET* pPointset,
	  int it,
	  int k1);

int swaptr(POINTSET* pPointset,
	   int t1,
	   int t2,
	   int k1,
	   int k2);

int xcmp(POINTSET* pPointset,
	 int* i1,
	 int* i2);

int bcmp(POINTSET* pPointset,
	 int* i1,
	 int* i2);

//void fsort(POINTSET* pPointset,
//	   int* pv,
//	   int i1,
//	   int i2,
//	   int (*fnz)());

void fsort(POINTSET* pPointset,
	   int* pv,
	   int i1,
	   int i2,
	   int (*fnz)(POINTSET*, int*, int*));


int LineSegmentsIntersect(	double x1, double y1, double x2, double y2, //line segment a
							double x3, double y3, double x4, double y4); //line segment b

int LineMB(	double x1, double y1, double x2, double y2, //line segment 
			double* m, double* b); //y = mx + b

int IsPointUpLineMB(double x, double y, //point
					double* m, double* b); //lineMB, y = mx + b

//2012august11, poirier, begin
int LineSegmentsIntersect(	double x1, double y1, double x2, double y2, //line segment 1
							double x3, double y3, double x4, double y4, //line segment 2
							double* pfX, double* pfY); 
//2012august11, poirier, end

#endif // _POINTSET_H 
