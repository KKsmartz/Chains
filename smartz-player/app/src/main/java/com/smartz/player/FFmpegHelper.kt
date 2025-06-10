package com.smartz.player

object FFmpegHelper {

    init {
        System.loadLibrary("smartzplayer")
    }

    // 获取 FFmpeg 版本信息
    external fun getFFmpegVersion(): String
}
