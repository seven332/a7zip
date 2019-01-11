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

jint JNI_OnLoad(JavaVM* vm, void*) {
  JNIEnv* env;

  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  JavaEnv::Initialize(vm);
  RETURN_JNI_ERR_IF_NOT_ZERO(JavaInArchive::Initialize(env));
  RETURN_JNI_ERR_IF_NOT_ZERO(InStream::Initialize(env));
  RETURN_JNI_ERR_IF_NOT_ZERO(SequentialOutStream::Initialize(env));
  RETURN_JNI_ERR_IF_NOT_ZERO(P7Zip::Initialize("libp7zip.so"));

  return JNI_VERSION_1_6;
}
