#pragma once
#include "curl/curl.h"
#include "Tools.h"
#include "json.hpp"

using json = nlohmann::ordered_json;

#include <jni.h>
#include <string>
#include <ctime>
#include <pthread.h>

std::string title, version;
std::string UUID;

static std::string slotZ = "";
static std::string inVip = "";
static std::string autoskillsZ = "";
static std::string expired = "";
static std::string EXP = "LOGIN FIRST";
static std::string name = "";
static std::string device = "";
static std::string status = "";
static std::string floating = "";
static std::string battleData = "";
static std::string clientManager = "";

std::string g_Token, g_Auth;
bool bValid = false;

bool bInitDone;

namespace SignIN
{
    static bool user;
}

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Stub implementations for device info functions.
// Replace these with your actual implementations.
std::string GetAndroidID(JNIEnv* env, jobject application) {
    return "android_id_stub";
}

std::string GetDeviceModel(JNIEnv* env) {
    return "device_model_stub";
}

std::string GetDeviceBrand(JNIEnv* env) {
    return "device_brand_stub";
}

std::string GetDeviceManufacturer(JNIEnv* env) {
    return "device_manufacturer_stub";
}

std::string GetDeviceSerial(JNIEnv* env) {
    return "device_serial_stub";
}

std::string GetDeviceFingerPrint(JNIEnv* env) {
    return "device_fingerprint_stub";
}

std::string GetSDKVersion(JNIEnv* env) {
    return "sdk_version_stub";
}

std::string GetDeviceID(JNIEnv* env) {
    return "device_id_stub";
}

std::string GetDeviceUniqueIdentifier(JNIEnv* env, const char* hwid) {
    return std::string(hwid);
}

// Modified Login function to bypass login and always succeed.
std::string Login(JavaVM* jvm, const char* user_key, bool* success) {
    JNIEnv* env = nullptr;
    if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        *success = false;
        return "Failed to attach current thread";
    }

    jclass looperClass = env->FindClass("android/os/Looper");
    jmethodID prepareMethod = env->GetStaticMethodID(looperClass, "prepare", "()V");
    env->CallStaticVoidMethod(looperClass, prepareMethod);

    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, "sCurrentActivityThread", "Landroid/app/ActivityThread;");
    jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);

    jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, "mInitialApplication", "Landroid/app/Application;");
    jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);

    // Compose hwid using user_key and device info
    std::string hwid = user_key ? user_key : "";
    hwid += GetAndroidID(env, mInitialApplication);
    hwid += GetDeviceModel(env);
    hwid += GetDeviceBrand(env);
    hwid += GetDeviceManufacturer(env);
    hwid += GetDeviceSerial(env);
    hwid += GetDeviceFingerPrint(env);
    hwid += GetSDKVersion(env);
    hwid += GetDeviceID(env);

    std::string UUID = GetDeviceUniqueIdentifier(env, hwid.c_str());

    jvm->DetachCurrentThread();

    // Bypass network call and token validation - always succeed
    g_Token = "bypass_token";
    g_Auth = "bypass_token";
    *success = true;

    // Set other globals to simulate logged-in state
    slotZ = "bypass_device";
    expired = "never";
    inVip = "true";
    autoskillsZ = "enabled";
    clientManager = "bypass_client";
    battleData = "true";

    return ""; // no error message
}

static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

bool download_file(std::string url, std::string path) {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

        FILE* file = fopen(path.c_str(), "wb");
        if (!file) {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode result = curl_easy_perform(curl);

        fclose(file);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (result == CURLE_OK) {
            return true;
        } else {
            return false;
        }
    }
    curl_global_cleanup();
    return false;
}
