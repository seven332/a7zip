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

#include "BufferedStoreInStream.h"
#include "Log.h"
#include "Utils.h"

#define ARRAY_SIZE DEFAULT_BUFFER_SIZE

using namespace a7zip;

bool BufferedStoreInStream::initialized = false;
jmethodID BufferedStoreInStream::method_read = nullptr;
jmethodID BufferedStoreInStream::method_seek = nullptr;
jmethodID BufferedStoreInStream::method_tell = nullptr;
jmethodID BufferedStoreInStream::method_size = nullptr;
jmethodID BufferedStoreInStream::method_close = nullptr;

BufferedStoreInStream::BufferedStoreInStream(JNIEnv* env, jobject store, jbyteArray array) {
  this->env = env;
  this->store = store;
  this->array = array;
}

BufferedStoreInStream::~BufferedStoreInStream() {
  this->env->CallVoidMethod(this->store, method_close);
  CLEAR_IF_EXCEPTION_PENDING(this->env);

  this->env->DeleteGlobalRef(this->store);
  this->env->DeleteGlobalRef(this->array);
}

HRESULT BufferedStoreInStream::Read(void* data, UInt32 size, UInt32* processedSize) {
  if (processedSize != nullptr) {
    *processedSize = 0;
  }

  if (size == 0) {
    return S_OK;
  }

  // Make size not bigger than ARRAY_SIZE
  size = MIN(ARRAY_SIZE, size);

  jint read = this->env->CallIntMethod(this->store, method_read, this->array, 0, size);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  // Check EOF
  if (read <= 0) {
    return S_OK;
  }

  this->env->GetByteArrayRegion(this->array, 0, read, static_cast<jbyte*>(data));
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  if (processedSize != nullptr) {
    *processedSize = static_cast<UInt32>(read);
  }

  return S_OK;
}

HRESULT BufferedStoreInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) {
  jlong actual_offset;

  switch (seekOrigin) {
    case STREAM_SEEK_SET: {
      actual_offset = static_cast<jlong>(offset);
      break;
    }
    case STREAM_SEEK_CUR: {
      jlong position = this->env->CallLongMethod(this->store, method_tell);
      RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);
      actual_offset = position + offset;
      break;
    }
    case STREAM_SEEK_END: {
      jlong size = this->env->CallLongMethod(this->store, method_size);
      RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);
      actual_offset = size + offset;
      break;
    }
    default: {
      return E_INVALIDARG;
    }
  }

  if (actual_offset < 0) {
    return E_INVALIDARG;
  }

  this->env->CallVoidMethod(this->store, method_seek, actual_offset);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  if (newPosition != nullptr) {
    *newPosition = static_cast<UInt64>(actual_offset);
  }

  return S_OK;
}

HRESULT BufferedStoreInStream::GetSize(UInt64* size) {
  jlong store_size = this->env->CallLongMethod(this->store, method_size);
  RETURN_E_JAVA_EXCEPTION_IF_EXCEPTION_PENDING(this->env);

  if (size != nullptr) {
    *size = static_cast<UInt64>(store_size);
  }

  return S_OK;
}

HRESULT BufferedStoreInStream::Initialize(JNIEnv* env) {
  if (initialized) {
    return S_OK;
  }

  jclass clazz = env->FindClass("okio/BufferedStore");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  method_read = env->GetMethodID(clazz, "read", "([BII)I");
  if (method_read == nullptr) return E_METHOD_NOT_FOUND;
  method_seek = env->GetMethodID(clazz, "seek", "(J)V");
  if (method_seek == nullptr) return E_METHOD_NOT_FOUND;
  method_tell = env->GetMethodID(clazz, "tell", "()J");
  if (method_tell == nullptr) return E_METHOD_NOT_FOUND;
  method_size = env->GetMethodID(clazz, "size", "()J");
  if (method_size == nullptr) return E_METHOD_NOT_FOUND;
  method_close = env->GetMethodID(clazz, "close", "()V");
  if (method_close == nullptr) return E_METHOD_NOT_FOUND;

  initialized = true;
  return JNI_OK;
}

HRESULT BufferedStoreInStream::Create(
    JNIEnv* env,
    jobject store,
    BufferedStoreInStream** in_stream
) {
  if (!initialized) {
    return E_NOT_INITIALIZED;
  }

  jobject g_store = env->NewGlobalRef(store);
  if (g_store == nullptr) {
    return E_OUTOFMEMORY;
  }

  jbyteArray array = env->NewByteArray(ARRAY_SIZE);
  if (array == nullptr) {
    env->DeleteGlobalRef(g_store);
    return E_FAILED_CONSTRUCT;
  }

  jbyteArray g_array = static_cast<jbyteArray>(env->NewGlobalRef(array));
  if (g_array == nullptr) {
    env->DeleteGlobalRef(g_store);
    return E_OUTOFMEMORY;
  }

  *in_stream = new BufferedStoreInStream(env, g_store, g_array);

  return S_OK;
}
