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

#include "gtest/gtest.h"

TEST(Test, Pass)
{
  EXPECT_EQ(1, 1);
}

jint a7zip_native_test(
    JNIEnv* env,
    jclass,
    jstring log_file
) {
  const char* c_log_file = env->GetStringUTFChars(log_file, nullptr);

  std::string xml_path("--gtest_output=xml:");

  int argc = 2;
  const char* argv[3] = {"/a7zip-test", (xml_path + c_log_file).c_str(), nullptr};

  testing::InitGoogleTest(&argc, const_cast<char**>(argv));
  int result = RUN_ALL_TESTS();

  env->ReleaseStringUTFChars(log_file, c_log_file);

  return result;
}

static JNINativeMethod test_methods[] = {
    { "nativeTest",
      "(Ljava/lang/String;)I",
      (void*) a7zip_native_test }
};

jint JNI_OnLoad(JavaVM* vm, void*) {
  JNIEnv* env;

  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass clazz = env->FindClass("com/hippo/a7zip/NativeTest");

  env->RegisterNatives(clazz, test_methods, std::extent<decltype(test_methods)>::value);

  return JNI_VERSION_1_6;
}
