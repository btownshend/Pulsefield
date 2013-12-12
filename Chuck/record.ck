// Record output into a file

dac => Gain g => WvOut2 out => blackhole;
1.0=>g.gain;
"audio.wav" => string capture;
if (me.args())
   me.arg(0)=>capture;

capture =>out.wavFilename;
<<<"Writing to ",capture>>>;
60::second=>now;
out.closeFile();
<<<"Closed ",capture>>>;

