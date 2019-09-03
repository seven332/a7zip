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

#ifndef __A7ZIP_JAVA_INPUT_ARCHIVE_H__
#define __A7ZIP_JAVA_INPUT_ARCHIVE_H__

#include <jni.h>

#include <Common/MyWindows.h>

namespace a7zip {
    namespace JavaInputArchive {

        HRESULT RegisterMethods(JNIEnv *env);

        HRESULT UnregisterMethods(JNIEnv *env);

    }
}

#endif //__A7ZIP_JAVA_INPUT_ARCHIVE_H__
