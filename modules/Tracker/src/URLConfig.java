import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

public class URLConfig {
	String filename;
	HashMap<String,String> hosts;
	HashMap<String,Integer> ports;

	URLConfig(String configFile) throws IOException {
		System.out.println("Loading config from "+configFile);
		filename=new String(configFile);
		hosts=new HashMap<String,String>();
		ports=new HashMap<String,Integer>();
		BufferedReader fd=null;

		fd=new BufferedReader(new FileReader(configFile));

		String line;
		while ((line=fd.readLine()) != null) {
			if (line.length() < 2)
				continue;
			String parts[]=line.split(",");
			if (parts[0].charAt(0) == '-') 
				;
			else if (parts.length!=3)
				System.out.println("Bad line in "+configFile+": "+line);
			else {
				hosts.put(parts[0], parts[1]);
				ports.put(parts[0], Integer.parseInt(parts[2].trim()));
			}
		}
		fd.close();
	}

	String getHost(String ident)  {	
		return hosts.get(ident);
	}

	int getPort(String ident)  {	
		return ports.get(ident);
	}

}
