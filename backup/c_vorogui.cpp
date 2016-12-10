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
#include "stdafx.h"

#include "Tonic.h" //spi

#include "c_pointset.h"
#include "c_vorogui.h"

#include <assert.h>


#include "ControlSwitcherExpSynth.h"
#include "BandlimitedOscillatorExpSynth.h"
#include "BufferPlayerExpSynth.h"
#include "CompressorExpSynth.h"
#include "FMDroneExpSynth.h"
#include "FilteredNoiseSynth.h"
#include "StepSequencerBufferPlayerExpSynth.h"
#include "StepSequencerBufferPlayerEffectExpSynth.h"
#include "StepSequencerExpSynth.h"
#include "DelayExpSynth.h"
#include "FilterExpSynth.h"
#include "EventsExpSynth.h"
#include "EventsExpBufferPlayerSynth.h"
#include "LFNoiseTestSynth.h"
#include "ReverbTestSynth.h"
#include "SimpleStepSeqSynth.h"
#include "SimpleStepSequencerBufferPlayerSynth.h"
#include "SineSumSynth.h"
#include "XYSpeedSynth.h"
#include "StereoDelayTestSynth.h"
#include "CompressorDuckingTestSynth.h"


#include <Windowsx.h>

#include <typeinfo>    // for 'typeid'

#include <assert.h>

#include <fstream>
#include <iostream>
using namespace std;


extern int global_xwidth;
extern int global_yheight;
/*
extern class ControlSwitcherExpSynth* global_pSynth;
*/
//int global_xwidth;
//int global_yheight;
class Synth* global_pSynth;

//extern class BandlimitedOscillatorExpSynth* global_pSynth;
//extern class BufferPlayerExpSynth* global_pSynth;
//extern class CompressorExpSynth* global_pSynth;
//extern class CompressorDuckingTestSynth* global_pSynth;
//extern class FMDroneExpSynth* global_pSynth;
//extern class FilteredNoiseSynth* global_pSynth;
//extern class StepSequencerBufferPlayerExpSynth* global_pSynth;
//extern class StepSequencerBufferPlayerEffectExpSynth* global_pSynth;
//extern class StepSequencerExpSynth* global_pSynth;
//extern class DelayExpSynth* global_pSynth;
//extern class FilterExpSynth* global_pSynth;
//extern class EventsExpSynth* global_pSynth;
//extern class EventsExpBufferPlayerSynth* global_pSynth;
//extern class LFNoiseTestSynth* global_pSynth;
//extern class ReverbTestSynth* global_pSynth;
//extern class SimpleStepSeqSynth* global_pSynth;
//extern class SineSumSynth* global_pSynth;
//extern class SimpleStepSequencerBufferPlayerSynth* global_pSynth;
//extern class XYSpeedSynth* global_pSynth;
//extern class StereoDelayTestSynth* global_pSynth;

//vorogui globals
int vorogui_numberofframepoints = 0;
int vorogui_numberofpoints = 0;
int vorogui_itriseed = 0;
bool vorogui_displaytextlabelflag = false;
int vorogui_idpointtobemoved = -1;

float rand_FloatRange(float a, float b)
{
	return ((b - a)*((float)rand() / RAND_MAX)) + a;
}

void VOROGUI_Init(class Synth* pSynth)
{
	//global_xwidth = xwidth;
	//global_yheight = yheight;
	global_pSynth = pSynth;
}

