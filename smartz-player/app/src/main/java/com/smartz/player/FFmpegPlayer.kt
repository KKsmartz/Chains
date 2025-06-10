package com.smartz.player

import android.view.Surface

class FFmpegPlayer {
    companion object {
        init {
            System.loadLibrary("smartzplayer")
        }
    }

    private external fun nativeInit()
    private external fun nativeSetDataSource(path: String): Int
    private external fun nativeSetSurface(surface: Surface?)
    private external fun nativeStart()
    private external fun nativeStop()
    private external fun nativeRelease()

    init {
        nativeInit()
    }

    fun setDataSource(path: String): Int {
        return nativeSetDataSource(path)
    }

    fun setSurface(surface: Surface?) {
        nativeSetSurface(surface)
    }

    fun start() {
        nativeStart()
    }

    fun stop() {
        nativeStop()
    }

    fun release() {
        nativeRelease()
    }
}
