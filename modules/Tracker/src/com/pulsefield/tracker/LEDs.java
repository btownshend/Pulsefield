package com.pulsefield.tracker;

import java.io.BufferedOutputStream;
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
	private final int refreshPeriod=1000;  // Refresh rate in msec
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
	
	int nused;
	int leds[];  // Color of each LED, packed the same as for processing
	int ledOrder[];    // Map from arduino LED number to sequence here; -1 indicates LED should be blanked
	int divisions;   // Number of outsider divisions

	public LEDs(String host, int port) {
		hostName=host;
		portNumber=port;
		syncCounter=0;
		ledOrder=new int[nphys];
		for (int i=0;i<ledOrder.length;i++)
			ledOrder[i]=i;
		nused=nphys;
		leds=new int[nused];
		for (int i=0;i<nused;i++)
			leds[i]=0x111;
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

	public int numLED() { return nused; }

	public void set(int i, int color) {
		leds[i]=color;
	}
	
	public void set(int i, int red, int green, int blue) {
		leds[i]=(red<<16)|(green<<8)|blue;
	}

	public int get(int i) {
		return leds[i];
	}

	void processincoming() {
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
					if (nbehind > 1) {
						logger.warning("Behind by "+nbehind+" sync messages");
						try {
							Thread.sleep(10);
						} catch (InterruptedException e) {
							; // ignore
						}
					}
				}
				lastRead=(byte)d;
			}
		} catch (IOException e) {
			logger.warning("exception: "+e.getMessage());
		}
		//logger.info(" done");
	}

	void bg() {
		//stripid();
		rainbow();
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
				// TODO Auto-generated catch block
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
			update();

			long finish=System.nanoTime();
			logger.info("Update loop took "+(finish-start)/1000000+" msec");
		}
		// Not reached
	}

	void syncwait() {
		int nattempts=50;
//		if (lastSyncReceived==syncCounter)
//			sendsync();  // Keep an extra sync pending
		while (lastSyncReceived != syncCounter && nattempts-->0) {
			keepalive();
			processincoming();
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {
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


	void clear() {
		for (int i=0;i<nused;i++)
			leds[i]=0;
	}

	void update() {
		logger.info("update");
		if (false) {
			byte msg[]=new byte[4];
			msg[0]='A';
			msg[1]=(byte)0;
			msg[2]=(byte)0;
			msg[3]=(byte)0;
			send(msg);
			msg=new byte[6];
			for (int j=0;j<6;j++) {
			msg[0]='S';
			int ind=j*160+(syncCounter%160);
			msg[1]=(byte)(ind&0xff);
			msg[2]=(byte)((ind>>>8)&0xff);
			msg[3]=0x7f;
			msg[4]=0;
			msg[5]=0;
			send(msg);
			}
		}  else {
			int pos=0;

			while (pos<nphys) {
				int nsend=nphys-pos;
				if (nsend>160) nsend=160;

				byte msg[]=new byte[nsend*3+4];
				msg[0]='F';
				msg[1]=(byte)(pos&0xff); 
				msg[2]=(byte)((pos>>>8)&0xff);  // First LED to set
				msg[3]=(byte)nsend;
				for (int i=0;i<nsend;i++) {
					int ind=ledOrder[i+pos];
					int c;
					if (ind>=0)
						c=leds[ind];
					else
						c=0;  // Blank unused LEDs
					msg[i*3+4]=(byte)((c&0xff00)>>>8|0x80);  //green
					msg[i*3+5]=(byte)((c&0xff0000)>>>16|0x80);  // red
					msg[i*3+6]=(byte)((c&0xff)|0x80);  // blue
//					if (i==0)
//						logger.info("pos="+pos+", c="+c);
				}
				sendsync();
				send(msg);
				syncwait();
//				logger.info("msg="+msg[0]+","+(int)msg[1]+","+(int)msg[2]+","+(int)msg[3]);
//				String str="";
//				for (int i=4;i<msg.length;i++)
//					str=str+(((int)msg[i])&0xff)+",";
//				logger.info(str);
				pos+=nsend;
			}
		}
		byte goMsg[]=new byte[1];
		goMsg[0]='G';  // Go
		send(goMsg);
	}

	// Set outsiders flags for given pos from 32-bits packed into val
	public void setOutsiders(int pos, int val) {
		try {
			for (int i=0;i<32;i++) {
				int k1=(int)((pos*32+i-0.5)*nused/divisions);
				int k2=(int)((pos*32+i+0.5)*nused/divisions);
				if (k1>=nused)
					break;
				//logger.info("pos="+pos+", k1="+k1+", k2="+k2);
				for (int k=k1;k<k2;k++) {
					if (k>=0 && k<nused)
						if ((val & (1<<i)) != 0)
							leds[k]=0xff0000;
						else
							leds[k]=0x00ff00;
				}
			}
		} catch (Exception e) {
			logger.warning("setOutsiders: "+e.getMessage());
		}
	}

	// Set number of divisions used in outsider settings
	public void setOutsiderDivisions(int ndiv) {
		divisions=ndiv;
	}
}


