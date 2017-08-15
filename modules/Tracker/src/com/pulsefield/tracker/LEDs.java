package com.pulsefield.tracker;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.time.Instant;
import java.util.logging.Logger;

public class LEDs extends Thread {
	private final static Logger logger = Logger.getLogger(Tracker.class.getName());
	public static LEDs theLEDs = null;
	String hostName;
	int portNumber;
	Socket socket;
	Instant lastOpenAttempt;
	final int retryInterval=5000;   // Number of msec between open attempts
	int syncCounter;
	int lastSyncReceived;   // Last sync counter value received
	byte lastRead;
	InputStream is;
	OutputStream os;
	static int phase;
	final int nstrip=8;
	final int ledperstrip=160;
	final int nphys=nstrip*ledperstrip;
	final float MAXUPDATERATE=20;   // Update at this rate; throttled by arduino
	int leds[];  // Color to set LED, packed the same as for processing
	private int physleds[];  // Current state of each LED in Arduino
	int ledOrder[];    // Map from arduino LED number to sequence here; -1 indicates LED should be blanked
	Boolean outsiders[];  // Outsiders, true if someone present; array maps full circle, starting at logical LED 1

	public LEDs(String host, int port) {
		hostName=host;
		portNumber=port;
		syncCounter=0;
		// State of each LED in arduino
		physleds=new int[nphys];
		for (int i=0;i<physleds.length;i++)
			physleds[i]=-1;
		// Ordering of LEDS;   physical LED p ->  ledOrder[p] logical LED;  ledOrder[0]==-1 for unused LEDs
		ledOrder=new int[nphys];
		for (int i=0;i<ledOrder.length;i++)
			ledOrder[i]=i;
		// Desired new state of logical LEDs
		leds=new int[nphys];
		for (int i=0;i<leds.length;i++)
			leds[i]=0x0;
		// Outsiders
		outsiders=new Boolean[360];
		for (int i=0;i<outsiders.length;i++)
			outsiders[i]=(i<90);
		start();
	}

	void open() {
		lastOpenAttempt = Instant.now();
		logger.finer("Attempting to open socket to "+hostName+":"+portNumber);
		try {
			socket = new Socket();
			socket.connect(new InetSocketAddress(hostName, portNumber),1000);
			//socket.setSendBufferSize(50);
			logger.config("Opened socket to "+hostName+":"+portNumber+" with sendbuf="+socket.getSendBufferSize());
			socket.setTcpNoDelay(true);
			is=socket.getInputStream();
			os=socket.getOutputStream();
		} catch (Exception e) {
			logger.info("Unable to open socket to Arduino at "+hostName+":"+portNumber+" : "+e.getMessage());
		}
	}

	void close() {
		if (socket.isConnected())
			try {
				socket.close();
			} catch (IOException e) {
				logger.warning("Unable to close socket: "+e.getMessage());
			}
	}

	void flush() {
		try{
			os.flush();
		} catch (Exception e) {
			logger.warning("Unable to flush socket output: "+e.getMessage() );
			try {
				socket.close();
			} catch (IOException e1) {
				; // ignore
			}
		}
	}

	// Reopen socket if needed;  return true if open
	boolean keepalive() {
		if (socket!=null && socket.isConnected() && !socket.isClosed()) {
			return true;
		}
		if (Instant.now().isBefore(lastOpenAttempt.plusMillis(retryInterval)))
			return false;
		open();
		return socket.isConnected();
	}

	// Send data - return 0 if success
	int send(byte data[]) {
		//logger.info("send("+((char)data[0])+",["+(data.length-1)+"]");
		if (!keepalive())
			return -1;
		try {
			os.write(data);
		} catch (Exception e) {
			close();
			logger.warning("Unable to send data to arduino socket: "+e.getMessage()+" isconnected="+socket.isConnected()+", isclosed="+socket.isClosed());
			return -1;
		}
		flush();
		return 0;
	}

	// Send sync request
	void sendsync() {
		byte msg[]=new byte[2];
		msg[0]='V';
		syncCounter=(syncCounter+1)&0xff;
		msg[1]=(byte)syncCounter;
		send(msg);
		logger.info("Sent sync "+syncCounter);
	}

