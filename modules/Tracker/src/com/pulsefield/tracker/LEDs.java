package com.pulsefield.tracker;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.time.Instant;
import java.util.logging.Logger;

public class LEDs extends Thread {
	private final static Logger logger = Logger.getLogger(Tracker.class.getName());
	public static LEDs theLEDs = null;
	private final int refreshPeriod=100;  // Refresh rate in msec
	String hostName;
	int portNumber;
	Socket socket;
	Instant lastOpenAttempt;
	final int retryInterval=5000;   // Number of msec between open attempts
	byte syncCounter;
	byte lastSyncReceived;   // Last sync counter value received

	int nused, nphys;
	int leds[];  // Color of each LED, packed the same as for processing
	int ledOrder[];    // Map from arduino LED number to sequence here; -1 indicates LED should be blanked
	int divisions;   // Number of outsider divisions
	
	public LEDs(String host, int port) {
		hostName=host;
		portNumber=port;
		syncCounter=0;
		nphys=8*160;
		ledOrder=new int[nphys];
		for (int i=0;i<ledOrder.length;i++)
			ledOrder[i]=i;
		nused=nphys;
		leds=new int[nused];
		start();
	}

	void open() {
		lastOpenAttempt = Instant.now();
		logger.finer("Attempting to open socket to "+hostName+":"+portNumber);
		try {
			socket = new Socket();
			socket.connect(new InetSocketAddress(hostName, portNumber),1000);
			logger.config("Opened socket to "+hostName+":"+portNumber);
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
	
	public int get(int i) {
		return leds[i];
	}
	
	// Separate thread to process input
	public void run() {
		logger.info("Started thread");
		open();
		int prev=0;
		long lastUpdate=System.nanoTime();
		while (true) {
			if (!keepalive())
				continue;
			update();
			sendsync();
			while (lastSyncReceived!=syncCounter) {
				int d;
				try {
					d = socket.getInputStream().read();
				} catch (IOException e) {
					logger.warning("Unable to read from socket: "+e.getMessage());
					d=-1;
				}
				if (d==-1)
					break; 
				if (prev=='A') {
					lastSyncReceived=(byte)d;
					int nbehind=syncCounter-lastSyncReceived;
					if (nbehind<0) nbehind+=256;
					if (nbehind > 1)
						logger.warning("Behind by "+nbehind+" sync messages");
				}
				prev=d;
			}
			long now=System.nanoTime();
			int wait=(int)(refreshPeriod-((now-lastUpdate)/1000000));
			if (wait>0)
				try {
					Thread.sleep(wait);
				} catch (InterruptedException e) {
					; // ignore
				}
		}
		// Not reached
	}

	// Reopen socket if needed;  return true if open
	boolean keepalive() {
		if (socket!=null && socket.isConnected()) {
			return true;
		}
		if (Instant.now().isBefore(lastOpenAttempt.plusMillis(retryInterval)))
			return false;
		open();
		return socket.isConnected();
	}

	// Send data - return 0 if success
	int send(byte data[]) {
		if (!keepalive())
			return -1;
		try {
			socket.getOutputStream().write(data);
		} catch (Exception e) {
			logger.warning("Unable to open arduino socket: "+e.getMessage());
			return -1;
		}
		return 0;
	}


	// Send sync request (and flush buffer)
	void sendsync() {
		byte msg[]=new byte[2];
		msg[0]='V';
		syncCounter++;
		msg[1]=syncCounter;
		send(msg);
		flush();
	}

	void flush() {
		try{
			socket.getOutputStream().flush();
		} catch (Exception e) {
			logger.warning("Unable to flush socket output: "+e.getMessage() );
		}
	}


	void clear() {
		for (int i=0;i<nused;i++)
			leds[i]=0;
	}

	void update() {
		byte msg[]=new byte[255+4];
		int pos=0;
		msg[0]='F';
		while (pos<nphys) {
			msg[1]=(byte)(pos&0xff); 
			msg[2]=(byte)(pos>>8);  // First LED to set
			int nsend=nphys-pos;
			if (nsend>255) nsend=255;
			msg[3]=(byte)nsend;
			for (int i=0;i<nsend;i++) {
				int ind=ledOrder[i+pos];
				int c;
				if (ind>=0)
					c=leds[ind];
				else
					c=0;  // Blank unused LEDs
				msg[i*3+4]=(byte)((c&0xff00)>>8);  //green
				msg[i*3+5]=(byte)((c&0xff0000)>>16);  // red
				msg[i*3+6]=(byte)((c&0xff));  // blue
			}
			send(msg);
			pos+=nsend;
		}
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
