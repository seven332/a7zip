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

#include <type_traits>

#include <include_windows/windows.h>
#include <7zip/Archive/IArchive.h>

#include "BufferedStoreInStream.h"
#include "Log.h"
#include "P7Zip.h"
#include "Utils.h"

#ifdef LOG_TAG
#  undef LOG_TAG
#  define LOG_TAG "JavaArchive"
#endif

using namespace a7zip;

jlong a7zip_NativeCreate(
    JNIEnv* env,
    jclass,
    jobject store
) {
  BufferedStoreInStream* stream = nullptr;
  HRESULT result = BufferedStoreInStream::Create(env, store, &stream);
  if (result != S_OK) {
    // TODO throw exception
    return 0;
  }
  if (stream == nullptr) {
    // TODO throw exception
    return 0;
  }

  CMyComPtr<IInStream> inStream(stream);
  InArchive* archive = nullptr;

  result = P7Zip::OpenArchive(stream, &archive);

  if (result == S_OK) {
    return reinterpret_cast<jlong>(archive);
  } else {
    // TODO throw exception
    return 0;
  }
}

jstring a7zip_NativeGetFormatName(
    JNIEnv* env,
    jclass,
    jlong nativePtr
) {
  if (nativePtr == 0) {
    // TODO throw exception
    return nullptr;
  }

  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);

  return env->NewStringUTF(archive->GetFormatName());
}

jint a7zip_NativeGetNumberOfEntries(
    JNIEnv*,
    jclass,
    jlong nativePtr
) {
  if (nativePtr == 0) {
    // TODO throw exception
    return 0;
  }

  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);

  UInt32 number;
  HRESULT result = archive->GetNumberOfEntries(number);

  if (result == S_OK) {
    return number;
  } else {
    // TODO throw exception
    return 0;
  }
}

static void shrink(BSTR bstr) {
  jchar* jstr = reinterpret_cast<jchar*>(bstr);
  UINT n = ::SysStringLen(bstr);
  for (UINT i = 0; i < n; i++) {
    jstr[i] = static_cast<jchar>(bstr[i]);
  }
  jstr[n] = 0;
}

jstring a7zip_NativeGetEntryPath(
    JNIEnv* env,
    jclass,
    jlong nativePtr,
    jint index
) {
  if (nativePtr == 0) {
    // TODO throw exception
    return 0;
  }

  InArchive* archive = reinterpret_cast<InArchive*>(nativePtr);

  BSTR path = nullptr;
  HRESULT result = archive->GetEntryPath(static_cast<UInt32>(index), &path);

  if (result == S_OK && path != nullptr) {
    shrink(path);
    jstring jstr = env->NewString(reinterpret_cast<const jchar*>(path), ::SysStringLen(path));
    ::SysFreeString(path);
    return jstr;
  }

  if (path != nullptr) {
    ::SysFreeString(path);
  }

  // TODO throw exception

  return nullptr;
}

void a7zip_NativeClose(
    JNIEnv*,
    jclass,
    jlong nativePtr
) {
  if (nativePtr == 0) {
    return;
  }

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
    { "nativeGetEntryPath",
      "(JI)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetEntryPath) },
    { "nativeClose",
      "(J)V",
      reinterpret_cast<void *>(a7zip_NativeClose) }
};

static HRESULT RegisterMethods(JNIEnv* env) {
  jclass clazz = env->FindClass("com/hippo/a7zip/Archive");
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
