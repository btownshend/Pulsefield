import javax.imageio.*;
import javax.imageio.stream.MemoryCacheImageInputStream;

import java.awt.image.*;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.InputStream;
import java.io.*;
import java.net.*;
import java.util.List;
import java.util.Map.Entry;

/**
 * 
 */

/**
 * @author bst
 *
 */
public class GetVisible {
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		System.out.println("Hello world\n");
		try {
			URL cam1=new URL("http://192.168.0.71/mjpeg?res=half&x0=0&x1=500&y0=0&y1=500&fps=15");
			HttpURLConnection conn= (HttpURLConnection)cam1.openConnection();
			//conn.setRequestMethod("GET");
			//conn.setRequestProperty("Accept-Charset", "UTF-8");
			if (conn.getResponseCode() != HttpURLConnection.HTTP_OK)
				System.err.println("Bad response: "+conn.getResponseCode());
			else {
				for (Entry<String, List<String>> header : conn.getHeaderFields().entrySet()) {
					System.out.println(header.getKey() + "=" + header.getValue());
				}
				InputStream is=conn.getInputStream();
//				is=new BufferedInputStream(new FileInputStream("/tmp/xx"));
				System.out.println("is is a "+is.toString());
				CountedInputStream cis=new CountedInputStream(is);
				DataInputStream ds=new DataInputStream(cis);
				ImageIO.setUseCache(false);
				for (int i=0;i<4;i++) {
					String s;
					int nskip=0;
					System.out.println("Searching for sep at pos="+cis.getpos());
					do {
						s=ds.readLine();
//						System.out.println("Got: "+s);
						nskip=nskip+1;
					} while (!s.equals("--fbdr"));
					System.out.println("Skipped "+nskip+" lines looking for --fbdr, found at pos="+cis.getpos());
					nskip=0;
					is.mark(100000);
					do {
						s=ds.readLine();
						nskip=nskip+1;
					} while (!s.equals(""));
					System.out.println("Skipped "+nskip+" lines, pos="+cis.getpos());
					int lastpos=cis.getpos();
					//					int b;
					//					for (int j=0;j<36;j++) {
					//						b=is.read();
					//						System.out.print(Integer.toString(b)+",");					
					//					}
					//					BufferedReader isr=new BufferedReader(new InputStreamReader(is,"US-ASCII"));
					//					for (int j=0;j<3;j++) {
					//						String s=isr.readLine();
					//						System.out.println("Content = '"+s+"'");
					//					}
					BufferedImage bi= ImageIO.read(ds);
					System.out.println("Got image of size "+bi.getWidth()+","+bi.getHeight());
					System.out.println("New pos="+cis.getpos());
					// Rewind in case ImageIO read past the next image
					is.reset();
					is.skip((cis.getpos()-lastpos)-1000);
				}
			}
		} catch (Exception e) {
			System.err.println("Exception: "+e.toString()+";"+ e.getLocalizedMessage());
		}
	}

}
