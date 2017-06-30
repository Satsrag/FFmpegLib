package com.zuga.ffmpeg;

public class FFmpegUtil {

    static {
        System.loadLibrary("FFmpegLib");
    }

    public static native int getThumb(String inputFile, String outputFile, String time);
}
