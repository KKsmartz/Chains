#include <jni.h>
#include <string>
#include <queue>
#include <thread>
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
}

// 播放器状态
enum PlayerState {
    PLAYER_IDLE = 0,
    PLAYER_PREPARING,
    PLAYER_PREPARED,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_STOPPED,
    PLAYER_ERROR
};

// 全局变量
struct PlayerContext {
    // 播放状态
    PlayerState state;
    // 解封装上下文
    AVFormatContext* format_ctx;
    // 音频解码器上下文
    AVCodecContext* audio_codec_ctx;
    // 视频解码器上下文
    AVCodecContext* video_codec_ctx;
    // 音频流索引
    int audio_stream_index;
    // 视频流索引
    int video_stream_index;
    // 视频宽高
    int video_width;
    int video_height;
    // Surface
    ANativeWindow* native_window;
    // 播放线程
    std::thread decode_thread;
    // 音频队列
    std::queue<AVFrame*> audio_frame_queue;
    // 视频队列
    std::queue<AVFrame*> video_frame_queue;
    // 停止标志
    bool stop_request;
};

static PlayerContext* player_ctx = nullptr;

// 初始化播放器
extern "C" JNIEXPORT void JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeInit(JNIEnv* env, jobject thiz) {
    if (!player_ctx) {
        player_ctx = new PlayerContext();
        player_ctx->state = PLAYER_IDLE;
        player_ctx->format_ctx = nullptr;
        player_ctx->audio_codec_ctx = nullptr;
        player_ctx->video_codec_ctx = nullptr;
        player_ctx->audio_stream_index = -1;
        player_ctx->video_stream_index = -1;
        player_ctx->native_window = nullptr;
        player_ctx->stop_request = false;
    }
}

// 设置播放源
extern "C" JNIEXPORT jint JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeSetDataSource(JNIEnv* env, jobject thiz, jstring path) {
    if (!player_ctx) return -1;

    const char* input_path = env->GetStringUTFChars(path, nullptr);
    int ret = avformat_open_input(&player_ctx->format_ctx, input_path, nullptr, nullptr);
    env->ReleaseStringUTFChars(path, input_path);

    if (ret < 0) {
        player_ctx->state = PLAYER_ERROR;
        return -1;
    }

    if (avformat_find_stream_info(player_ctx->format_ctx, nullptr) < 0) {
        player_ctx->state = PLAYER_ERROR;
        return -1;
    }

    // 查找音视频流
    for (unsigned int i = 0; i < player_ctx->format_ctx->nb_streams; i++) {
        if (player_ctx->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
            player_ctx->audio_stream_index == -1) {
            player_ctx->audio_stream_index = i;
        }
        if (player_ctx->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            player_ctx->video_stream_index == -1) {
            player_ctx->video_stream_index = i;
        }
    }

    // 初始化解码器
    if (player_ctx->video_stream_index >= 0) {
        AVStream* video_stream = player_ctx->format_ctx->streams[player_ctx->video_stream_index];
        const AVCodec* video_codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
        player_ctx->video_codec_ctx = avcodec_alloc_context3(video_codec);
        avcodec_parameters_to_context(player_ctx->video_codec_ctx, video_stream->codecpar);
        avcodec_open2(player_ctx->video_codec_ctx, video_codec, nullptr);

        player_ctx->video_width = player_ctx->video_codec_ctx->width;
        player_ctx->video_height = player_ctx->video_codec_ctx->height;
    }

    if (player_ctx->audio_stream_index >= 0) {
        AVStream* audio_stream = player_ctx->format_ctx->streams[player_ctx->audio_stream_index];
        const AVCodec* audio_codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
        player_ctx->audio_codec_ctx = avcodec_alloc_context3(audio_codec);
        avcodec_parameters_to_context(player_ctx->audio_codec_ctx, audio_stream->codecpar);
        avcodec_open2(player_ctx->audio_codec_ctx, audio_codec, nullptr);
    }

    player_ctx->state = PLAYER_PREPARED;
    return 0;
}

