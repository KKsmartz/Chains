package com.kit.smartz_map

import android.content.Context
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class SmartGLSurfaceView(context: Context?, attrs: AttributeSet?) :
    GLSurfaceView(context, attrs) {

    init {
        // Set the OpenGL ES version to 3.0
        setEGLContextClientVersion(3)
        // Set the renderer for drawing on the GLSurfaceView
        setRenderer(SmartRenderer())
        // Render the view only when there is a change in the drawing data
        renderMode = RENDERMODE_WHEN_DIRTY
    }
}

class SmartRenderer : GLSurfaceView.Renderer {
    init {
        System.loadLibrary("smartz_map") // Load the native library
    }

    override fun onSurfaceCreated(gl: javax.microedition.khronos.opengles.GL10?, config: javax.microedition.khronos.egl.EGLConfig?) {
        // Initialize OpenGL settings here
        GLES30.glClearColor(1.0f, 0.0f, 0.0f, 1.0f) // Set clear color to black
    }

    override fun onDrawFrame(gl: javax.microedition.khronos.opengles.GL10?) {
        // Draw the frame here
    }

    override fun onSurfaceChanged(gl: javax.microedition.khronos.opengles.GL10?, width: Int, height: Int) {
        // Handle surface changes here
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT or GLES30.GL_DEPTH_BUFFER_BIT)
        nativeInit()
    }

    external fun nativeInit() // Native method to initialize OpenGL resources
}