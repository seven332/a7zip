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

#include "JavaInArchive.h"

#include <cstdio>
#include <type_traits>

#include <include_windows/windows.h>
#include <7zip/Archive/IArchive.h>

#include "InStream.h"
#include "JavaHelper.h"
#include "Log.h"
#include "SequentialOutStream.h"
#include "SevenZip.h"
#include "Utils.h"

#ifdef LOG_TAG
#  undef LOG_TAG
#endif //LOG_TAG
#define LOG_TAG "JavaArchive"

using namespace a7zip;

#define THROW_ARCHIVE_EXCEPTION_RETURN(ENV, CODE)                                         \
  do {                                                                                    \
    if ((CODE) == E_NO_PASSWORD || (CODE) == E_WRONG_PASSWORD) {                          \
      JavaHelper::ThrowException(ENV, "com/hippo/a7zip/PasswordException", (CODE));       \
    } else {                                                                              \
      JavaHelper::ThrowException(ENV, "com/hippo/a7zip/ArchiveException", (CODE));        \
    }                                                                                     \
    return;                                                                               \
  } while (0)

#define THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(ENV, CODE, VALUE)                            \
  do {                                                                                    \
    if ((CODE) == E_NO_PASSWORD || (CODE) == E_WRONG_PASSWORD) {                          \
      JavaHelper::ThrowException(ENV, "com/hippo/a7zip/PasswordException", (CODE));       \
    } else {                                                                              \
      JavaHelper::ThrowException(ENV, "com/hippo/a7zip/ArchiveException", (CODE));        \
    }                                                                                     \
    return VALUE;                                                                         \
  } while (0)

#define CHECK_CLOSED_RETURN(ENV, NATIVE_PTR)                                              \
  do {                                                                                    \
    if ((NATIVE_PTR) == 0) {                                                              \
      JavaHelper::ThrowException(                                                         \
          ENV, "java/lang/IllegalStateException", "This Archive is closed.");             \
      return;                                                                             \
    }                                                                                     \
  } while (0)

#define CHECK_CLOSED_RETURN_VALUE(ENV, NATIVE_PTR, VALUE)                                 \
  do {                                                                                    \
    if ((NATIVE_PTR) == 0) {                                                              \
      JavaHelper::ThrowException(                                                         \
          ENV, "java/lang/IllegalStateException", "This Archive is closed.");             \
      return VALUE;                                                                       \
    }                                                                                     \
  } while (0)

static void CopyJStringToBSTR(BSTR bstr, const jchar* jstr, int length) {
  for (int i = 0; i < length; i++) {
    *bstr++ = *jstr++;
  }
  *bstr = 0;
}

jlong a7zip_NativeOpen(
    JNIEnv* env,
    jclass,
    jobject stream,
    jstring password
) {
  CMyComPtr<IInStream> in_stream = nullptr;
  HRESULT result = InStream::Create(env, stream, in_stream);
  if (result != S_OK || in_stream == nullptr) {
    // Call java methods before throw exception
    if (in_stream != nullptr) {
      in_stream.Release();
    }
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, 0);
  }

  const jchar* j_password = nullptr;
  BSTR bstr_password = nullptr;
  if (password != nullptr) {
    jsize length = env->GetStringLength(password);
    j_password = env->GetStringChars(password, nullptr);
    bstr_password = ::SysAllocStringLen(nullptr, static_cast<UINT>(length));
    CopyJStringToBSTR(bstr_password, j_password, length);
  }

  InArchive* archive = nullptr;
  result = SevenZip::OpenArchive(in_stream, bstr_password, &archive);

  if (password != nullptr) {
    ::SysFreeString(bstr_password);
    env->ReleaseStringChars(password, j_password);
  }

  if (result != S_OK || archive == nullptr) {
    // Call java methods before throw exception
    in_stream.Release();
    if (archive != nullptr) {
      delete archive;
    }
    THROW_ARCHIVE_EXCEPTION_RETURN_VALUE(env, result, 0);
  }

  return reinterpret_cast<jlong>(archive);
}

jstring a7zip_NativeGetFormatName(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, nullptr);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);
  return env->NewStringUTF(archive->GetFormatName());
}

jint a7zip_NativeGetNumberOfEntries(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

  UInt32 number = 0;
  HRESULT result = archive->GetNumberOfEntries(number);
  return result == S_OK ? number : -1;
}