POINTSET* VOROGUI_CreatePointset(bool buildtinflag)
{
	POINTSET* pPOINTSET = NULL;

	/*
	//find out total number of elements
	vorogui_numberofframepoints = 2 * (global_xwidth - 1) + 2 * (global_yheight-1);
	vector<ControlParameter> params = global_pSynth->getParameters();
	vorogui_numberofpoints = params.size();

	pPOINTSET = NewPointset(vorogui_numberofpoints + vorogui_numberofframepoints);
	//add frame of points
	pPOINTSET->npts = 0;
	for (int i = 0; i < (global_xwidth - 1); i++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + i + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int j = 0; j < (global_yheight - 1); j++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + (global_xwidth - 1) + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + j + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int i = 0; i < (global_xwidth - 1); i++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + (global_xwidth - 1) - i + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + (global_yheight - 1) + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int j = 0; j < (global_yheight - 1); j++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + (global_yheight - 1) - j + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	*/
	//find out total number of elements
	int nx = 20;
	int ny = 20;
	vorogui_numberofframepoints = 2 * nx + 2 * ny;
	vector<ControlParameter> params = global_pSynth->getParameters();
	vorogui_numberofpoints = params.size();

	pPOINTSET = NewPointset(vorogui_numberofpoints + vorogui_numberofframepoints);
	//add frame of points
	pPOINTSET->npts = 0;
	float fxstep = (global_xwidth - 1) / float(nx);
	float fystep = (global_yheight - 1) / float(ny);
	for (int i = 0; i < nx; i++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + i*fxstep + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int j = 0; j < ny; j++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + (global_xwidth - 1) + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + j*fystep + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int i = 0; i < nx; i++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + (global_xwidth - 1) - i*fxstep + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + (global_yheight - 1) + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}
	for (int j = 0; j < ny; j++)
	{
		pPOINTSET->px[pPOINTSET->npts] = 0.0 + rand_FloatRange(0.0, 0.01);
		pPOINTSET->py[pPOINTSET->npts] = 0.0 + (global_yheight - 1) - j*fystep + rand_FloatRange(0.0, 0.01);
		pPOINTSET->controlratio[pPOINTSET->npts] = -1.0;
		pPOINTSET->npts++;
	}

	//add points
	for (int i = vorogui_numberofframepoints; i < (vorogui_numberofpoints + vorogui_numberofframepoints); i++)
	{
		pPOINTSET->px[i] = 0.0 + rand_FloatRange(2.0, global_xwidth - 2.0);
		pPOINTSET->py[i] = 0.0 + rand_FloatRange(2.0, global_yheight - 2.0);
		pPOINTSET->controlratio[i] = 0.0;
		//verify if new point colliding
		if ((i - vorogui_numberofframepoints)>1)
		{
			for (int ii = vorogui_numberofframepoints; ii<(i - 1); ii++)
			{
				//if (abs(pPOINTSET->px[ii] - pPOINTSET->px[i])<0.0000001)
				if (abs(pPOINTSET->px[ii] - pPOINTSET->px[i])<1.0)
				{
					//if (abs(pPOINTSET->py[ii] - pPOINTSET->py[i])<0.0000001)
					if (abs(pPOINTSET->py[ii] - pPOINTSET->py[i])<1.0)
					{
						//reject newly added point
						i = i - 1;
						break;
					}
				}
			}
		}
	}
	//set control ratio according to synth's parameters
	//vector<ControlParameter> params = global_pSynth->getParameters();
	for (unsigned int i = 0; i < params.size(); i++)
	{
		TonicFloat min = params[i].getMin();
		TonicFloat max = params[i].getMax();
		TonicFloat value = params[i].getValue();
		string mystring = params[i].getName();

		if((max-min)!=0.0) pPOINTSET->controlratio[vorogui_numberofframepoints + i] = (value-min)/(max-min);
			else  pPOINTSET->controlratio[vorogui_numberofframepoints + i] = 0.0;
		//bypass for now
		//pPOINTSET->controlratio[vorogui_numberofframepoints + i] = 0.5;
	}

	pPOINTSET->npts = vorogui_numberofpoints + vorogui_numberofframepoints;
	/*
	pPOINTSET->xmin = 0.0;
	pPOINTSET->xmax = 0.0 + global_xwidth +1; //+1 to be safe because adding random offset
	pPOINTSET->ymin = 0.0;
	pPOINTSET->ymax = 0.0 + global_yheight +1; //+1 to be safe because adding random offset
	*/
	for (int i = 0; i < pPOINTSET->npts; i++)
	{
		if (pPOINTSET->px[i] < pPOINTSET->xmin) pPOINTSET->xmin = pPOINTSET->px[i];
		if (pPOINTSET->px[i] > pPOINTSET->xmax) pPOINTSET->xmax = pPOINTSET->px[i];
		if (pPOINTSET->py[i] < pPOINTSET->ymin) pPOINTSET->ymin = pPOINTSET->py[i];
		if (pPOINTSET->py[i] > pPOINTSET->ymax) pPOINTSET->ymax = pPOINTSET->py[i];
	}

	/*
	FILE* pFILE = fopen("debug.txt", "w");
	for (int i = 0; i < pPOINTSET->npts; i++)
	{
		fprintf(pFILE, "%f, %f\n", pPOINTSET->px[i], pPOINTSET->py[i]);
	}
	fclose(pFILE);
	*/
	if (buildtinflag)
	{
		BuildTriangleNetwork(pPOINTSET);
		ComputeAllTriangleCenters(pPOINTSET);
	}

	return pPOINTSET;
}

