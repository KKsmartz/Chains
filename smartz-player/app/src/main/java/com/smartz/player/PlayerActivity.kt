package com.smartz.player

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.open.smartz_player.R
import java.io.File

class PlayerActivity : AppCompatActivity(), SurfaceHolder.Callback {
    private val REQUEST_PERMISSION = 1
    private var player: FFmpegPlayer? = null
    private var surfaceView: SurfaceView? = null
    private var videoPath: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_player)

        surfaceView = findViewById(R.id.surface_view)
        surfaceView?.holder?.addCallback(this)

        findViewById<Button>(R.id.btn_play).setOnClickListener {
            checkPermissionAndPlay()
        }

        findViewById<Button>(R.id.btn_stop).setOnClickListener {
            player?.stop()
        }

        // 初始化播放器
        player = FFmpegPlayer()
    }

    private fun checkPermissionAndPlay() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
            != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                REQUEST_PERMISSION
            )
        } else {
            startPlay()
        }
    }

    private fun startPlay() {
        // 这里使用一个示例视频路径，实际使用时需要替换为真实的视频路径
        videoPath = "${getExternalFilesDir(null)}/test.mp4"
        if (!File(videoPath!!).exists()) {
            Toast.makeText(this, "视频文件不存在", Toast.LENGTH_SHORT).show()
            return
        }

        val ret = player?.setDataSource(videoPath!!) ?: -1
        if (ret != 0) {
            Toast.makeText(this, "设置视频源失败", Toast.LENGTH_SHORT).show()
            return
        }

        player?.start()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        player?.setSurface(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        player?.setSurface(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        player?.setSurface(null)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_PERMISSION) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                startPlay()
            } else {
                Toast.makeText(this, "需要存储权限才能播放视频", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        player?.release()
        player = null
    }
}
