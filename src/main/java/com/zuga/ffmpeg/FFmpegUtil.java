package com.zuga.ffmpeg;

public class FFmpegUtil {

    private volatile static boolean mIsRunning = false;

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

    public static void compress(String inFile, String outFile) {
        if (mIsRunning) return;
        mIsRunning = true;
        ffmegCompress(inFile, outFile);
        mIsRunning = false;
    }

    public static boolean isRunning() {
        return mIsRunning;
    }

    private static native void initFFmpeg(boolean debug);

    private static native int ffmpegRun(String[] cmd);

    private static native int getThumb(String inputFile, String outputFile, String time);

    private static native void clipVideo(double startTime, double endTime, String inFile, String outFile);

    private static native void ffmegCompress(String inFile, String outFile);
}
