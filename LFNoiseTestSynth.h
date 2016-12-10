/*
 * Copyright (c) 2015-2016 Stephane Poirier
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
//

/*
Because of the fancy TONIC_REGISTER_SYNTH macro, and because this synth has the same interface
as Synth it's not necessary to create a header file. You can create an instance of this synth using 
SynthFactory::createInstance(SYNTH_NAME), where SYNTH_NAME is the name supplied to TONIC_REGISTER_SYNTH.
*/

#ifndef LFNoiseTestSynth_h
#define LFNoiseTestSynth_h

#include "Tonic.h"

using namespace Tonic;

class LFNoiseTestSynth : public Synth{
public:
	LFNoiseTestSynth()
	{
		ControlParameter pitch = addParameter("sinefreq", 500).min(10.0).max(10000.0); //spi

		Generator outputGen = SineWave().freq(
		//500 + 500 * LFNoise().setFreq( //original
		pitch + pitch * LFNoise().setFreq(
		//addParameter("noiseFreq", 100)) //original
		addParameter("noisefreq", 100).min(1.0).max(10000.0)) //spi
		)
		//* 0.3; //original
		*addParameter("vol", 0.3).min(0.0).max(1.0); //spi
    
		setOutputGen(outputGen);
	}
};

//TONIC_REGISTER_SYNTH(LFNoiseTestSynth);


#endif
