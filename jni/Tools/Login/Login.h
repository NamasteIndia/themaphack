#include <jni.h>
#include <string>
#include <curl/curl.h>
#include "json.hpp" // for nlohmann::json

using json = nlohmann::json;

std::string g_Token;
std::string g_Auth;

std::string GetAndroidID(JNIEnv* env, jobject application) {
    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getContentResolver = env->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");

    jobject contentResolver = env->CallObjectMethod(application, getContentResolver);

    jclass secureClass = env->FindClass("android/provider/Settings$Secure");
    jmethodID getString = env->GetStaticMethodID(secureClass, "getString",
        "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");

    jstring androidIdStr = env->NewStringUTF("android_id");
    jstring androidId = (jstring)env->CallStaticObjectMethod(secureClass, getString, contentResolver, androidIdStr);

    const char* androidIdChars = env->GetStringUTFChars(androidId, nullptr);
    std::string result(androidIdChars);
    env->ReleaseStringUTFChars(androidId, androidIdChars);

    env->DeleteLocalRef(androidIdStr);
    env->DeleteLocalRef(androidId);
    env->DeleteLocalRef(secureClass);
    env->DeleteLocalRef(contentResolver);
    env->DeleteLocalRef(contextClass);

    return result;
}

std::string GetDeviceModel(JNIEnv* env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID modelField = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
    jstring model = (jstring)env->GetStaticObjectField(buildClass, modelField);

    const char* modelChars = env->GetStringUTFChars(model, nullptr);
    std::string result(modelChars);
    env->ReleaseStringUTFChars(model, modelChars);

    env->DeleteLocalRef(model);
    env->DeleteLocalRef(buildClass);

    return result;
}

std::string GetDeviceBrand(JNIEnv* env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID brandField = env->GetStaticFieldID(buildClass, "BRAND", "Ljava/lang/String;");
    jstring brand = (jstring)env->GetStaticObjectField(buildClass, brandField);

    const char* brandChars = env->GetStringUTFChars(brand, nullptr);
    std::string result(brandChars);
    env->ReleaseStringUTFChars(brand, brandChars);

    env->DeleteLocalRef(brand);
    env->DeleteLocalRef(buildClass);

    return result;
}

std::string GetDeviceManufacturer(JNIEnv* env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID manufacturerField = env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
    jstring manufacturer = (jstring)env->GetStaticObjectField(buildClass, manufacturerField);

    const char* manufacturerChars = env->GetStringUTFChars(manufacturer, nullptr);
    std::string result(manufacturerChars);
    env->ReleaseStringUTFChars(manufacturer, manufacturerChars);

    env->DeleteLocalRef(manufacturer);
    env->DeleteLocalRef(buildClass);

    return result;
}

std::string GetDeviceSerial(JNIEnv* env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID serialField = env->GetStaticFieldID(buildClass, "SERIAL", "Ljava/lang/String;");
    jstring serial = (jstring)env->GetStaticObjectField(buildClass, serialField);

    const char* serialChars = env->GetStringUTFChars(serial, nullptr);
    std::string result(serialChars);
    env->ReleaseStringUTFChars(serial, serialChars);

    env->DeleteLocalRef(serial);
    env->DeleteLocalRef(buildClass);

    return result;
}

std::string GetDeviceFingerPrint(JNIEnv* env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID fingerprintField = env->GetStaticFieldID(buildClass, "FINGERPRINT", "Ljava/lang/String;");
    jstring fingerprint = (jstring)env->GetStaticObjectField(buildClass, fingerprintField);

    const char* fingerprintChars = env->GetStringUTFChars(fingerprint, nullptr);
    std::string result(fingerprintChars);
    env->ReleaseStringUTFChars(fingerprint, fingerprintChars);

    env->DeleteLocalRef(fingerprint);
    env->DeleteLocalRef(buildClass);

    return result;
}

std::string GetSDKVersion(JNIEnv* env) {
    jclass versionClass = env->FindClass("android/os/Build$VERSION");
    jfieldID sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
    jint sdkInt = env->GetStaticIntField(versionClass, sdkIntField);

    env->DeleteLocalRef(versionClass);

    return std::to_string(sdkInt);
}

std::string GetDeviceID(JNIEnv* env) {
    // You can customize this if you want to get Android device ID differently
    return GetAndroidID(env, nullptr);
}

// Dummy implementation for UUID generation based on hwid
std::string GetDeviceUniqueIdentifier(JNIEnv* env, const char* hwid) {
    // For simplicity, just return the hwid string as UUID
    return std::string(hwid);
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Login(JavaVM* jvm, const char* user_key, bool* success) {
    *success = true;  // Always succeed immediately

    // Attach current thread for JNI calls
    JNIEnv* env = nullptr;
    if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        *success = false;
        return "Failed to attach current thread";
    }

    auto looperClass = env->FindClass("android/os/Looper");
    auto prepareMethod = env->GetStaticMethodID(looperClass, "prepare", "()V");
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

    // Bypass network call and token validation

    // Set global token and auth to dummy values to simulate login success
    g_Token = "dummy_token";
    g_Auth = "dummy_token";

    *success = true;

    return std::string(""); // No error message
}