void VOROGUI_DestroyPointset(POINTSET* pPOINTSET)
{
	DeletePointset(pPOINTSET);
}


bool VOROGUI_GetVoronoiPolygon(POINTSET* pPOINTSET, int ivertex, POINT* pPointOutputPolygon, int* p_numpointoutputpolygon)
{
	if (pPOINTSET == NULL || pPointOutputPolygon == NULL || p_numpointoutputpolygon == NULL)
	{
		return 0;
	}

	int numtrifound, numneighborfound;//, itriseed;
	//int itriseed=0;
	int p_arraytri[200];
	int p_arrayneighbor[200];
	if (FindAllValidTriSurroundingVertex(pPOINTSET,
		ivertex,
		&vorogui_itriseed, //&itriseed, //&vorogui_itriseed,
		&numtrifound,
		p_arraytri,
		&numneighborfound,
		p_arrayneighbor) == TRUE)
	{
		//if all surrounding triangles are valid,		 
		// build voronoi polygon using each adjtri's center 
		assert(numtrifound<200); //development-time error, if allowed, define p_arraytri[n] with n>200
		for (int j = 0; j<numtrifound; j++)
		{
			POINT myPoint;
			myPoint.x = pPOINTSET->ctx[p_arraytri[j]];
			myPoint.y = pPOINTSET->cty[p_arraytri[j]];
			pPointOutputPolygon[j] = myPoint;
		}
		// To close the polygon, the last point is equal to the first point 
		pPointOutputPolygon[numtrifound].x = pPointOutputPolygon[0].x;
		pPointOutputPolygon[numtrifound].y = pPointOutputPolygon[0].y;
		*p_numpointoutputpolygon = numtrifound + 1;
		return 1;
	}
	*p_numpointoutputpolygon = 0;
	return 0;
}

void VOROGUI_GetPolygonMinMax(POINT* pPointInputPolygon, int numpointinputpolygon, long* p_xmin, long* p_xmax, long* p_ymin, long* p_ymax, int* p_idpointymin)
{
	*p_xmin = LONG_MAX;
	*p_xmax = LONG_MIN;
	*p_ymin = LONG_MAX;
	*p_ymax = LONG_MIN;
	*p_idpointymin = -1;
	for (int i = 0; i<numpointinputpolygon; i++)
	{
		if (pPointInputPolygon[i].y<*p_ymin)
		{
			*p_ymin = pPointInputPolygon[i].y;
			*p_idpointymin = i;
		}
		if (pPointInputPolygon[i].y>*p_ymax)
		{
			*p_ymax = pPointInputPolygon[i].y;
		}
		if (pPointInputPolygon[i].x<*p_xmin)
		{
			*p_xmin = pPointInputPolygon[i].x;
		}
		if (pPointInputPolygon[i].x>*p_xmax)
		{
			*p_xmax = pPointInputPolygon[i].x;
		}
	}
}

