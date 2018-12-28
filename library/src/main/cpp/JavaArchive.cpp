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

#include "JavaArchive.h"

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
    case E_UNKNOWN_FORMAT:
      return "Unknown archive format.";
    default:
      return "Unknown error.";
  }
}

#define THROW_ARCHIVE_EXCEPTION_RETURN(ENV, CODE)                                         \
  do {                                                                                    \
    ThrowException(ENV, "com/hippo/a7zip/ArchiveException", GetMessageForCode(CODE));     \
    return;                                                                               \
  } while (0)

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
    jlong nativePtr
) {
  CHECK_CLOSED_RETURN_VALUE(env, nativePtr, nullptr);
  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);
  return env->NewStringUTF(archive->GetFormatName());
}

jint a7zip_NativeGetNumberOfEntries(
    JNIEnv* env,
    jclass,
    jlong nativePtr
) {
  CHECK_CLOSED_RETURN_VALUE(env, nativePtr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);

  UInt32 number;
  HRESULT result = archive->GetNumberOfEntries(number);
  if (result != S_OK) {
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, 0);
  }

  return number;
}

static void shrink(BSTR bstr) {
  jchar* jstr = reinterpret_cast<jchar*>(bstr);
  UINT n = ::SysStringLen(bstr);
  for (UINT i = 0; i < n; i++) {
    jstr[i] = static_cast<jchar>(bstr[i]);
  }
  jstr[n] = 0;
}

jstring a7zip_NativeGetArchiveStringProperty(
    JNIEnv* env,
    jclass,
    jlong native_ptr,
    jint prop_id
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

  BSTR path = nullptr;
  HRESULT result = archive->GetArchiveStringProperty(static_cast<PROPID>(prop_id), &path);
  if (result != S_OK) {
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, nullptr);
  }

  if (path == nullptr) {
    return nullptr;
  }

  shrink(path);
  jstring jstr = env->NewString(reinterpret_cast<const jchar*>(path), ::SysStringLen(path));
  ::SysFreeString(path);
  return jstr;
}

jstring a7zip_NativeGetEntryStringProperty(
    JNIEnv* env,
    jclass,
    jlong native_ptr,
    jint index,
    jint prop_id
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

  BSTR path = nullptr;
  HRESULT result = archive->GetEntryStringProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &path);
  if (result != S_OK) {
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, nullptr);
  }

  if (path == nullptr) {
    return nullptr;
  }

  shrink(path);
  jstring jstr = env->NewString(reinterpret_cast<const jchar*>(path), ::SysStringLen(path));
  ::SysFreeString(path);
  return jstr;
}

jstring a7zip_NativeGetEntryPath(
    JNIEnv* env,
    jclass,
    jlong nativePtr,
    jint index
) {
  CHECK_CLOSED_RETURN_VALUE(env, nativePtr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);

  BSTR path = nullptr;
  HRESULT result = archive->GetEntryPath(static_cast<UInt32>(index), &path);
  if (result != S_OK || path == nullptr) {
    if (path != nullptr) ::SysFreeString(path);
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, nullptr);
  }

  shrink(path);
  jstring jstr = env->NewString(reinterpret_cast<const jchar*>(path), ::SysStringLen(path));
  ::SysFreeString(path);
  return jstr;
}

void a7zip_NativeClose(
    JNIEnv* env,
    jclass,
    jlong nativePtr
) {
  CHECK_CLOSED_RETURN(env, nativePtr);
  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);
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
    { "nativeGetArchiveStringProperty",
      "(JI)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveStringProperty) },
    { "nativeGetEntryStringProperty",
      "(JII)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetEntryStringProperty) },
    { "nativeGetEntryPath",
      "(JI)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetEntryPath) },
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

HRESULT JavaArchive::Initialize(JNIEnv* env) {
  RETURN_SAME_IF_NOT_ZERO(RegisterMethods(env));
  return S_OK;
}
