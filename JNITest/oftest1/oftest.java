class oftest {
    private native void print();
    public static void main(String[] args) {
        new oftest().print();
    }
    static {
        System.loadLibrary("exampleLibrary");
    }
}
