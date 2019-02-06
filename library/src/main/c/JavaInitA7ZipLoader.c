/*
 * Copyright 2019 Hippo Seven
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

#include <dlfcn.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define MAX_MSG_SIZE 1024

static jint ThrowExceptionInternal(
    JNIEnv* env,
    const char* exception_name,
    const char* message,
    va_list va_args
) {
  char formatted_message[MAX_MSG_SIZE];
  vsnprintf(formatted_message, MAX_MSG_SIZE, message, va_args);
  jclass exception_class = (*env)->FindClass(env, exception_name);
  return (*env)->ThrowNew(env, exception_class, formatted_message);
}

static jint ThrowException(
    JNIEnv* env,
    const char* exception_name,
    const char* message,
    ...
) {
  va_list va_args;
  va_start(va_args, message);
  jint result = ThrowExceptionInternal(env, exception_name, message, va_args);
  va_end(va_args);
  return result;
}

#define THROW_EXCEPTION(ENV, ...)                                     \
  ThrowException(ENV, "java/lang/UnsatisfiedLinkError", __VA_ARGS__)

typedef int (*InitializeFunc)();
typedef int (*TerminateFunc)();

typedef struct {
  void* handle;
  TerminateFunc terminate;
} Library;

static jlong A7ZipLoader_NativeLoadLibrary(
    JNIEnv* env,
    __unused jclass clazz,
    jstring libname
) {
  const char* c_libname = (*env)->GetStringUTFChars(env, libname, NULL);
  if (c_libname == NULL) {
    THROW_EXCEPTION(env, "GetStringUTFChars fails");
    return 0;
  }

  // Make a copy of libname c string
  char c_libname_copy[strlen(c_libname) + 1];
  strcpy(c_libname_copy, c_libname);
  (*env)->ReleaseStringUTFChars(env, libname, c_libname);

  void* handle = dlopen(c_libname_copy, RTLD_LAZY | RTLD_LOCAL);
  if (handle == NULL) {
    THROW_EXCEPTION(env, "Can't open dynamic library %s: %s", c_libname_copy, dlerror());
    return 0;
  }

  InitializeFunc initialize = dlsym(handle, "Initialize");
  if (initialize == NULL) {
    dlclose(handle);
    THROW_EXCEPTION(env, "Can't find Initialize function in dynamic library %s: %s", c_libname_copy, dlerror());
    return 0;
  }

  if (initialize() != 0) {
    dlclose(handle);
    THROW_EXCEPTION(env, "Can't initialize dynamic library %s", c_libname_copy);
    return 0;
  }

  TerminateFunc terminate = dlsym(handle, "Terminate");
  if (terminate == NULL) {
    dlclose(handle);
    THROW_EXCEPTION(env, "Can't find Terminate function in dynamic library %s: %s ", c_libname_copy, dlerror());
    return 0;
  }

  Library* library = malloc(sizeof(Library));
  if (library == NULL) {
    dlclose(handle);
    THROW_EXCEPTION(env, "Out of memory");
    return 0;
  }

  library->handle = handle;
  library->terminate = terminate;

  return (jlong) library;
}

static void A7ZipLoader_NativeUnloadLibrary(
    __unused JNIEnv* env,
    __unused jclass clazz,
    jlong native_ptr
) {
  if (native_ptr == 0) {
    return;
  }

  Library* library = (Library *) native_ptr;

  if (library->terminate != NULL) {
    library->terminate();
  }
  if (library->handle != NULL) {
    dlclose(library->handle);
  }

  memset(library, 0, sizeof(Library));
  free(library);
}

static JNINativeMethod a7zip_methods[] = {
    { "nativeLoadLibrary",
      "(Ljava/lang/String;)J",
      A7ZipLoader_NativeLoadLibrary },
    { "nativeUnloadLibrary",
      "(J)V",
      A7ZipLoader_NativeUnloadLibrary }
};

jint JNI_OnLoad(
    JavaVM* vm,
    __unused void* reserved
) {
  JNIEnv* env;

  if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass clazz = (*env)->FindClass(env, "com/hippo/a7zip/A7ZipLoader");
  if (clazz == NULL) {
    return JNI_ERR;
  }
  jint result = (*env)->RegisterNatives(env, clazz, a7zip_methods, sizeof(a7zip_methods) / sizeof(JNINativeMethod));
  if (result < 0) {
    return JNI_ERR;
  }

  return JNI_VERSION_1_6;
}