#define GET_ARCHIVE_PROPERTY_START(METHOD_NAME, RETURN_TYPE)                              \
RETURN_TYPE METHOD_NAME(JNIEnv* env, jclass, jlong native_ptr, jint prop_id) {            \
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);                                          \
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

#define GET_ARCHIVE_PROPERTY_END                                                          \
}

#define GET_ENTRY_PROPERTY_START(METHOD_NAME, RETURN_TYPE)                                \
RETURN_TYPE METHOD_NAME(JNIEnv* env, jclass, jlong native_ptr, jint index, jint prop_id) {\
  CHECK_CLOSED_RETURN_VALUE(env, native_ptr, 0);                                          \
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

#define GET_ENTRY_PROPERTY_END                                                            \
}

#define GET_PROPERTY_TYPE(GETTER)                                                         \
  PropType prop_type;                                                                     \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? prop_type : PT_UNKNOWN;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchivePropertyType, jint)
  GET_PROPERTY_TYPE(archive->GetArchivePropertyType(static_cast<PROPID>(prop_id), &prop_type))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryPropertyType, jint)
  GET_PROPERTY_TYPE(archive->GetEntryPropertyType(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &prop_type))
GET_ENTRY_PROPERTY_END

#define GET_BOOL_TYPE(GETTER)                                                             \
  bool bool_prop;                                                                         \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? bool_prop : false;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveBooleanProperty, jboolean)
  GET_BOOL_TYPE(archive->GetArchiveBooleanProperty(static_cast<PROPID>(prop_id), &bool_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryBooleanProperty, jboolean)
  GET_BOOL_TYPE(archive->GetEntryBooleanProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &bool_prop))
GET_ENTRY_PROPERTY_END

#define GET_INT_TYPE(GETTER)                                                              \
  Int32 int_prop;                                                                         \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? int_prop : 0;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveIntProperty, jint)
  GET_INT_TYPE(archive->GetArchiveIntProperty(static_cast<PROPID>(prop_id), &int_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryIntProperty, jint)
  GET_INT_TYPE(archive->GetEntryIntProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &int_prop))
GET_ENTRY_PROPERTY_END

#define GET_LONG_TYPE(GETTER)                                                             \
  Int64 long_prop;                                                                        \
  HRESULT result = (GETTER);                                                              \
  return result == S_OK ? long_prop : 0;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveLongProperty, jlong)
  GET_LONG_TYPE(archive->GetArchiveLongProperty(static_cast<PROPID>(prop_id), &long_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryLongProperty, jlong)
  GET_LONG_TYPE(archive->GetEntryLongProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &long_prop))
GET_ENTRY_PROPERTY_END

static void shrink(BSTR bstr) {
  jchar* jstr = reinterpret_cast<jchar*>(bstr);
  UINT n = ::SysStringLen(bstr);
  for (UINT i = 0; i < n; i++) {
    jstr[i] = static_cast<jchar>(bstr[i]);
  }
  jstr[n] = 0;
}

#define GET_STRING_PROPERTY(GETTER)                                                       \
  BSTR str_prop = nullptr;                                                                \
  HRESULT result = (GETTER);                                                              \
  if (result != S_OK || str_prop == nullptr) {                                            \
    if (str_prop != nullptr) ::SysFreeString(str_prop);                                   \
    return nullptr;                                                                       \
  }                                                                                       \
  shrink(str_prop);                                                                       \
  jstring jstr =                                                                          \
      env->NewString(reinterpret_cast<const jchar*>(str_prop), ::SysStringLen(str_prop)); \
  ::SysFreeString(str_prop);                                                              \
  return jstr;

GET_ARCHIVE_PROPERTY_START(a7zip_NativeGetArchiveStringProperty, jstring)
  GET_STRING_PROPERTY(archive->GetArchiveStringProperty(static_cast<PROPID>(prop_id), &str_prop))
GET_ARCHIVE_PROPERTY_END

GET_ENTRY_PROPERTY_START(a7zip_NativeGetEntryStringProperty, jstring)
  GET_STRING_PROPERTY(archive->GetEntryStringProperty(static_cast<UInt32>(index), static_cast<PROPID>(prop_id), &str_prop))
GET_ENTRY_PROPERTY_END

