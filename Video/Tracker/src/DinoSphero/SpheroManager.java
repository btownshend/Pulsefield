package DinoSphero;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingDeque;


public class SpheroManager {
	private static final String endpoint = "http://localhost:9999";
	
	private class SpheroMessage {
		public float heading;
		public float velocity;
		public float duration;
	}
	
	private final BlockingQueue<SpheroMessage> messageQueue = new LinkedBlockingDeque<SpheroMessage>();

	public SpheroManager() {
		Thread t = new Thread() {
			public void run() {
				loop();
			}
		};
		t.start();
	}
	
	private void loop() {
		try {
			while (true) {
				SpheroMessage msg = messageQueue.take();
				
				String urlstr = String.format("%s/move", endpoint);
				String params = String.format("heading=%f&velocity=%f&duration=%f", msg.heading, msg.velocity, msg.duration);
				
				URL url = new URL(urlstr);
				HttpURLConnection connection = null;
				try {
					connection = (HttpURLConnection)url.openConnection();           
					connection.setDoOutput(true);
					connection.setDoInput(true);
					connection.setInstanceFollowRedirects(false); 
					connection.setRequestMethod("POST"); 
					connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded"); 
					connection.setRequestProperty("charset", "utf-8");
					connection.setRequestProperty("Content-Length", "" + Integer.toString(params.getBytes().length));
					connection.setUseCaches(false);

					DataOutputStream wr = new DataOutputStream(connection.getOutputStream());
					try {
						wr.writeBytes(params);
						wr.flush();
					}
					finally {
						wr.close();
					}
				}
				catch (IOException exn) {
					System.err.println(exn.getLocalizedMessage());
				}
				finally {
					if (connection != null) {
						connection.disconnect();
					}
				}
			}
		}
		catch (InterruptedException exn) {
		}
		catch (MalformedURLException exn) {
			System.err.println(exn.getLocalizedMessage());
		}
	}
	
	public void sendMove(float heading, float velocity, float duration) {
		SpheroMessage msg = new SpheroMessage();
		msg.heading = heading;
		msg.velocity = velocity;
		msg.duration = duration;
		messageQueue.add(msg);
	}
}
