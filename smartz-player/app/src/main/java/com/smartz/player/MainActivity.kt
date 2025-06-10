package com.smartz.player

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.open.smartz_player.R
import java.io.File

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        // 显示 FFmpeg 版本号
        findViewById<TextView>(R.id.tv_version).text = "FFmpeg 版本: ${FFmpegHelper.getFFmpegVersion()}"

        // 设置播放按钮点击事件
        findViewById<Button>(R.id.btn_play).setOnClickListener {
            val videoPath = "${getExternalFilesDir(null)}/test.mp4"
            if (File(videoPath).exists()) {
                startActivity(Intent(this, PlayerActivity::class.java).apply {
                    putExtra("video_path", videoPath)
                })
            } else {
                // 提示用户需要先拷贝测试视频到指定目录
                android.app.AlertDialog.Builder(this)
                    .setTitle("提示")
                    .setMessage("请先将测试视频 test.mp4 拷贝到：\n$videoPath")
                    .setPositiveButton("确定", null)
                    .show()
            }
        }
    }
}
