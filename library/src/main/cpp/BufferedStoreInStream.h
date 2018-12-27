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

#ifndef __A7ZIP_BUFFERED_STORE_IN_STREAM_H__
#define __A7ZIP_BUFFERED_STORE_IN_STREAM_H__

#include <jni.h>

#include <Common/MyCom.h>
#include <7zip/IStream.h>

namespace a7zip {

class BufferedStoreInStream:
    public IInStream,
    public IStreamGetSize,
    public CMyUnknownImp
{
 private:
  BufferedStoreInStream(JNIEnv* env, jobject store, jbyteArray array);

 public:
  virtual ~BufferedStoreInStream();

 public:
  MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

  STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition);

  STDMETHOD(GetSize)(UInt64* size);

 private:
  JNIEnv* env;
  jobject store;
  jbyteArray array;

 public:
  static HRESULT Initialize(JNIEnv* env);
  static HRESULT Create(JNIEnv* env, jobject store, BufferedStoreInStream** in_stream);

 private:
  static bool initialized;
  static jmethodID method_read;
  static jmethodID method_seek;
  static jmethodID method_tell;
  static jmethodID method_size;
  static jmethodID method_close;
};

}

#endif //__A7ZIP_BUFFERED_STORE_IN_STREAM_H__
