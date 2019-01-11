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

#include <jni.h>

#include "InStream.h"
#include "JavaEnv.h"
#include "JavaInArchive.h"
#include "SequentialOutStream.h"
#include "P7Zip.h"
#include "Utils.h"

using namespace a7zip;

#ifdef A7ZIP_EXTRACT
#ifdef A7ZIP_LITE
#define SEVEN_ZIP_LIBRARY_NAME "libp7zip-extract-lite.so"
#else
#define SEVEN_ZIP_LIBRARY_NAME "libp7zip-extract.so"
#endif // A7ZIP_LITE
#else
#ifdef A7ZIP_LITE
#define SEVEN_ZIP_LIBRARY_NAME "libp7zip-lite.so"
#else
#define SEVEN_ZIP_LIBRARY_NAME "libp7zip-full.so"
#endif // A7ZIP_LITE
#endif // A7ZIP_EXTRACT

jint JNI_OnLoad(JavaVM* vm, void*) {
  JNIEnv* env;

  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  JavaEnv::Initialize(vm);
  RETURN_JNI_ERR_IF_NOT_ZERO(InStream::Initialize(env));
  RETURN_JNI_ERR_IF_NOT_ZERO(SequentialOutStream::Initialize(env));
  RETURN_JNI_ERR_IF_NOT_ZERO(P7Zip::Initialize(SEVEN_ZIP_LIBRARY_NAME));

  return JNI_VERSION_1_6;
}

#define EXPORT __attribute__ ((visibility ("default")))

extern "C" {
  EXPORT int Initialize();
  EXPORT int Terminate();
}

int Initialize() {
  JavaEnv env;
  return JavaInArchive::RegisterMethods(static_cast<JNIEnv*>(env));
}

int Terminate() {
  JavaEnv env;
  return JavaInArchive::UnregisterMethods(static_cast<JNIEnv*>(env));
}