	void syncwait() {
		int nattempts=50;
		//		if (lastSyncReceived==syncCounter)
		//			sendsync();  // Keep an extra sync pending
		while (lastSyncReceived != syncCounter && nattempts-->0) {
			keepalive();
			try {
				while (is.available()>0) {
					int d = is.read();
					logger.info("Available: "+is.available()+", got "+d);
					if (d==-1)
						break;
					if (lastRead=='A') {
						lastSyncReceived=d;
						logger.info("At sync "+syncCounter+", received sync "+lastSyncReceived);
						int nbehind=syncCounter-lastSyncReceived;
						if (nbehind<0) nbehind+=256;
						if (nbehind > 0)
							logger.warning("Behind by "+nbehind+" sync messages");
					}
					lastRead=(byte)d;
				}
			} catch (IOException e) {
				logger.warning("exception: "+e.getMessage());
			}
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {
				; // ignore
			}
		}
	}

	byte[] makeFUpdate(int pos, int nsend, int newPhys[]) {
		byte msg[]=new byte[nsend*3+4];
		msg[0]='F';
		msg[1]=(byte)(pos&0xff); 
		msg[2]=(byte)((pos>>>8)&0xff);  // First LED to set
		msg[3]=(byte)nsend;
		for (int i=0;i<nsend;i++) {
			int c = newPhys[i+pos];
			physleds[i+pos]=c;
			msg[i*3+4]=(byte)((c&0xff00)>>>8|0x80);  //green
			msg[i*3+5]=(byte)((c&0xff0000)>>>16|0x80);  // red
			msg[i*3+6]=(byte)((c&0xff)|0x80);  // blue
		}
		return msg;
	}
	
	void update() {
		logger.info("update");

		int newPhys[]=new int[nphys];
		int ndiffs=0;
		for (int i=0;i<nphys;i++) {
			int ind=ledOrder[i];
			if (ind>=0)
				newPhys[i]=leds[ind];
			else
				newPhys[i]=0;  // Blank unused LEDs
			ndiffs+=(newPhys[i]==physleds[i])?0:1;
		}
		int pos=0;
		while (pos<nphys) {
			int nsend=nphys-pos;
			if (nsend>160) nsend=160;
			byte msg[]=makeFUpdate(pos,nsend,newPhys);
			sendsync();
			send(msg);
			syncwait();
			pos+=nsend;
		}

		byte goMsg[]=new byte[3];
		goMsg[0]='P';   // Pause setting
		int pauseTime=(int)(1000/MAXUPDATERATE+0.5);
		assert(pauseTime<=255);
		goMsg[1]=(byte)pauseTime;
		goMsg[2]='G';  // Go
		send(goMsg);
	}

	// Separate thread to process input
	public void run() {
		logger.info("Started thread");
		open();
		while (true) {
			// Loop forever, updating Arduino with current LED settings
			if (!keepalive())
				continue;
			long start=System.nanoTime();
			//echotest();
			//echotest();
			bg();
			fg();
			update();

			long finish=System.nanoTime();
			logger.info("Update loop took "+(finish-start)/1000000+" msec");
		}
		// Not reached
	}

	// Public functions

	public int numLED() { return leds.length; }

	public void clear() {
		for (int i=0;i<leds.length;i++)
			leds[i]=0;
	}

	public void set(int i, int color) {
		leds[i]=color;
	}

	public void set(int i, int red, int green, int blue) {
		leds[i]=(red<<16)|(green<<8)|blue;
	}

	public int get(int i) {
		return leds[i];
	}

	void bg() {
		//stripid();
		//rainbow();
		pulsebow();
	}

	// Set strips for ID
	void stripid() {		
		final String colnames[]={"red","green", "blue","magenta","cyan","yellow","pinkish", "white"};
		final int col[][]={{255,0,0},{0,255,0},{0,0,255},{255,0,255},{0,255,255}, {255,255,0}, {255,64,64}, {255,255,255}};
		final int clkpins[]={48,46,44,42,36,34,32,40};
		final int datapins[]={49,47,45,43,37,35,33,41};
		final int nphase=3;
		clear();
		for (int k=0;k<nstrip;k++) {
			int offset=k*ledperstrip;
			for (int i=phase;i<ledperstrip;i+=nphase)
				set(offset+i,col[k][0],col[k][1],col[k][2]);
			logger.info(String.format("Strip %d is %s.  Clk=pin %d, Data=pin %d\n", k, colnames[k],clkpins[k],datapins[k]));
		}
		phase=(phase+1)%nphase;
	}

