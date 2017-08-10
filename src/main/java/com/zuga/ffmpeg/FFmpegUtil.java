package com.zuga.ffmpeg;

public class FFmpegUtil {

    private volatile static boolean mIsRunning = false;
    private static CompressListener mListener;

    static {
        System.loadLibrary("FFmpegLib");
    }

    public static int ffmpegCMDRun(String cmd) {
        if (mIsRunning) return -1;
        mIsRunning = true;
        String regulation = "[ \\t]+";
        final String[] split = cmd.split(regulation);
        int i = ffmpegRun(split);
        mIsRunning = false;
        return i;
    }

    public static void init(boolean debug) {
        if (mIsRunning) return;
        mIsRunning = true;
        initFFmpeg(debug);
        mIsRunning = false;
    }

    public static boolean chip(double startTime, double endTime, String inFile, String outFile) {
        if (mIsRunning) return false;
        mIsRunning = true;
        clipVideo(startTime, endTime, inFile, outFile);
        mIsRunning = false;
        return true;
    }

    public static int thumb(String inputFile, String outputFile, String time) {
        if (mIsRunning) return -1;
        mIsRunning = true;
        int thumb = getThumb(inputFile, outputFile, time);
        mIsRunning = false;
        return thumb;
    }

    public static int compress(String inFile, String outFile, long videoBitRate, long audioBitRate, int width, int height, int threadCount) {
        if (mIsRunning) return -1;
        mIsRunning = true;
        int ret = ffmegCompress(inFile, outFile, videoBitRate, audioBitRate, width, height, threadCount);
        mIsRunning = false;
        return ret;
    }

    public static void updateCompress(String videoPath, float progress) {
        if (mListener != null) {
            mListener.onUpdateProgress(videoPath, progress);
        }
    }

    public static void setCompressListener(CompressListener listener) {
        mListener = listener;
    }

    public static void cancelCompress() {
        cancelFFmpegCompress();
    }

    public static boolean isRunning() {
        return mIsRunning;
    }

    private static native void initFFmpeg(boolean debug);

    private static native int ffmpegRun(String[] cmd);

    private static native int getThumb(String inputFile, String outputFile, String time);

    private static native void clipVideo(double startTime, double endTime, String inFile, String outFile);

    private static native int ffmegCompress(String inFile, String outFile, long videoBitRate, long audioBitRate, int width, int height, int threadCount);

    private static native void cancelFFmpegCompress();

    public interface CompressListener {
        void onUpdateProgress(String videoPath, float progress);
    }
}
