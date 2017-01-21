import net.java.games.jogl.*;

public class HelloWorld {
  public static void main (String args[]) {
    try {
      System.loadLibrary("jogl");
      System.out.println("Hello World! (The native libraries are installed.)");
      GLCapabilities caps = new GLCapabilities();
      System.out.println("Hello JOGL! (The jar appears to be available.)");
    } catch (Exception e) {
      System.out.println(e);
    }
  }
}
