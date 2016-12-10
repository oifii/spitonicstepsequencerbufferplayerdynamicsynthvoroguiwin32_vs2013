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

#ifndef StereoDelayTestSynth_h
#define StereoDelayTestSynth_h

#include "Tonic.h"

using namespace Tonic;

class StereoDelayTestSynth : public Synth {
  
public:
  StereoDelayTestSynth(){
    
    ControlMetro metro = ControlMetro().bpm(100);
    ControlGenerator freq = ControlRandom().trigger(metro).min(0).max(1);
  
    Generator tone = RectWave().freq(
		//freq * addParameter("frequencyRandomAmount") + 100
		freq * addParameter("frequencyRandomAmount", 0.0).min(0.0).max(10000.0) + 100
		//+ addParameter("freq")
		+ addParameter("freq", 0.0).min(0.0).max(1.0)
      ) * SineWave().freq(50);
    
    ADSR env = ADSR()
      .attack(0.01)
      //.decay( addParameter("decay") )
	  .decay(addParameter("decay", 0.0).min(0.0).max(1.0))
	  .sustain(0)
      .release(0)
      .doesSustain(false)
      .trigger(metro);
    
	StereoDelay delay = StereoDelay(3.0f, 3.0f, 3.0f, 3.0f)
    .delayTimeLeft( 0.5 + SineWave().freq(0.2) * 0.01)
    .delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
    .feedback(0.3)
    .dryLevel(0.8)
	//.wetLevel(0.2);
	.wetLevel(0.8);
    
    Generator filterFreq = (SineWave().freq(0.01) + 1) * 200 + 225;
    
    LPF24 filter = LPF24().Q(2).cutoff( filterFreq );

	//Generator outputGen = filter.input(delay.input(tone * env)) * 0.3;
	Generator outputGen = filter.input(delay.input(tone * env)) * 1.0;
    
    setOutputGen(outputGen);
  }
  
  
};

//TONIC_REGISTER_SYNTH(StereoDelayTestSynth)

#endif
