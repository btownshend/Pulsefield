package DinoDuino;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.Charset;

import gnu.io.CommPortIdentifier; 
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent; 
import gnu.io.SerialPortEventListener; 

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingDeque;


public class SerialManager implements SerialPortEventListener {
	SerialPort serialPort;
	/**
	* A BufferedReader which will be fed by a InputStreamReader 
	* converting the bytes into characters 
	* making the displayed results codepage independent
	*/
	private BufferedReader input;
	/** The output stream to the port */
	private OutputStream output;
	/** Milliseconds to block while waiting for port open */
	private static final int TIME_OUT = 2000;
	/** Default bits per second for COM port. */
	private static final int DATA_RATE = 115200;

	private final BlockingQueue<String> messageQueue = new LinkedBlockingDeque<String>();
	
	private final Charset charset = Charset.forName("UTF-8");
	
	public SerialManager(final CommPortIdentifier portId) {
		Thread serialThread = new Thread() {
			public void run() {
				initializeSerial(portId);
			}
		};
		serialThread.setName("Serial");
		serialThread.setDaemon(true);
		serialThread.start();
	}
	
	protected void finalize() throws Throwable {
		close();
	}
	
	public void pushMessage(String msg) {
		System.out.printf("Queuing message %s\n", msg);
		messageQueue.add(msg);
	}
	
	private void initializeSerial(CommPortIdentifier portId) {
		System.out.printf("PortID %s", portId.getName());
		try {
			// open serial port, and use class name for the appName.
			serialPort = (SerialPort) portId.open(this.getClass().getName(),
					TIME_OUT);

			// set port parameters
			serialPort.setSerialPortParams(DATA_RATE,
					SerialPort.DATABITS_8,
					SerialPort.STOPBITS_1,
					SerialPort.PARITY_NONE);

			// open the streams
			input = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));
			output = serialPort.getOutputStream();

			// add event listeners
			serialPort.addEventListener(this);
			serialPort.notifyOnDataAvailable(true);
			
			// Recommended so that the Arduino has time to wake up.
			
			Thread.sleep(1500);
			while (true) {
				String msg = messageQueue.take();
				System.err.printf("Writing %s\n", msg);
				byte[] bytes = msg.getBytes(charset);
				output.write(bytes, 0, bytes.length);
				output.flush();
			}
		} catch (Exception e) {
			close();
			System.err.println(e.toString());
		}
	}

	/**
	 * This should be called when you stop using the port.
	 * This will prevent port locking on platforms like Linux.
	 */
	public void close() {
		System.out.println("Closing SerialManager");
		if (serialPort != null) {
			serialPort.removeEventListener();
			serialPort.close();
			serialPort = null;
		}
	}

	/**
	 * Handle an event on the serial port. Read the data and print it.
	 */
	public synchronized void serialEvent(SerialPortEvent oEvent) {
		if (oEvent.getEventType() == SerialPortEvent.DATA_AVAILABLE) {
			try {
				String inputLine=input.readLine();
				System.out.println(inputLine);
			} catch (Exception e) {
				System.err.println(e.toString());
				close();
			}
		}
		// Ignore all the other eventTypes, but you should consider the other ones.
	}
}
