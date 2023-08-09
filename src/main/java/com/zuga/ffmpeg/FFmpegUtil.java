package com.zuga.ffmpeg;

public class FFmpegUtil {

    private volatile static boolean mIsRunning = false;
    private volatile static boolean mIsFFmpegCmdRunning = false;
    private static CompressListener mListener;

    static {
        System.loadLibrary("FFmpegLib");
    }

    public static int ffmpegCMDRun(String cmd) {
        if (mIsFFmpegCmdRunning) return -1;
        mIsFFmpegCmdRunning = true;
        String regulation = "[ \\t]+";
        final String[] split = cmd.split(regulation);
        int i = ffmpegRun(split);
        mIsFFmpegCmdRunning = false;
        return i;
    }

    public static void init(boolean debug) {
        if (mIsRunning) return;
        mIsRunning = true;
        initFFmpeg(debug);
        mIsRunning = false;
    }

    public static int compress(int inFile, long offset, long length, int outFile, long videoBitRate, long audioBitRate, int width, int height, int videoID) {
        if (mIsRunning) {
            return -1;
        }
        mIsRunning = true;
        int ret = ffmegCompress(inFile, offset, length, outFile, videoBitRate, audioBitRate, width, height, videoID);
        mIsRunning = false;
        return ret;
    }

    public static void updateCompress(int videoId, float progress) {
        if (mListener != null) {
            mListener.onUpdateProgress(videoId, progress);
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

    private static native int ffmegCompress(int inFd, long offset, long length, int outFd, long videoBitRate, long audioBitRate,
                                            int width, int height, int videoID);

    private static native void cancelFFmpegCompress();

    public interface CompressListener {
        void onUpdateProgress(int videoId, float progress);
    }
}