void VOROGUI_SlicePolygon(POINT* pPointInputPolygon, int numpointinputpolygon, double dfValue, POINT* pPointOutputPolygon, int* p_numpointoutputpolygon)
{
	if (dfValue>1.0)
	{
		ASSERT(FALSE);
		return;
	}
	else if (dfValue == 1.0)
	{
		for (int i = 0; i<numpointinputpolygon; i++)
		{
			pPointOutputPolygon[i] = pPointInputPolygon[i];
		}
		*p_numpointoutputpolygon = numpointinputpolygon;
	}
	else
	{
		//1) find ymin and ymax (as well as xmin and xmax)
		long xmin, xmax, ymin, ymax;
		int idpointymin;
		VOROGUI_GetPolygonMinMax(pPointInputPolygon, numpointinputpolygon, &xmin, &xmax, &ymin, &ymax, &idpointymin);

		//2) find two points where level line intersect polygon
		/*
		//   and build the two new polygons
		POINT pPointOutputPolygon1[100];
		int numpointoutputpolygon1;
		POINT pPointOutputPolygon2[100];
		int numpointoutputpolygon2;
		*/

		double pfX[100]; //we only need 2, but for safety
		double pfY[100]; //we only need 2, but for safety
		double x3, y3, x4, y4; //level line segment
		x3 = xmin;
		x4 = xmax;
		//y3=ymin+(ymax-ymin)*dfValue;
		y3 = ymax - (ymax - ymin)*dfValue;
		y4 = y3;
		int idintersect = 0;
		for (int i = 0; i<numpointinputpolygon - 1; i++)
		{
			double x1, y1, x2, y2; //polygon segment
			x1 = pPointInputPolygon[i].x;
			y1 = pPointInputPolygon[i].y;
			x2 = pPointInputPolygon[i + 1].x;
			y2 = pPointInputPolygon[i + 1].y;
			if (LineSegmentsIntersect(x1, y1, x2, y2, //line segment 1
				x3, y3, x4, y4, //line segment 2
				&pfX[idintersect], &pfY[idintersect]) == 1)
			{
				idintersect++;
				if (idintersect>2)
				{
					//LineSegmentsIntersect has found a third intersecting point?
					//OK, that happens because of intersection on a vertex.
					//will have to prevent duplicate points in the output.
					//ASSERT(FALSE);
				}
			}

		}
		/*
		//3) select polygon and copy polygon points to output
		int idstart=idpointymin;
		int idend=idpointymin-1;
		if(idend<0) idend=numpointinputpolygon-2;
		*/
		//for NOW, only copy intersection points to output (avoid duplicates)
		pPointOutputPolygon[0].x = (long)pfX[0];
		pPointOutputPolygon[0].y = (long)pfY[0];
		*p_numpointoutputpolygon = 1;
		for (int i = 1; i<idintersect; i++)
		{
			//if(pPointOutputPolygon[i].x==pPointOutputPolygon[0].x && pPointOutputPolygon[i].y==pPointOutputPolygon[0].y)
			//if(pfX[i]==pPointOutputPolygon[0].x && pfY[i]==pPointOutputPolygon[0].y)
			if ((((long)pfX[i]) == pPointOutputPolygon[0].x && ((long)pfY[i]) == pPointOutputPolygon[0].y)
				//|| ((i==2)&&(((long)pfX[i])==pPointOutputPolygon[1].x && ((long)pfY[i])==pPointOutputPolygon[1].y)) )
				|| ((i>1) && (((long)pfX[i]) == pPointOutputPolygon[1].x && ((long)pfY[i]) == pPointOutputPolygon[1].y)))
			{
				//skip duplicate
				continue;
			}
			else
			{
				//pPointOutputPolygon[i].x=pfX[1];
				//pPointOutputPolygon[i].y=pfY[1];
				pPointOutputPolygon[1].x = (long)pfX[i];
				pPointOutputPolygon[1].y = (long)pfY[i];
				ASSERT(*p_numpointoutputpolygon != 2);
				//if this assert fails, we are overwriting onto the second output point.
				//that means more than 2 intersecting points were found (excluding duplicates),
				//this case was not forseen by this implementation. 
				*p_numpointoutputpolygon = 2;
			}
		}
	}
}

