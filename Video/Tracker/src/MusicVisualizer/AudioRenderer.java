package MusicVisualizer;
import processing.core.PApplet;
import ddf.minim.AudioListener;
import ddf.minim.AudioSource;
import ddf.minim.analysis.*;

/// abstract class for audio visualization

public abstract class AudioRenderer implements AudioListener {
  float[] left;
  float[] right;
  public synchronized void samples(float[] samp) { left = samp; }
  public synchronized void samples(float[] sampL, float[] sampR) { left = sampL; right = sampR; }
  public void start(PApplet parent) {}
  public void stop(PApplet parent) {}
  public abstract void draw(PApplet parent); 
  public abstract void drawLaserPerson(PApplet parent, int id);
}


// abstract class for FFT visualization

abstract class FourierRenderer extends AudioRenderer {
  FFT fft; 
  float maxFFT;
  float[] leftFFT;
  float[] rightFFT;
  float[] monoFFT;
  FourierRenderer(AudioSource source) {
    float gain = (float) .125;
    fft = new FFT(source.bufferSize(), source.sampleRate());
    maxFFT =  source.sampleRate() / source.bufferSize() * gain;
    fft.window(FFT.HAMMING);
  }
  
  void calc(int bands) {
    if(left != null) {
      monoFFT = new float[bands];
      leftFFT = new float[bands];
      fft.linAverages(bands);
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


