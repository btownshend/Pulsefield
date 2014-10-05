package DinoDuino;

import java.util.*;

import gnu.io.CommPortIdentifier;


public class AllSerialManager {
	private static final String PORT_NAMES[] = { 
		"/dev/tty.usbmodemfd121",
		"/dev/tty.usbserial-A9007UX1", // Mac OS X
                    "/dev/ttyACM0", // Raspberry Pi
		"/dev/ttyUSB0", // Linux
		"COM3", // Windows
	};
	
	private final ArrayList<SerialManager> managers = new ArrayList<SerialManager>();

	public void initialize() {
		// the next line is for Raspberry Pi and 
		// gets us into the while loop and was suggested here was suggested http://www.raspberrypi.org/phpBB3/viewtopic.php?f=81&t=32186
		System.setProperty("gnu.io.rxtx.SerialPorts", "/dev/ttyACM0");

		@SuppressWarnings("unchecked")
		Enumeration<CommPortIdentifier> portEnum = CommPortIdentifier.getPortIdentifiers();

		while (portEnum.hasMoreElements()) {
			CommPortIdentifier currPortId = portEnum.nextElement();
			System.out.printf("Found port %s\n", currPortId.getName());
			for (String portName : PORT_NAMES) {
				if (currPortId.getName().equals(portName)) {
					synchronized (managers) {
						managers.add(new SerialManager(currPortId));
					}
				}
			}
		}
	}
	
	public int getPortCount() {
		synchronized (managers) {
			return managers.size();
		}
	}
	
	public SerialManager getManager(int manager) {
		synchronized (managers) {
			return managers.get(manager);
		}
	}
}