void VOROGUI_DrawPointset(POINTSET* pPOINTSET, HDC hdc)
{
	HPEN hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HBRUSH hbrush = CreateSolidBrush(RGB(50, 50, 50));
	HBRUSH hnullbrush = (HBRUSH)GetStockObject(NULL_BRUSH);

	HPEN holdpen = (HPEN)SelectObject(hdc, hpen);
	HBRUSH holdbrush = (HBRUSH)SelectObject(hdc, hbrush);

	vector<ControlParameter> params = global_pSynth->getParameters();
	for (unsigned int i = 0; i < params.size(); i++)
	{
		TonicFloat min = params[i].getMin();
		TonicFloat max = params[i].getMax();
		TonicFloat value = params[i].getValue();
		string mystring = params[i].getName();
	}
	
	//////////////////////
	//draw voronoi regions
	//////////////////////
	POINT pPointVoronoiPolygon[100];
	int numpointvoronoipolygon = 0;

	for (int idpoint = vorogui_numberofframepoints; idpoint<pPOINTSET->npts; idpoint++)
	{
		double dfValue = pPOINTSET->controlratio[idpoint];
		//double dfValue = 1.0;
		if (dfValue>=0.0)
		{
			//redraw each voronoi polygon but sliced in "y" according to the control level
			if (VOROGUI_GetVoronoiPolygon(pPOINTSET, idpoint, pPointVoronoiPolygon, &numpointvoronoipolygon) == TRUE)
			{
				if (dfValue == 1.0)
				{
					//////////////////////////////////////////////
					//redraw the voronoi polygon completely filled
					//////////////////////////////////////////////
					Polygon(hdc, pPointVoronoiPolygon, numpointvoronoipolygon); //+1 for the last point 
				}
				else if (dfValue == 0.0)
				{
					/////////////////////////////////////////////
					//redraw the voronoi polygon completely empty
					/////////////////////////////////////////////
					HBRUSH hpreviousbrush = (HBRUSH)SelectObject(hdc, hnullbrush);
					Polygon(hdc, pPointVoronoiPolygon, numpointvoronoipolygon); //+1 for the last point 
					SelectObject(hdc, hpreviousbrush);
				}
				else
				{
					/////////////////////////////////////////////
					//redraw the voronoi polygon completely empty
					/////////////////////////////////////////////
					HBRUSH hpreviousbrush = (HBRUSH)SelectObject(hdc, hnullbrush);
					Polygon(hdc, pPointVoronoiPolygon, numpointvoronoipolygon); //+1 for the last point 
					SelectObject(hdc, hpreviousbrush);

					///////////////////////////////////////
					//floodfill part of the voronoi polygon
					///////////////////////////////////////
					POINT pPointOutputPolygon[100];
					int numpointoutputpolygon;
					VOROGUI_SlicePolygon(pPointVoronoiPolygon, numpointvoronoipolygon, dfValue, pPointOutputPolygon, &numpointoutputpolygon);
					//for now, draw the intersection line
					if (numpointoutputpolygon == 2)
					{
						MoveToEx(hdc, pPointOutputPolygon[0].x, pPointOutputPolygon[0].y, NULL);
						LineTo(hdc, pPointOutputPolygon[1].x, pPointOutputPolygon[1].y);
						POINT myPOINT;
						myPOINT.x = (pPointOutputPolygon[0].x + pPointOutputPolygon[1].x) / 2;
						myPOINT.y = ((pPointOutputPolygon[0].y + pPointOutputPolygon[1].y) / 2) + 1; //one pixel below the intersection line's center
						FloodFill(hdc, myPOINT.x, myPOINT.y, RGB(255, 0, 0));
					}
					/*
					POINT myPOINT;
					myPOINT.x = (pPointOutputPolygon[0].x + pPointOutputPolygon[1].x) / 2;
					myPOINT.y = ((pPointOutputPolygon[0].y + pPointOutputPolygon[1].y) / 2) + 1; //one pixel below the intersection line's center
					FloodFill(hdc, myPOINT.x, myPOINT.y, RGB(255,0,0));
					*/
				}
				if (vorogui_displaytextlabelflag==true)
				{
					//display parameter name
					//TextOutW(hdc, pPOINTSET->px[idpoint], pPOINTSET->py[idpoint], L"test string", 11);
					string mystring = params[idpoint - vorogui_numberofframepoints].getName();
					TextOutA(hdc, pPOINTSET->px[idpoint], pPOINTSET->py[idpoint], mystring.c_str(), mystring.length());
				}
			}
			else
			{
				/*
				fprintf(pFile, "vertex=%d, numneighbor=%d,  ",ivertex,numneighborfound);
				for(j=0; j<numneighborfound; j++)
				{
				fprintf(pFile, " %d", p_arrayneighbor[j]);
				}
				fprintf(pFile, "\n");
				*/
			}
		}

	}
	SelectObject(hdc, holdpen);
	SelectObject(hdc, holdbrush);
	DeleteObject(hpen);
	DeleteObject(hbrush);
	return;
}

