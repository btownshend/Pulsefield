package DinoDuino;


public class DinoManager {
	// For now, we just use one manager, and we send dinosaur * SERVOS_PER_DINO + servo to the Arduino.
	private static final int DINOS = 2;
	private static final int SERVOS_PER_DINO = 8;
	static boolean loggedError=false;
	private final AllSerialManager allSerialManager = new AllSerialManager();

	public void initialize() {
		System.err.println("DinoManager.initialize");
		Thread t = new Thread() {
			public void run() {
				allSerialManager.initialize();
			}
		};
		t.start();
	}

	public int size() {
		return DINOS;
	}

	public int numServos(int dinosaur) {
		return SERVOS_PER_DINO;
	}

	public void setServo(int dinosaur, int servo, float angle, float timeToMove) {
		System.out.printf("Asked to send %.3f %.3f to dino %d, servo %d\n", angle, timeToMove, dinosaur, servo);
		
		if (dinosaur >= DINOS) {
			System.err.println("Cannot send command -- not enough dinos");
			return;
		}

		if (servo >= numServos(dinosaur)) {
			System.err.println("Cannot send command -- not enough servos");
			return;
		}

		int managerNum = 0;
		int remoteServo = dinosaur * SERVOS_PER_DINO + servo;

		if (managerNum >= allSerialManager.getPortCount()) {
			if (!loggedError)
				System.err.printf("Cannot send command -- serial port %d not open\n", managerNum);
			loggedError=true;
			return;
		}

		// Message format:
		// S <servo num, 2 digits> <angle in degrees, 3 digits> <time in msec, 4 digits>
		String msg = String.format("S%02d%03d%04d\n", remoteServo, (int)angle, (int)(timeToMove * 1000));
		sendMessage(managerNum, msg);
	}


	public void setLED(int ledId, int brightness) {
		System.out.printf("Asked to set LED %2d brightness to %d\n", ledId, brightness);

		// Message format:
		// L <LED ID, 2 digits> <brightness 0-99, 2 digits>
		String msg = String.format("L%02d%02d\n", ledId, brightness);
		sendMessage(0, msg);
	}


	void sendMessage(int managerNum, String msg) {
		if (managerNum >= allSerialManager.getPortCount()) {
			if (!loggedError)
				System.err.printf("Cannot send command -- serial port %d not open\n", managerNum);
			loggedError=true;
			return;
		}

		SerialManager manager = allSerialManager.getManager(managerNum);
		//System.out.printf("Sending message %s to manager %d\n", msg, managerNum);
		manager.pushMessage(msg);
	}


	public static void main(String[] args) throws Exception {
		final DinoManager dino = new DinoManager();
		dino.initialize();

		System.out.println("Started");

		try {
			Thread.sleep(3000);
			dino.setServo(1, 1, 90.f, 1.0f);

			//the following line will keep this app alive for 1000 seconds,
			//waiting for events to occur and responding to them (printing incoming messages to console).
			Thread.sleep(1000000);
		}
		catch (InterruptedException ie) {
		}
	}
}
