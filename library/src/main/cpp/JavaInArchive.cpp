/*
 * Copyright 2018 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JavaInArchive.h"

#include <cstdio>
#include <type_traits>

#include <include_windows/windows.h>
#include <7zip/Archive/IArchive.h>

#include "BufferedStoreInStream.h"
#include "Log.h"
#include "P7Zip.h"
#include "Utils.h"

#ifdef LOG_TAG
#  undef LOG_TAG
#endif //LOG_TAG
#define LOG_TAG "JavaArchive"

using namespace a7zip;

#define MAX_MSG_SIZE 1024

static jint ThrowExceptionInternal(
    JNIEnv* env,
    const char* exception_name,
    const char* message,
    va_list va_args
) {
  char formatted_message[MAX_MSG_SIZE];
  vsnprintf(formatted_message, MAX_MSG_SIZE, message, va_args);
  jclass exception_class = env->FindClass(exception_name);
  return env->ThrowNew(exception_class, formatted_message);
}

static jint ThrowException(
    JNIEnv* env,
    const char* exception_name,
    const char* message,
    ...
) {
  va_list va_args;
  va_start(va_args, message);
  jint result = ThrowExceptionInternal(env, exception_name, message, va_args);
  va_end(va_args);
  return result;
}

// const string
static const char* GetMessageForCode(HRESULT code) {
  switch (code) {
    case E_NOT_INITIALIZED:
      return "The module is not initialized.";
    case S_OK:
    case E_INTERNAL:
      return "a7zip is buggy.";
    case E_CLASS_NOT_FOUND:
      return "Can't find the class.";
    case E_METHOD_NOT_FOUND:
      return "Can't find the method.";
    case E_JAVA_EXCEPTION:
      return "Catch a java exception.";
    case E_FAILED_CONSTRUCT:
      return "Failed to create new class.";
    case E_FAILED_REGISTER:
      return "Failed to register methods.";
    case E_INCONSISTENT_PROP_TYPE:
      return "Inconsistent property type.";
    case E_EMPTY_PROP:
      return "Empty property.";
    case E_UNKNOWN_FORMAT:
      return "Unknown archive format.";
    default:
      return "Unknown error.";
  }
}

#define THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(ENV, CODE, VALUE)                            \
  do {                                                                                    \
    ThrowException(ENV, "com/hippo/a7zip/ArchiveException", GetMessageForCode(CODE));     \
    return VALUE;                                                                         \
  } while (0)

#define CHECK_CLOSED_RETURN(ENV, NATIVE_PTR)                                              \
  do {                                                                                    \
    if (NATIVE_PTR == 0) {                                                                \
      ThrowException(ENV, "java/lang/IllegalStateException", "This Archive is closed.");  \
      return;                                                                             \
    }                                                                                     \
  } while (0)

#define CHECK_CLOSED_RETURN_VALUE(ENV, NATIVE_PTR, VALUE)                                 \
  do {                                                                                    \
    if (NATIVE_PTR == 0) {                                                                \
      ThrowException(ENV, "java/lang/IllegalStateException", "This Archive is closed.");  \
      return VALUE;                                                                       \
    }                                                                                     \
  } while (0)

jlong a7zip_NativeCreate(
    JNIEnv* env,
    jclass,
    jobject store
) {
  CMyComPtr<IInStream> stream = nullptr;
  HRESULT result = BufferedStoreInStream::Create(env, store, stream);
  if (result != S_OK || stream == nullptr) {
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, 0);
  }

  InArchive* archive = nullptr;
  result = P7Zip::OpenArchive(stream, &archive);
  if (result != S_OK || archive == nullptr) {
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, 0);
  }

  return reinterpret_cast<jlong>(archive);
}

jstring a7zip_NativeGetFormatName(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, nullptr);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);
  return env->NewStringUTF(archive->GetFormatName());
}

jint a7zip_NativeGetNumberOfEntries(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

  UInt32 number = 0;
  HRESULT result = archive->GetNumberOfEntries(number);
  return result == S_OK ? number : -1;
}

#define GET_ARCHIVE_PROPERTY_START(METHOD_NAME, RETURN_TYPE)                              \
RETURN_TYPE METHOD_NAME(JNIEnv* env, jclass, jlong native_ptr, jint prop_id) {            \
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);                                          \
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

#define GET_ARCHIVE_PROPERTY_END                                                          \
}

#define GET_ENTRY_PROPERTY_START(METHOD_NAME, RETURN_TYPE)                                \
RETURN_TYPE METHOD_NAME(JNIEnv* env, jclass, jlong native_ptr, jint index, jint prop_id) {\
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);                                          \
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

#define GET_ENTRY_PROPERTY_END                                                            \
}

#define GET_PROPERTY_TYPE(GETTER)                                                         \
  PropType prop_type;                                                                     \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? prop_type : PT_UNKNOWN;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchivePropertyType, jint)
  GET_PROPERTY_TYPE(archive->GetArchivePropertyType(static_cast<PROPID>(prop_id), &prop_type))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryPropertyType, jint)
  GET_PROPERTY_TYPE(archive->GetEntryPropertyType(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &prop_type))
GET_ENTRY_PROPERTY_END

#define GET_BOOL_TYPE(GETTER)                                                             \
  bool bool_prop;                                                                         \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? bool_prop : false;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveBooleanProperty, jboolean)
  GET_BOOL_TYPE(archive->GetArchiveBoolProperty(static_cast<PROPID>(prop_id), &bool_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryBooleanProperty, jboolean)
  GET_BOOL_TYPE(archive->GetEntryBoolProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &bool_prop))
GET_ENTRY_PROPERTY_END

static void shrink(BSTR bstr) {
  jchar* jstr = reinterpret_cast<jchar*>(bstr);
  UINT n = ::SysStringLen(bstr);
  for (UINT i = 0; i < n; i++) {
    jstr[i] = static_cast<jchar>(bstr[i]);
  }
  jstr[n] = 0;
}

#define GET_STRING_PROPERTY(GETTER)                                                       \
  BSTR str_prop = nullptr;                                                                \
  HRESULT result = (GETTER);                                                              \
  if (result != S_OK || str_prop == nullptr) {                                            \
    if (str_prop != nullptr) ::SysFreeString(str_prop);                                   \
    return nullptr;                                                                       \
  }                                                                                       \
  shrink(str_prop);                                                                       \
  jstring jstr =                                                                          \
      env->NewString(reinterpret_cast<const jchar*>(str_prop), ::SysStringLen(str_prop)); \
  ::SysFreeString(str_prop);                                                              \
  return jstr;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveStringProperty, jstring)
  GET_STRING_PROPERTY(archive->GetArchiveStringProperty(static_cast<PROPID>(prop_id), &str_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryStringProperty, jstring)
  GET_STRING_PROPERTY(archive->GetEntryStringProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &str_prop))
GET_ENTRY_PROPERTY_END

void a7zip_NativeClose(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN(env, native_ptr);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);
  delete archive;
}

static JNINativeMethod archive_methods[] = {
    { "nativeCreate",
      "(Lokio/BufferedStore;)J",
      reinterpret_cast<void *>(a7zip_NativeCreate) },
    { "nativeGetFormatName",
      "(J)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetFormatName) },
    { "nativeGetNumberOfEntries",
      "(J)I",
      reinterpret_cast<void *>(a7zip_NativeGetNumberOfEntries) },
    { "nativeGetArchivePropertyType",
      "(JI)I",
      reinterpret_cast<void *>(a7zip_NativeGetArchivePropertyType) },
    { "nativeGetArchiveBooleanProperty",
      "(JI)Z",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveBooleanProperty) },
    { "nativeGetArchiveStringProperty",
      "(JI)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveStringProperty) },
    { "nativeGetEntryPropertyType",
      "(JII)I",
      reinterpret_cast<void *>(a7zip_NativeGetEntryPropertyType) },
    { "nativeGetEntryBooleanProperty",
      "(JII)Z",
      reinterpret_cast<void *>(a7zip_NativeGetEntryBooleanProperty) },
    { "nativeGetEntryStringProperty",
      "(JII)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetEntryStringProperty) },
    { "nativeClose",
      "(J)V",
      reinterpret_cast<void *>(a7zip_NativeClose) }
};

static HRESULT RegisterMethods(JNIEnv* env) {
  jclass clazz = env->FindClass("com/hippo/a7zip/InArchive");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  jint result = env->RegisterNatives(clazz, archive_methods, std::extent<decltype(archive_methods)>::value);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(env);
  if (result < 0) {
    return E_FAILED_REGISTER;
  }

  return S_OK;
}

HRESULT JavaInArchive::Initialize(JNIEnv* env) {
  RETURN_SAME_IF_NOT_ZERO(RegisterMethods(env));
  return S_OK;
}
