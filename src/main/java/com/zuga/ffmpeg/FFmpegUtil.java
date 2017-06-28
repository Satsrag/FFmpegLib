package com.zuga.ffmpeg;

public class FFmpegUtil {

    static {
        System.loadLibrary("FFmpegLib");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * //     * which is packaged with this application.
     * //
     */
    public static native int getThumbs(String inputFile, String outputFile, int totalTime);

    public static native int getThumb(String inputFile, String outputFile, String time);
}