int VOROGUI_GetNearestPointsetObject(POINTSET* pPOINTSET, double dfX, double dfY, int* p_itriseed/*=NULL*/)
{
	if (pPOINTSET == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}
	if (dfX<(double)0 || dfX>(double)global_xwidth ||
		dfY<(double)0 || dfY>(double)global_yheight)
	{
		return -1;
	}
	int iTriSeed = 0;
	if (p_itriseed == NULL) p_itriseed = &iTriSeed;
	int idObject = FindNearestNeighbor(pPOINTSET, dfX, dfY, p_itriseed);
	return idObject;
}

void VOROGUI_OnLButtonDown(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);
	if (!(wParam&MK_SHIFT) && !(wParam&MK_CONTROL))
	{
		//////////////////////
		//change control level
		//////////////////////
		//get nearest pointset object 
		int idpoint = VOROGUI_GetNearestPointsetObject(pPOINTSET, xPos, yPos, &vorogui_itriseed);
		if (idpoint >= vorogui_numberofframepoints)
		{
			/*
			double dfValue=0.50;
			*/
			POINT pPointVoronoiPolygon[100];
			int numpointvoronoipolygon = 0;
			if (VOROGUI_GetVoronoiPolygon(pPOINTSET, idpoint, pPointVoronoiPolygon, &numpointvoronoipolygon) == TRUE)
			{

				long xmin, xmax, ymin, ymax;
				int idpointymin;
				VOROGUI_GetPolygonMinMax(pPointVoronoiPolygon, numpointvoronoipolygon, &xmin, &xmax, &ymin, &ymax, &idpointymin);
				double dfRatio = 0.0;
				if (ymax != ymin) dfRatio = abs(ymax - yPos)*1.0 / abs(ymax - ymin);

				if (dfRatio >= 0.0 && dfRatio <= 1.0)
				{
					pPOINTSET->controlratio[idpoint] = dfRatio;
				}
				else
				{
					//ASSERT(FALSE);
					if (dfRatio>1.0) dfRatio = 1.0;
					if (dfRatio<0.0) dfRatio = 0.0;
					pPOINTSET->controlratio[idpoint] = dfRatio;
				}
				vector<ControlParameter> params = global_pSynth->getParameters();
				//params[idpoint - vorogui_numberofframepoints].setNormalizedValue(dfRatio);
				TonicFloat min = params[idpoint - vorogui_numberofframepoints].getMin();
				TonicFloat max = params[idpoint - vorogui_numberofframepoints].getMax();
				string mystring = params[idpoint - vorogui_numberofframepoints].getName();
				global_pSynth->setParameter(mystring, dfRatio*(max - min) + min);
			}
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}
	}
	else if (wParam&MK_CONTROL)
	{
		/////////////////////////
		//store point to be moved
		/////////////////////////
		//get nearest pointset object
		int idpoint = VOROGUI_GetNearestPointsetObject(pPOINTSET, xPos, yPos, &vorogui_itriseed);
		if (idpoint >= vorogui_numberofframepoints)
		{
			vorogui_idpointtobemoved = idpoint;
		}
	}
	return;
}

