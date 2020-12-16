package com.nexstreaming.anigifsdk;

/**
 * 1001 - nxAniGif Library 0.0.5 update & buffpool was modified.
 */
public final class nexAniGifSDK {

    private native int getHandle();
    private native int freeHandle(int handle);
    private native int initHandle(int handle, int cpuCoreCount, int width, int height, int mode, int delayMs, int quantization_method, float quantization_gamma, int sample_factor, int rgb666 );
    private native int pushByteRGBAFixels(int handle, int falgs, byte [] rgba );
    private native byte [] popResult(int handle);
    private native int encodeFileToFile(int handle, String inputFile, int inputFormat, String outFile, int async);
    private native int test(int handle, String inputFile, int inputFormat, String outFile);

    private native int useBufferPool(int handle, String outFile);
    private native int pushByteRGBA2BufferPool(int handle, int falgs, byte [] rgba);
    private native int outputWait();

    private final static int version = 1001;

    private int handle = -1;
    private static int countOfHandles = 0;

    private int width = 360;
    private int height = 360;
    private Mode mode = Mode.Quality;
    private int delayMs = 100;

    private int quantization_method = 1;	//default 1
    private float quantization_gamma = (float) 1.0;	//default 1.0
    private int sample_factor = 10;			//default 1
    private boolean rgb666 = true;

    private boolean isRunnable = false;
    private boolean useBufferPool = false;
    private int cpuCoreCount = 1;

    public final static int kFormat_RGBA = 0;
    public final static int kFormat_YUV420 = 1;

    public static enum Mode{
        Speed,
        Quality
    };

    private nexAniGifSDK(){

    }

    public static int versionCode(){
        return version;
    }

    public static nexAniGifSDK createInstance( ){
        nexAniGifSDK ret = new nexAniGifSDK();
        int h = ret.getHandle();
        if( h < 0 ){
            ret = null;
            return null;
        }
        countOfHandles++;
        ret.handle = h;
        return ret;

    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public Mode getMode() {
        return mode;
    }

    public void setMode(Mode mode) {
        this.mode = mode;
    }

    public int getDelayMs() {
        return delayMs;
    }

    public void setDelayMs(int delayMs) {
        this.delayMs = delayMs;
    }

    public int getQuantizationMethod() {
        return quantization_method;
    }

    public void setQuantizationMethod(int quantization_method) {
        this.quantization_method = quantization_method;
    }

    public float getQuantizationGamma() {
        return quantization_gamma;
    }

    public void setQuantizationGamma(float quantization_gamma) {
        this.quantization_gamma = quantization_gamma;
    }

    public int getSampleFactor() {
        return sample_factor;
    }

    public void setSampleFactor(int sample_factor) {
        this.sample_factor = sample_factor;
    }

    public void setUseRGB666(boolean bRgb666) {
        this.rgb666 = bRgb666;
    }

    public int getCpuCoreCount() {
        return cpuCoreCount;
    }

    public void setCpuCoreCount(int cpuCoreCount) {
        this.cpuCoreCount = cpuCoreCount;
    }

    public int setup(){
        if( handle < 0 ){
            throw new RuntimeException("already destroyed!");
        }

        int m = 0;
        if( mode == Mode.Quality ){
            m = 1;
        }

        int rval = initHandle( handle, cpuCoreCount,  width,  height,  m,  delayMs,  quantization_method,  quantization_gamma,  sample_factor, (rgb666)?1:0);
        if( rval == 0 )
            isRunnable = true;
        return rval;
    }

    public boolean useBufferPool(String outputFilePath){
        int i = useBufferPool(handle,outputFilePath);
        if( i == 0 ){
            useBufferPool = true;
        }else{
            useBufferPool = false;
        }
        return useBufferPool;
    }

    public byte[] encodeFrame(boolean isLastFrame, byte[] rgba ){
        if( useBufferPool ){
            throw new RuntimeException("running useBufferPool mode.");
        }

        if( !isRunnable ) {
            throw new RuntimeException("you must call init()!");
        }

        if( rgba == null ){
            throw new RuntimeException("input byte[] is null");
        }

        if( cpuCoreCount != 1 ){
            throw new RuntimeException("you must setCpuCoreCount(1)!");
        }

        if( rgba.length != (width * height *4) ){
            throw new RuntimeException("input byte[] size not "+(rgba.length)+"=/="+(width * height *4) );
        }

        int rval = pushByteRGBAFixels(handle,isLastFrame?1:0 ,rgba);
        if( rval <= 0 ){
            return new byte[0];
        }

        if(  isLastFrame ){
            byte[] r = popResult(handle);
            byte[] n = new byte[r.length+1];
            System.arraycopy(r,0,n,0,r.length);
            n[r.length] = 0x3b;
            return n;
        }
        return popResult(handle);
    }
    private int lastOutputSize = 0;

    public int pushRGBAFrame( byte[] rgba){
        if( rgba.length != (width * height *4) ){
            throw new RuntimeException("input byte[] size not "+(rgba.length)+"=/="+(width * height *4) );
        }

        if( useBufferPool ){
            pushByteRGBA2BufferPool(handle, 0, rgba);
            lastOutputSize = 0;
        }else {
            lastOutputSize = pushByteRGBAFixels(handle, 0, rgba);
        }
        return lastOutputSize;
    }

    public byte[] popEncodedFrame(){
        if( lastOutputSize > 0 ) {
            return popResult(handle);
        }
        return new byte[0];
    }

    public byte[] sendEoS(){
        if( useBufferPool ){
            pushByteRGBA2BufferPool(handle, 1, null);
            outputWait();
            return new byte[0];
        }
        if( lastOutputSize > 0 ){
            return new byte[0];
        }
        pushByteRGBAFixels(handle,1 ,null);
        return popResult(handle);
    }

    public int destroy(){
        freeHandle(handle);
        isRunnable = false;
        handle = -1;
        return 0;
    }

    public int encodeFileToFileAsync(String dumpFile, String outfile){
        return encodeFileToFile(handle,dumpFile,0,outfile,1);
    }

    public int encodeFileToFile(String dumpFile, String outfile){
        return encodeFileToFile(handle,dumpFile,0,outfile,0);
    }

    public int encodeFileToFileAsync(String dumpFile, int inputFormat, String outfile){
        return encodeFileToFile(handle,dumpFile,inputFormat,outfile,1);
    }

    public int encodeFileToFile(String dumpFile,int inputFormat, String outfile){
        return encodeFileToFile(handle,dumpFile,inputFormat,outfile,0);
    }


    public int pool(String dumpFile, String outfile){
        return test(handle,dumpFile,0,outfile);
    }

    static {
        System.loadLibrary("nxAniGifjni");
    }

}