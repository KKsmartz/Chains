#include <jni.h>
#include <android/log.h>
#include <string>
#include "map/core/PrimitiveDrawer.h"

void nativeInit(JNIEnv *env, jobject /* this */) {
    // 在这里进行初始化操作
    __android_log_print(ANDROID_LOG_INFO, "SmartzMap", "Native init called");

    // 例如，加载地图数据或设置渲染参数等
    // 这里只是一个示例，实际的初始化逻辑需要根据需求实现
    std::string initMessage = "Smart Renderer Initialized";
    __android_log_print(ANDROID_LOG_INFO, "SmartzMap", "%s", initMessage.c_str());
    drawLine(0.0f, 0.0f, 1.0f, 1.0f, 0xFFFF0000); // 绘制一条红色线段
}

jint registerNativeMethods(JNIEnv *env) {
    jclass clazz = env->FindClass("com/kit/smartz_map/SmartRenderer");
    if (clazz == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Failed to find class");
        return JNI_ERR; // 类未找到
    }

    JNINativeMethod methods[] = {
            {"nativeInit", "()V", (void *) nativeInit},
    };

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Failed to register native methods");
        return JNI_ERR; // 注册失败
    }

    return JNI_OK; // 成功
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Failed to get JNIEnv");
        return JNI_ERR; // 获取 JNIEnv 失败
    }

    // 注册本地方法
    if (registerNativeMethods(env) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Failed to register native methods");
        return JNI_ERR; // 注册失败
    }

    return JNI_VERSION_1_6; // 返回 JNI 版本
}

