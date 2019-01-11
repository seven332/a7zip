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

#include "SequentialOutStream.h"

#include "JavaEnv.h"
#include "Utils.h"

#define ARRAY_SIZE DEFAULT_BUFFER_SIZE

using namespace a7zip;

bool SequentialOutStream::initialized = false;
jmethodID SequentialOutStream::method_write = nullptr;
jmethodID SequentialOutStream::method_close = nullptr;

SequentialOutStream::SequentialOutStream(jobject stream, jbyteArray array) {
  this->stream = stream;
  this->array = array;
}

SequentialOutStream::~SequentialOutStream() {
  JavaEnv env;
  if (!env.IsValid()) return;

  env->CallVoidMethod(stream, method_close);
  CLEAR_IF_EXCEPTION_PENDING(env);

  env->DeleteGlobalRef(stream);
  env->DeleteGlobalRef(array);
}

HRESULT SequentialOutStream::Write(const void* data, UInt32 size, UInt32* processedSize) {
  if (processedSize != nullptr) {
    *processedSize = 0;
  }

  if (size == 0) {
    return S_OK;
  }

  JavaEnv env;
  if (!env.IsValid()) return E_JAVA_EXCEPTION;

  // Make size not bigger than ARRAY_SIZE
  size = MIN(ARRAY_SIZE, size);

  // Copy data from native buffer to java buffer
  env->SetByteArrayRegion(array, 0, size, reinterpret_cast<const jbyte*>(data));
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(env);

  // Write data to sink
  env->CallVoidMethod(stream, method_write, array, 0, size);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(env);

  if (processedSize != nullptr) {
    *processedSize = size;
  }

  return S_OK;
}

HRESULT SequentialOutStream::Initialize(JNIEnv* env) {
  if (initialized) {
    return S_OK;
  }

  jclass clazz = env->FindClass("com/hippo/a7zip/SequentialOutStream");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  method_write = env->GetMethodID(clazz, "write", "([BII)V");
  if (method_write == nullptr) return E_METHOD_NOT_FOUND;
  method_close = env->GetMethodID(clazz, "close", "()V");
  if (method_close == nullptr) return E_METHOD_NOT_FOUND;

  initialized = true;
  return S_OK;
}

HRESULT SequentialOutStream::Create(
    JNIEnv* env,
    jobject stream,
    CMyComPtr<ISequentialOutStream>& out_stream
) {
  if (!initialized) {
    return E_NOT_INITIALIZED;
  }

  jobject g_stream = env->NewGlobalRef(stream);
  if (g_stream == nullptr) {
    return E_OUTOFMEMORY;
  }

  jbyteArray array = env->NewByteArray(ARRAY_SIZE);
  if (array == nullptr) {
    env->DeleteGlobalRef(g_stream);
    return E_FAILED_CONSTRUCT;
  }

  jbyteArray g_array = static_cast<jbyteArray>(env->NewGlobalRef(array));
  if (g_array == nullptr) {
    env->DeleteGlobalRef(g_stream);
    return E_OUTOFMEMORY;
  }

  out_stream = new SequentialOutStream(g_stream, g_array);

  return S_OK;
}
