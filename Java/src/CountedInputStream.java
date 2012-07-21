import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


public  class CountedInputStream extends FilterInputStream {
	int pos,oldpos;
	public CountedInputStream(InputStream is)  {
		// TODO Auto-generated constructor stub
		super(is);
		pos=0;
		oldpos=0;
	}

	public int read() throws IOException {
		int res= super.read();
		pos=pos+res;
		return res;
	}

	public int read(byte[] b) throws IOException {
		int res= super.read(b);
		pos=pos+res;
		return res;
	}

	public int read(byte[] b, int off, int len) throws IOException {
		int res=super.read(b,off,len);
		pos=pos+res;
		return res;
	}
	
	public int getpos() {
		return pos;
	}
	
	public void mark(int readlimit) {
		super.mark(readlimit);
		oldpos=pos;
	}
	
	public void reset() throws IOException {
		super.reset();
		pos=oldpos;
	}
}
