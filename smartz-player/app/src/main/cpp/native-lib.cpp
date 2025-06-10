#include <jni.h>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_smartz_player_FFmpegHelper_getFFmpegVersion(JNIEnv* env, jobject clazz) {
    return env->NewStringUTF(av_version_info());
}