// 设置显示Surface
extern "C" JNIEXPORT void JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeSetSurface(JNIEnv* env, jobject thiz, jobject surface) {
    if (!player_ctx) return;

    if (player_ctx->native_window) {
        ANativeWindow_release(player_ctx->native_window);
        player_ctx->native_window = nullptr;
    }

    if (surface) {
        player_ctx->native_window = ANativeWindow_fromSurface(env, surface);
        if (player_ctx->native_window) {
            ANativeWindow_setBuffersGeometry(player_ctx->native_window,
                                           player_ctx->video_width,
                                           player_ctx->video_height,
                                           WINDOW_FORMAT_RGBA_8888);
        }
    }
}

// 开始播放
extern "C" JNIEXPORT void JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeStart(JNIEnv* env, jobject thiz) {
    if (!player_ctx || player_ctx->state != PLAYER_PREPARED) return;

    player_ctx->stop_request = false;
    player_ctx->state = PLAYER_PLAYING;

    // 启动解码线程
    player_ctx->decode_thread = std::thread([]{
        AVPacket* packet = av_packet_alloc();
        while (!player_ctx->stop_request) {
            if (av_read_frame(player_ctx->format_ctx, packet) < 0) {
                break;
            }

            if (packet->stream_index == player_ctx->video_stream_index) {
                // 视频解码
                avcodec_send_packet(player_ctx->video_codec_ctx, packet);
                AVFrame* frame = av_frame_alloc();
                while (avcodec_receive_frame(player_ctx->video_codec_ctx, frame) == 0) {
                    player_ctx->video_frame_queue.push(frame);
                    frame = av_frame_alloc();
                }
                av_frame_free(&frame);
            }
            else if (packet->stream_index == player_ctx->audio_stream_index) {
                // 音频解码
                avcodec_send_packet(player_ctx->audio_codec_ctx, packet);
                AVFrame* frame = av_frame_alloc();
                while (avcodec_receive_frame(player_ctx->audio_codec_ctx, frame) == 0) {
                    player_ctx->audio_frame_queue.push(frame);
                    frame = av_frame_alloc();
                }
                av_frame_free(&frame);
            }

            av_packet_unref(packet);
        }
        av_packet_free(&packet);
    });
}

// 停止播放
extern "C" JNIEXPORT void JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeStop(JNIEnv* env, jobject thiz) {
    if (!player_ctx) return;

    player_ctx->stop_request = true;
    if (player_ctx->decode_thread.joinable()) {
        player_ctx->decode_thread.join();
    }

    // 清理资源
    while (!player_ctx->audio_frame_queue.empty()) {
        av_frame_free(&player_ctx->audio_frame_queue.front());
        player_ctx->audio_frame_queue.pop();
    }
    while (!player_ctx->video_frame_queue.empty()) {
        av_frame_free(&player_ctx->video_frame_queue.front());
        player_ctx->video_frame_queue.pop();
    }

    player_ctx->state = PLAYER_STOPPED;
}

// 释放播放器
extern "C" JNIEXPORT void JNICALL
Java_com_smartz_player_FFmpegPlayer_nativeRelease(JNIEnv* env, jobject thiz) {
    if (!player_ctx) return;

    if (player_ctx->state == PLAYER_PLAYING) {
        Java_com_smartz_player_FFmpegPlayer_nativeStop(env, thiz);
    }

    if (player_ctx->video_codec_ctx) {
        avcodec_free_context(&player_ctx->video_codec_ctx);
    }
    if (player_ctx->audio_codec_ctx) {
        avcodec_free_context(&player_ctx->audio_codec_ctx);
    }
    if (player_ctx->format_ctx) {
        avformat_close_input(&player_ctx->format_ctx);
    }
    if (player_ctx->native_window) {
        ANativeWindow_release(player_ctx->native_window);
    }

    delete player_ctx;
    player_ctx = nullptr;
}