void VOROGUI_OnLButtonUp(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	if (wParam&MK_CONTROL)
	{
		if (vorogui_idpointtobemoved != -1)
		{
			if (vorogui_idpointtobemoved >= vorogui_numberofframepoints)
			{
				//////////////////////////////////////////
				//check for collision with existing points
				//////////////////////////////////////////
				for (int i = 0; i < pPOINTSET->npts; i++)
				{
					if (abs(pPOINTSET->px[i] - xPos) < 1.0)
					{
						if (abs(pPOINTSET->py[i] - yPos) < 1.0)
						{
							//found collision
							//drop moving request
							vorogui_idpointtobemoved = -1;
							return;
						}

					}
				}
				////////////////////
				//safe to move point
				////////////////////
				pPOINTSET->px[vorogui_idpointtobemoved] = xPos;
				pPOINTSET->py[vorogui_idpointtobemoved] = yPos;
				vorogui_idpointtobemoved = -1;
				///////////////////////////////////////////////
				//update pointset's extent, tin and tri centers
				///////////////////////////////////////////////
				pPOINTSET->ntri = 0;
				pPOINTSET->xmin = MAXDBL;
				pPOINTSET->ymin = MAXDBL;
				pPOINTSET->xmax = MINDBL;
				pPOINTSET->ymax = MINDBL;
				for (int i = 0; i < pPOINTSET->npts; i++)
				{
					if (pPOINTSET->px[i] < pPOINTSET->xmin) pPOINTSET->xmin = pPOINTSET->px[i];
					if (pPOINTSET->px[i] > pPOINTSET->xmax) pPOINTSET->xmax = pPOINTSET->px[i];
					if (pPOINTSET->py[i] < pPOINTSET->ymin) pPOINTSET->ymin = pPOINTSET->py[i];
					if (pPOINTSET->py[i] > pPOINTSET->ymax) pPOINTSET->ymax = pPOINTSET->py[i];
				}
				BuildTriangleNetwork(pPOINTSET);
				ComputeAllTriangleCenters(pPOINTSET);

				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			}
		}
	}
}

void VOROGUI_OnRButtonUp(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (vorogui_displaytextlabelflag) vorogui_displaytextlabelflag = false;
		else vorogui_displaytextlabelflag = true;

		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
}

string VOROGUI_GetFilename()
{
	//use synth's class name without tags
	string mystring = typeid(global_pSynth).name();
	mystring = mystring.substr(6, mystring.length() - (6 + 2)); //remove "class" and " *" tags
	//add .txt extension
	mystring += ".txt";
	return mystring;
}

