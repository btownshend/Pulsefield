import ddf.minim.AudioListener;
//  class for FFT visualization
import ddf.minim.AudioSource;
import ddf.minim.Minim;
import ddf.minim.analysis.FFT;
import processing.core.PApplet;

/// abstract class for audio visualization

 abstract class AudioRenderer implements AudioListener {
		Minim minim;

  float[] left;
  float[] right;
  public synchronized void samples(float[] samp) { left = samp; }
  public synchronized void samples(float[] sampL, float[] sampR) { left = sampL; right = sampR; }
  
  	AudioRenderer(PApplet parent) {
		// setup player
		minim = new Minim(parent);
		minim.getLineIn().addListener(this);
  	}
}

public class Fourier extends AudioRenderer {
	FFT fft; 
	float maxFFT;
	float[] leftFFT;
	float[] rightFFT;
	float[] monoFFT;
	
	Fourier(PApplet parent) {
		super(parent);
		AudioSource source=minim.getLineIn();
		float gain = (float) .125;
		fft = new FFT(source.bufferSize(), source.sampleRate());
		PApplet.println("FFT buffer size="+source.bufferSize()+", SR="+source.sampleRate());
		maxFFT =  source.sampleRate() / source.bufferSize() * gain;
		fft.window(FFT.HAMMING);
	}

	void calc(int bands) {
		if(left != null) {
			monoFFT = new float[bands];
			leftFFT = new float[bands];
			int totalOctaves=(int)(Math.log(fft.specSize())/Math.log(2f)+0.5);
			int sampsPerOctave=(bands+totalOctaves-1)/totalOctaves;
			int baseFreq=(int)(fft.getBandWidth()*sampsPerOctave*totalOctaves/bands);
			fft.logAverages(baseFreq,sampsPerOctave);
//			PApplet.println("bands="+bands+", specSize="+fft.specSize()+", totalOct="+totalOctaves+",sampsPerOct="+sampsPerOctave+",baseFreq="+baseFreq+", navg="+fft.avgSize());
			fft.forward(left);
			for(int i = 0; i < bands; i++) { leftFFT[i] = fft.getAvg(i);   monoFFT[i]=leftFFT[i]; }
			if(right != null) {
				rightFFT = new float[bands];
				fft.linAverages(bands);
				fft.forward(right);
				for(int i = 0; i < bands; i++) { rightFFT[i] = fft.getAvg(i);  monoFFT[i]+=rightFFT[i]; }
			}
		}
	}
}
