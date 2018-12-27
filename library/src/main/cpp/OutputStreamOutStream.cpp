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

#include "OutputStreamOutStream.h"

#include "Utils.h"

#define ARRAY_SIZE DEFAULT_BUFFER_SIZE

using namespace a7zip;

bool OutStreamOutStream::initialized = false;
jmethodID OutStreamOutStream::method_write = nullptr;
jmethodID OutStreamOutStream::method_close = nullptr;

OutStreamOutStream::OutStreamOutStream(JNIEnv* env, jobject os, jbyteArray array) {
  this->env = env;
  this->os = os;
  this->array = array;
}

OutStreamOutStream::~OutStreamOutStream() {
  this->env->CallVoidMethod(this->os, method_close);
  CLEAR_IF_EXCEPTION_PENDING(this->env);

  this->env->DeleteGlobalRef(this->os);
  this->env->DeleteGlobalRef(this->array);
}

HRESULT OutStreamOutStream::Write(const void* data, UInt32 size, UInt32* processedSize) {
  if (processedSize != nullptr) {
    *processedSize = 0;
  }

  if (size == 0) {
    return S_OK;
  }

  // Make size not bigger than ARRAY_SIZE
  size = MIN(ARRAY_SIZE, size);

  // Copy data from native buffer to java buffer
  this->env->SetByteArrayRegion(this->array, 0, size, reinterpret_cast<const jbyte*>(data));
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  // Write data to sink
  this->env->CallObjectMethod(this->os, method_write, this->array, 0, size);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  if (processedSize != nullptr) {
    *processedSize = size;
  }

  return S_OK;
}

HRESULT OutStreamOutStream::Initialize(JNIEnv* env) {
  if (initialized) {
    return S_OK;
  }

  jclass clazz = env->FindClass("java/io/OutputStream");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  method_write = env->GetMethodID(clazz, "write", "([BII)V");
  if (method_write == nullptr) return E_METHOD_NOT_FOUND;
  method_close = env->GetMethodID(clazz, "close", "()V");
  if (method_close == nullptr) return E_METHOD_NOT_FOUND;

  initialized = true;
  return S_OK;
}

HRESULT OutStreamOutStream::Create(
    JNIEnv* env,
    jobject os,
    OutStreamOutStream** out_stream
) {
  if (!initialized) {
    return E_NOT_INITIALIZED;
  }

  jobject g_os = env->NewGlobalRef(os);
  if (g_os == nullptr) {
    return E_OUTOFMEMORY;
  }

  jbyteArray array = env->NewByteArray(ARRAY_SIZE);
  if (array == nullptr) {
    env->DeleteGlobalRef(g_os);
    return E_FAILED_CONSTRUCT;
  }

  jbyteArray g_array = static_cast<jbyteArray>(env->NewGlobalRef(array));
  if (g_array == nullptr) {
    env->DeleteGlobalRef(g_os);
    return E_OUTOFMEMORY;
  }

  *out_stream = new OutStreamOutStream(env, g_os, g_array);

  return S_OK;
}