void a7zip_NativeExtractEntry(
    JNIEnv* env,
    jclass,
    jlong native_ptr,
    jint index,
    jstring password,
    jobject stream
) {
  CHECK_CLOSED_RETURN(env, native_ptr);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);

  CMyComPtr<ISequentialOutStream> out_stream = nullptr;
  HRESULT result = SequentialOutStream::Create(env, stream, out_stream);
  if (result != S_OK || out_stream == nullptr) {
    if (out_stream != nullptr) {
      // Call java methods before throw exception
      out_stream.Release();
    } else {
      // Let java code closes the OutStream
    }
    THROW_ARCHIVE_EXCEPTION_RETURN(env, result);
  }

  const jchar* j_password = nullptr;
  BSTR bstr_password = nullptr;
  if (password != nullptr) {
    jsize length = env->GetStringLength(password);
    j_password = env->GetStringChars(password, nullptr);
    bstr_password = ::SysAllocStringLen(nullptr, static_cast<UINT>(length));
    CopyJStringToBSTR(bstr_password, j_password, length);
  }

  result = archive->ExtractEntry(static_cast<UInt32>(index), bstr_password, out_stream);

  if (password != nullptr) {
    ::SysFreeString(bstr_password);
    env->ReleaseStringChars(password, j_password);
  }

  if (result != S_OK) {
    // Call java methods before throw exception
    out_stream.Release();
    THROW_ARCHIVE_EXCEPTION_RETURN(env, result);
  }
}

void a7zip_NativeClose(
    JNIEnv* env,
    jclass,
    jlong native_ptr
) {
  CHECK_CLOSED_RETURN(env, native_ptr);
  InArchive* archive = reinterpret_cast<InArchive*>(native_ptr);
  delete archive;
}

static JNINativeMethod archive_methods[] = {
    { "nativeOpen",
      "(Lcom/hippo/a7zip/InStream;Ljava/lang/String;)J",
      reinterpret_cast<void *>(a7zip_NativeOpen) },
    { "nativeGetFormatName",
      "(J)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetFormatName) },
    { "nativeGetNumberOfEntries",
      "(J)I",
      reinterpret_cast<void *>(a7zip_NativeGetNumberOfEntries) },
    { "nativeGetArchivePropertyType",
      "(JI)I",
      reinterpret_cast<void *>(a7zip_NativeGetArchivePropertyType) },
    { "nativeGetArchiveBooleanProperty",
      "(JI)Z",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveBooleanProperty) },
    { "nativeGetArchiveIntProperty",
      "(JI)I",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveIntProperty) },
    { "nativeGetArchiveLongProperty",
      "(JI)J",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveLongProperty) },
    { "nativeGetArchiveStringProperty",
      "(JI)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetArchiveStringProperty) },
    { "nativeGetEntryPropertyType",
      "(JII)I",
      reinterpret_cast<void *>(a7zip_NativeGetEntryPropertyType) },
    { "nativeGetEntryBooleanProperty",
      "(JII)Z",
      reinterpret_cast<void *>(a7zip_NativeGetEntryBooleanProperty) },
    { "nativeGetEntryIntProperty",
      "(JII)I",
      reinterpret_cast<void *>(a7zip_NativeGetEntryIntProperty) },
    { "nativeGetEntryLongProperty",
      "(JII)J",
      reinterpret_cast<void *>(a7zip_NativeGetEntryLongProperty) },
    { "nativeGetEntryStringProperty",
      "(JII)Ljava/lang/String;",
      reinterpret_cast<void *>(a7zip_NativeGetEntryStringProperty) },
    { "nativeExtractEntry",
      "(JILjava/lang/String;Lcom/hippo/a7zip/SequentialOutStream;)V",
      reinterpret_cast<void *>(a7zip_NativeExtractEntry) },
    { "nativeClose",
      "(J)V",
      reinterpret_cast<void *>(a7zip_NativeClose) }
};

HRESULT JavaInArchive::RegisterMethods(JNIEnv* env) {
  jclass clazz = env->FindClass("com/hippo/a7zip/InArchive");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  jint result = env->RegisterNatives(clazz, archive_methods, std::extent<decltype(archive_methods)>::value);
  if (result < 0) {
    return E_FAILED_REGISTER;
  }

  return S_OK;
}

HRESULT JavaInArchive::UnregisterMethods(JNIEnv *env) {
  jclass clazz = env->FindClass("com/hippo/a7zip/InArchive");
  if (clazz == nullptr) return E_CLASS_NOT_FOUND;

  jint result = env->UnregisterNatives(clazz);
  if (result < 0) {
    return E_FAILED_UNREGISTER;
  }

  return S_OK;
}