POINTSET* VOROGUI_ReadFromDisk()
{
	vector<ControlParameter> params = global_pSynth->getParameters();

	//create pointset
	POINTSET* pPOINTSET = VOROGUI_CreatePointset(false);

	//if file valid, replace all parameters values and positions
	string filename = VOROGUI_GetFilename();
	/*
	FILE* pFILE = fopen(filename.c_str(), "r");
	if (pFILE)
	{
		fclose(pFILE);
	}
	*/
	string line;
	ifstream myfile(filename);
	int linecount = 0;
	if (myfile)  // same as: if (myfile.good())
	{
		while (getline(myfile, line))  // same as: while (getline( myfile, line ).good())
		{
			linecount++;
			if (linecount == 1)
			{
				//skip line, do nothing
			}
			else
			{
				istringstream buf(line);
				istream_iterator<string> beg(buf), end;
				vector<string> tokens(beg, end);
				int tokencount = 0;
				string parametername = "";
				for (vector<string>::iterator it = tokens.begin(), end = tokens.end(); it != end; ++it)
				{
					tokencount++;
					if (tokencount==1)
					{
						//parametername
						parametername = params[linecount-2].getName();
						string parameternamefromfile = (*it);
						if (parametername != parameternamefromfile) break;
					}
					else if (tokencount==2)
					{
						//parametervalue
						float parametervalue = atof((*it).c_str());
						//params[linecount - 2].value = atof((*it).c_str());
						global_pSynth->setParameter(parametername, parametervalue);
						TonicFloat min = params[linecount - 2].getMin();
						TonicFloat max = params[linecount - 2].getMax();
						assert( (min <= parametervalue) && (parametervalue <= max) );
						if ((max - min) != 0.0) pPOINTSET->controlratio[vorogui_numberofframepoints + linecount - 2] = (parametervalue - min) / (max - min);
						else  pPOINTSET->controlratio[vorogui_numberofframepoints + linecount - 2] = 0.0;

					}
					else if (tokencount==3)
					{
						pPOINTSET->px[vorogui_numberofframepoints + linecount - 2] = atof((*it).c_str());
					}
					else if (tokencount == 4)
					{
						pPOINTSET->py[vorogui_numberofframepoints + linecount - 2] = atof((*it).c_str());
					}
				}
			}
		}
		pPOINTSET->xmin = MAXDBL;
		pPOINTSET->ymin = MAXDBL;
		pPOINTSET->xmax = MINDBL;
		pPOINTSET->ymax = MINDBL;
		for (int i = 0; i < pPOINTSET->npts; i++)
		{
			if (pPOINTSET->px[i] < pPOINTSET->xmin) pPOINTSET->xmin = pPOINTSET->px[i];
			if (pPOINTSET->px[i] > pPOINTSET->xmax) pPOINTSET->xmax = pPOINTSET->px[i];
			if (pPOINTSET->py[i] < pPOINTSET->ymin) pPOINTSET->ymin = pPOINTSET->py[i];
			if (pPOINTSET->py[i] > pPOINTSET->ymax) pPOINTSET->ymax = pPOINTSET->py[i];
		}

	}

	//build tin
	BuildTriangleNetwork(pPOINTSET);
	ComputeAllTriangleCenters(pPOINTSET);
	return pPOINTSET;
}

void VOROGUI_WriteToDisk(POINTSET* pPOINTSET)
{
	string filename = VOROGUI_GetFilename();
	FILE* pFILE = fopen(filename.c_str(), "w");
	if (pFILE)
	{
		fprintf(pFILE, "%s\t%s\t%s\t%s\n", "parametername", "parametervalue", "parameterxposition", "parameteryposition"); //file header
		vector<ControlParameter> params = global_pSynth->getParameters();
		assert((pPOINTSET->npts - vorogui_numberofframepoints) == params.size());
		for (int idpoint = vorogui_numberofframepoints; idpoint < pPOINTSET->npts; idpoint++)
		{
			string parametername = params[idpoint - vorogui_numberofframepoints].getName();
			TonicFloat parametervalue = params[idpoint - vorogui_numberofframepoints].getValue();
			fprintf(pFILE, "%s\t%f\t%f\t%f\n", parametername.c_str(), parametervalue, pPOINTSET->px[idpoint], pPOINTSET->py[idpoint]);
		}
		fclose(pFILE);
	}
}
/*
vector<ControlParameter> params = global_pSynth->getParameters();
for (unsigned int i = 0; i < params.size(); i++)
{
	TonicFloat min = params[i].getMin();
	TonicFloat max = params[i].getMax();
	TonicFloat value = params[i].getValue();
	string mystring = params[i].getName();
}
*/