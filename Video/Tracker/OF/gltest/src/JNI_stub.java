public class JNI_stub  {
    public native void nsetup();
    public native void nupdate();
    public native void ndraw();
    
    static {
        String jlp=System.getProperty("java.library.path");
        System.out.println("java.library.path="+jlp);
        System.loadLibrary("gltest_jni");
    }
}
