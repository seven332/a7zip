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

#include "InternalSeekableInputStream.h"
#include "JavaEnv.h"
#include "JavaInputArchive.h"
#include "JavaSeekableInputStream.h"
#include "JavaInputStream.h"
#include "InternalOutputStream.h"
#include "SevenZip.h"
#include "Utils.h"

using namespace a7zip;

jint JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env;

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    JavaEnv::Initialize(vm);
    RETURN_JNI_ERR_IF_NOT_ZERO(InternalSeekableInputStream::Initialize(env));
    RETURN_JNI_ERR_IF_NOT_ZERO(JavaInStream::Initialize(env));
    RETURN_JNI_ERR_IF_NOT_ZERO(JavaSequentialInStream::Initialize(env));
    RETURN_JNI_ERR_IF_NOT_ZERO(InternalOutputStream::Initialize(env));
    RETURN_JNI_ERR_IF_NOT_ZERO(SevenZip::Initialize());

    return JNI_VERSION_1_6;
}

#define EXPORT __attribute__ ((visibility ("default")))

extern "C" {
EXPORT int Initialize();
EXPORT int Terminate();
}

int Initialize() {
    JavaEnv env;
    RETURN_SAME_IF_NOT_ZERO(JavaInputArchive::RegisterMethods(static_cast<JNIEnv *>(env)));
    RETURN_SAME_IF_NOT_ZERO(JavaInStream::RegisterMethods(static_cast<JNIEnv *>(env)));
    RETURN_SAME_IF_NOT_ZERO(JavaSequentialInStream::RegisterMethods(static_cast<JNIEnv *>(env)));
    return S_OK;
}

int Terminate() {
    JavaEnv env;
    RETURN_SAME_IF_NOT_ZERO(JavaInputArchive::UnregisterMethods(static_cast<JNIEnv *>(env)));
    RETURN_SAME_IF_NOT_ZERO(JavaInStream::UnregisterMethods(static_cast<JNIEnv *>(env)));
    RETURN_SAME_IF_NOT_ZERO(JavaSequentialInStream::UnregisterMethods(static_cast<JNIEnv *>(env)));
    return S_OK;
}
