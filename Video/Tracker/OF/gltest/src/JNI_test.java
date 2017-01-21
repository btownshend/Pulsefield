public class JNI_test {
    
    public static void main(String args[]) {
        Thread t = Thread.currentThread();
        System.out.println("Current thread: " + t);
        
        JNI_stub stub=new JNI_stub();
        stub.nsetup();
        while (true) {
            stub.ndraw();
        }
    }
}