	void pulsebow() {
		final float pperiod=4;  // Period of pulsing (in seconds)
		final float cperiod=20;   // Period of color rotation
		final float pspatial=250;  // Spatial period of pulsing
		final float cspatial=20000;  // Spatial period of color
		final float maxlev=1;
		final float minlev=0.2f;

		// Amplitude overall
		double t=System.nanoTime()/1.0e9;
		for (int i=0;i<leds.length;i++) {
			double p0=i*2*Math.PI/pspatial;

			double c0=i*2*Math.PI/cspatial;
			//cshift=[c0;c0+2*pi/3;c0+4*pi/3]';
			double pphase=(t*2*Math.PI/pperiod+p0)%(2*Math.PI);
			double amp=minlev+(maxlev-minlev)*(Math.sin(pphase)+1)/2;
			int v=0;
			for (int j=0;j<3;j++) {
				double cphase=(t*2*Math.PI/cperiod+c0+2*j*Math.PI/3)%(2*Math.PI);
				double col=(Math.sin(cphase)+1)/2;
				double lev=(Math.pow(amp*col,2)*0.97+.03) * 127;   // Response is nonlinear (approx squared)
				//String.format("t=%.1f, phase=%.0f, amp(1,:)=%.2f %.2f %.2f,lev=%.0f %.0f %.0f",t(1,1),phase(1,1)*180/pi,amp(1,:),lev(1,:));
				v=v|((int)lev)<<(16-8*j);
			}
			leds[i]=v;
		}
	}

	void rainbow() {
		final float baseperiod = 3;
		final float rperiod = baseperiod;
		final float gperiod = baseperiod + 2;
		final float bperiod = baseperiod + 3;
		final int offset=1;
		double t=System.nanoTime()/1.0e9;

		int r = (int)(63 + 63 * Math.cos(2 * Math.PI * (t / rperiod  )));
		int g = (int)(63 + 63 * Math.cos(2 * Math.PI * (t / gperiod  )));
		int b = (int)(63 + 63 * Math.cos(2 * Math.PI * (t / bperiod )));
		logger.info("t="+t+",r="+r+", g="+g+", b="+b);
		for (int index = 0; index < numLED()-offset; index++) {
			leds[index]=leds[index+offset];
		}
		for (int index = 1; index <= offset; index++)
			leds[numLED()-offset]=r<<16 | g<<8 | b;
		logger.info("Last LED="+leds[numLED()-1]);
	}

	void echotest() {
		final int ndata=10;
		byte data[]=new byte[ndata+3];
		data[0]='E';
		data[1]=ndata;
		data[2]=ndata;
		for (int i=0;i<ndata;i++)
			data[i+3]=(byte)i;
		logger.info("Running echotest with "+ndata+" bytes");
		send(data);
		for (int i=0;i<ndata;i++) {
			int d;
			try {
				d = is.read();
			} catch (IOException e) {
				logger.warning("echotest exception: "+e.getMessage());
				return;
			}
			if (d<0) {
				logger.warning("Echotest returned -1");
				return;
			}
			logger.info("Echo test["+i+"] "+data[i+3]+"->"+d);
		}
	}
	
	// Set number of divisions used in outsider settings
	synchronized public void setOutsiderDivisions(int ndiv) {
		if (ndiv!=outsiders.length) {
			logger.config("Resizing outsiders from "+outsiders.length+" to "+ndiv);
			outsiders=new Boolean[ndiv];
		}
	}
	
	// Set outsiders flags for given pos from 32-bits packed into val
	public void setOutsiders(int pos, int val) {
		for (int i=0;i<32;i++) {
			if (pos*32+i >= outsiders.length)
				break;
			outsiders[pos*32+i]=(val&(1<<i))!=0;
		}
	}

	// Update LEDs based on outsider flags
	synchronized void fg() {
		logger.info("fg: outsiders.length="+outsiders.length);
		for (int i=0;i<outsiders.length;i++) {
			if (outsiders[i]) {
				int k1=(int)((i-0.5)*leds.length/outsiders.length);
				int k2=(int)((i+0.5)*leds.length/outsiders.length);
				for (int k=k1;k<k2;k++) {
					if (k>=0 && k<leds.length)
						leds[k]=0xff0000;  // RED for outsiders
				}
			}
		}
	}

}


