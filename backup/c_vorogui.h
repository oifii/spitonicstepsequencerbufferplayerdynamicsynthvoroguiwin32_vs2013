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
#ifndef _VOROGUI_H
#define _VOROGUI_H

void VOROGUI_Init(class Tonic::Synth* pSynth);

POINTSET* VOROGUI_CreatePointset(bool buildtinflag=true);
void VOROGUI_DestroyPointset(POINTSET* pPOINTSET);

void VOROGUI_DrawPointset(POINTSET* pPOINTSET, HDC hdc);

void VOROGUI_OnLButtonDown(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam);
void VOROGUI_OnLButtonUp(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam);
void VOROGUI_OnRButtonUp(POINTSET* pPOINTSET, HWND hwnd, WPARAM wParam, LPARAM lParam);

POINTSET* VOROGUI_ReadFromDisk();
void VOROGUI_WriteToDisk(POINTSET* pPOINTSET);

#endif