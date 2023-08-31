#include "com_kawayu_go_socket_android_TcpClient.h"
#include "scoped_jstring.h"
#include "net_core.hpp"
#include "log/zlogger.hpp"
#include <string>
#include <vector>

JNIEnv* getJniEnv(JavaVM *jvm, bool &needDetach) {
    JNIEnv *env = NULL;
    int stat = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6;
        args.name = NULL;
        args.group = NULL;
        jvm->AttachCurrentThread(&env, &args);
        needDetach = true;
    }
    return env;
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_initHostAndPort
  (JNIEnv *env, jclass cls, jstring host, jstring backupIp, jint port, jboolean isTls, jobject listener) {
      zcdebug2("initHostAndPort");
      ScopedJstring host_jstr(env, host);
      ScopedJstring backupIp_jstr(env, backupIp);
      JavaVM *jvm;
      env->GetJavaVM(&jvm);
      jobject gListener = env->NewGlobalRef(listener);
      NetCore::InitHostAndPort(host_jstr.GetChar(), backupIp_jstr.GetChar(), (int)port, (bool)isTls, [jvm, gListener]()->std::string{
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID getConnectInfo = newEnv->GetMethodID(listenerClassRef, "getConnectInfo", "()Ljava/lang/String;");
          jstring connectInfo = (jstring)newEnv->CallObjectMethod(gListener, getConnectInfo);
          ScopedJstring *connect_info_jstr = new ScopedJstring(newEnv, connectInfo);
          std::string cConnectInfo = std::string(connect_info_jstr->SafeGetChar());
          delete connect_info_jstr;
          if (needDetach) {
              jvm->DetachCurrentThread();
          }
          return cConnectInfo;
      });
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_makeSureConnected
  (JNIEnv *, jclass) {
      zcdebug2("makeSureConnected");
      NetCore::MakeSureConnected();
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_disconnectAsync
  (JNIEnv *env, jclass cls, jint code, jboolean shouldReconnect) {
      zcdebug2("disconnectAsync");
      NetCore::DisconnectAsync((DisconnectCode)code, (bool)shouldReconnect);
}

JNIEXPORT jboolean JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_send
  (JNIEnv *env, jclass cls, jstring payloadType, jstring payload, jbyteArray data, jobject listener) {
      ScopedJstring payload_type_jstr(env, payloadType);
      ScopedJstring payload_jstr(env, payload);
      JavaVM *jvm;
      env->GetJavaVM(&jvm);
      jobject gListener = env->NewGlobalRef(listener);

      auto callback = [jvm, gListener](int errorCode, std::string response){
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID onSendResp = newEnv->GetMethodID(listenerClassRef, "onSendResp", "(ILjava/lang/String;)V");

          ScopedJstring *response_jstr = new ScopedJstring(newEnv, response.c_str());
          newEnv->CallVoidMethod(gListener, onSendResp, (jint)errorCode, response_jstr->GetJstr());
          newEnv->DeleteGlobalRef(gListener);
          delete response_jstr;
          if (needDetach) {
              jvm->DetachCurrentThread();
          }
      };
      if (data == NULL) {
          return (jboolean)NetCore::Send(payload_type_jstr.GetChar(), payload_jstr.GetChar(), callback);
      }else {
          int len = env->GetArrayLength(data);
          char cData[len];
          env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte*>(cData));
          return (jboolean)NetCore::Send(payload_type_jstr.GetChar(), payload_jstr.GetChar(), cData, len, callback);
      }
}

JNIEXPORT jboolean JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_sendNoReply
  (JNIEnv *env, jclass cls, jstring payloadType, jstring payload) {
      ScopedJstring payload_type_jstr(env, payloadType);
      ScopedJstring payload_jstr(env, payload);
      return (jboolean)NetCore::SendNoReply(payload_type_jstr.GetChar(), payload_jstr.GetChar());
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_setOnConnStatusChange
  (JNIEnv *env, jclass cls, jobject listener) {
      JavaVM *jvm;
      env->GetJavaVM(&jvm);
      jobject gListener = env->NewGlobalRef(listener);
      NetCore::SetOnConnStatusChange([jvm, gListener](ConnectStatus status){
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID onConnStatusChange = newEnv->GetMethodID(listenerClassRef, "onConnStatusChange", "(I)V");
          newEnv->CallVoidMethod(gListener, onConnStatusChange, (jint)status);
          if (needDetach) {
              jvm->DetachCurrentThread();
          }
      });
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_setOnSendReq
  (JNIEnv *env, jclass cls, jobject listener) {
      JavaVM *jvm;
      env->GetJavaVM(&jvm);

      jobject gListener = env->NewGlobalRef(listener);
      NetCore::SetOnSendReq([jvm, gListener](std::string payloadType, std::string payload){
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID onSendReq = newEnv->GetMethodID(listenerClassRef, "onSendReq", "(Ljava/lang/String;Ljava/lang/String;)V");

          ScopedJstring *payloadType_jstr = new ScopedJstring(newEnv, payloadType.c_str());
          ScopedJstring *payload_jstr = new ScopedJstring(newEnv, payload.c_str());
          newEnv->CallVoidMethod(gListener, onSendReq, payloadType_jstr->GetJstr(), payload_jstr->GetJstr());
          delete payloadType_jstr;
          delete payload_jstr;
          if (needDetach) {
              jvm->DetachCurrentThread();
          }
      });
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_setOnServerDisconnect
  (JNIEnv *env, jclass cls, jobject listener) {
      JavaVM *jvm;
      env->GetJavaVM(&jvm);
      jobject gListener = env->NewGlobalRef(listener);
      NetCore::SetOnServerDisconnect([jvm, gListener](SvrDisCode disCode){
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID onServerDisconnect = newEnv->GetMethodID(listenerClassRef, "onServerDisconnect", "(I)V");
          newEnv->CallVoidMethod(gListener, onServerDisconnect, (jint)disCode);
          if (needDetach) {
              jvm->DetachCurrentThread();
          }
      });
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_setHttpDnsListener
  (JNIEnv *env, jclass cls, jobject listener) {
      JavaVM *jvm;
      env->GetJavaVM(&jvm);
      jobject gListener = env->NewGlobalRef(listener);
      NetCore::SetHttpDns([jvm, gListener](std::string host)->std::vector<std::string>{
          bool needDetach = false;
          JNIEnv *newEnv = getJniEnv(jvm, needDetach);
          ScopedJstring *host_jstr = new ScopedJstring(newEnv, host.c_str());
          jclass listenerClassRef = newEnv->GetObjectClass(gListener);
          jmethodID getHttpDns = newEnv->GetMethodID(listenerClassRef, "getHttpDns", "(Ljava/lang/String;)[Ljava/lang/String;");
          jobjectArray result = (jobjectArray)newEnv->CallObjectMethod(gListener, getHttpDns, host_jstr->GetJstr());
          jvm->AttachCurrentThread(&newEnv, NULL);
          jsize size = newEnv->GetArrayLength(result);
          //将Java的String[]转换成C++的std::vector<std::string>
          std::vector<std::string> ips;
          for (int i = 0; i < size; i ++) {
              jstring obj = (jstring)newEnv->GetObjectArrayElement(result,i);
              if (obj == NULL) {
                  continue;
              }
              std::string ip = (std::string)newEnv->GetStringUTFChars(obj,NULL);
              ips.insert(ips.end(), ip);
          }
          jvm->DetachCurrentThread();
          return ips;
      });
}

JNIEXPORT void JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_setNetType
  (JNIEnv *env, jclass cls, jint netType) {
      NetCore::SetNetType((NetType)netType);
}

JNIEXPORT jint JNICALL Java_com_kawayu_go_1socket_1android_TcpClient_getConnectStatus
  (JNIEnv *env, jclass cls) {
      return (jint)NetCore::GetConnectStatus();
}
