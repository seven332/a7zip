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

#include "P7Zip.h"

#include <dlfcn.h>

#include <Windows/PropVariant.h>
#include <7zip/Archive/IArchive.h>
#include <7zip/IPassword.h>

#include "Log.h"
#include "Utils.h"

#ifdef LOG_TAG
#  undef LOG_TAG
#  define LOG_TAG "P7Zip"
#endif

using namespace a7zip;

typedef UInt32 (WINAPI *GetNumberFunc)(UInt32* numMethods);
typedef UInt32 (WINAPI *GetPropertyFunc)(UInt32 index, PROPID propID, PROPVARIANT* value);
typedef UInt32 (WINAPI *CreateObjectFunc)(const GUID* clsID, const GUID* iid, void** outObject);

class Method {
 public:
  bool has_name;
  AString name;
  bool has_encoder;
  GUID encoder;
  bool has_decoder;
  GUID decoder;
};

class Format {
 public:
  GUID class_id;
  bool has_name;
  AString name;
  UInt32 signature_offset;
  CObjectVector<CByteBuffer> signatures;
};

class CompressCodecsInfo :
    public ICompressCodecsInfo,
    public CMyUnknownImp {
 public:
  MY_UNKNOWN_IMP1(ICompressCodecsInfo)
  STDMETHOD(GetNumMethods)(UInt32* numMethods);
  STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT* value);
  STDMETHOD(CreateDecoder)(UInt32 index, const GUID* interfaceID, void** coder);
  STDMETHOD(CreateEncoder)(UInt32 index, const GUID* interfaceID, void** coder);
};

static bool initialized = false;
static void* handle = nullptr;
static GetNumberFunc get_number_of_methods = nullptr;
static GetNumberFunc get_number_of_formats = nullptr;
static GetPropertyFunc get_method_property = nullptr;
static GetPropertyFunc get_handler_property = nullptr;
static CreateObjectFunc create_object = nullptr;
static CObjectVector<Method> methods;
static CObjectVector<Format> formats;
static CompressCodecsInfo compress_codecs_info;

HRESULT CompressCodecsInfo::GetNumMethods(UInt32 *numMethods) {
  if (numMethods != nullptr) {
    *numMethods = methods.Size();
  }
  return S_OK;
}

HRESULT CompressCodecsInfo::GetProperty(UInt32 index, PROPID propID, PROPVARIANT* value) {
  Method& method = methods[index];

  switch (propID) {
    case NMethodPropID::kDecoderIsAssigned: {
      NWindows::NCOM::CPropVariant propVariant;
      propVariant = method.has_decoder;
      propVariant.Detach(value);
      return S_OK;
    }
    case NMethodPropID::kEncoderIsAssigned: {
      NWindows::NCOM::CPropVariant propVariant;
      propVariant = method.has_encoder;
      propVariant.Detach(value);
      return S_OK;
    }
    default: {
      return get_method_property(index, propID, value);
    }
  }
}

HRESULT CompressCodecsInfo::CreateDecoder(
    UInt32 index,
    const GUID* interfaceID,
    void** coder
) {
  Method& method = methods[index];

  if (method.has_decoder) {
    return create_object(&(method.decoder), interfaceID, coder);
  } else {
    return S_OK;
  }
}

HRESULT CompressCodecsInfo::CreateEncoder(
    UInt32 index,
    const GUID* interfaceID,
    void** coder
) {
  Method& method = methods[index];

  if (method.has_encoder) {
    return create_object(&(method.encoder), interfaceID, coder);
  } else {
    return S_OK;
  }
}

class ArchiveOpenCallback :
    public IArchiveOpenCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp {
 public:
  ArchiveOpenCallback(BSTR password) {
    this->password = ::SysAllocString(password);
  }

  ~ArchiveOpenCallback() {
    ::SysFreeString(password);
  }

 public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)
  INTERFACE_IArchiveOpenCallback({ return S_OK; });

  STDMETHOD(CryptoGetTextPassword)(BSTR *password) {
    *password = ::SysAllocString(this->password);
    return this->password != nullptr ? S_OK : E_NO_PASSWORD;
  }

 private:
  BSTR password;
};

static HRESULT LoadLibrary(const char *library_name) {
  if (handle != nullptr) return S_OK;

  handle = dlopen(library_name, RTLD_LAZY | RTLD_LOCAL);
  if (handle == nullptr) return E_DLOPEN;

  get_number_of_methods = reinterpret_cast<GetNumberFunc>(dlsym(handle, "GetNumberOfMethods"));
  get_number_of_formats = reinterpret_cast<GetNumberFunc>(dlsym(handle, "GetNumberOfFormats"));
  get_method_property = reinterpret_cast<GetPropertyFunc>(dlsym(handle, "GetMethodProperty"));
  get_handler_property = reinterpret_cast<GetPropertyFunc>(dlsym(handle, "GetHandlerProperty2"));
  create_object = reinterpret_cast<CreateObjectFunc>(dlsym(handle, "CreateObject"));
  if (get_method_property == nullptr
      || get_number_of_methods == nullptr
      || get_number_of_formats == nullptr
      || get_handler_property == nullptr
      || create_object == nullptr) {
    dlclose(handle);
    handle = nullptr;
    return E_DLSYM;
  }

  return JNI_OK;
}

#define GET_PROP_METHOD(METHOD_NAME, PROP_TYPE, VALUE_TYPE, CONVERTER)         \
HRESULT METHOD_NAME(                                                           \
    GetPropertyFunc get_property,                                              \
    UInt32 index,                                                              \
    PROPID prop_id,                                                            \
    PROP_TYPE& value,                                                          \
    bool& is_assigned                                                          \
) {                                                                            \
  NWindows::NCOM::CPropVariant prop;                                           \
  is_assigned = false;                                                         \
                                                                               \
  RETURN_SAME_IF_NOT_ZERO(get_property(index, prop_id, &prop));                \
                                                                               \
  if (prop.vt == VALUE_TYPE) {                                                 \
    is_assigned = true;                                                        \
    CONVERTER;                                                                 \
  } else if (prop.vt != VT_EMPTY) {                                            \
    return E_INCONSISTENT_PROP_TYPE;                                           \
  }                                                                            \
                                                                               \
  return S_OK;                                                                 \
}

GET_PROP_METHOD(GetAString, AString, VT_BSTR, value.SetFromWStr_if_Ascii(prop.bstrVal));

GET_PROP_METHOD(GetGUID, GUID, VT_BSTR, value = *reinterpret_cast<const GUID*>(prop.bstrVal));

GET_PROP_METHOD(GetCByteBuffer, CByteBuffer, VT_BSTR, value.CopyFrom((const Byte *)prop.bstrVal, ::SysStringByteLen(prop.bstrVal)));

GET_PROP_METHOD(GetUInt32, UInt32, VT_UI4, value = prop.ulVal);

#undef GET_PROP_METHOD

static HRESULT LoadMethods() {
  UInt32 method_number = 0;

  RETURN_SAME_IF_NOT_ZERO(get_number_of_methods(&method_number));

  for(UInt32 i = 0; i < method_number; i++) {
    Method& method = methods.AddNew();

#   define GET_PROP(METHOD, PROP, VALUE, ASSIGNED)                              \
    if (METHOD(get_method_property, i, PROP, VALUE, ASSIGNED) != S_OK) {        \
      methods.DeleteBack();                                                     \
      continue;                                                                 \
    }

    // It's ok to assume all characters in method name are in ascii charset
    GET_PROP(GetAString, NMethodPropID::kName, method.name, method.has_name);
    GET_PROP(GetGUID, NMethodPropID::kDecoder, method.decoder, method.has_decoder);
    GET_PROP(GetGUID, NMethodPropID::kEncoder, method.encoder, method.has_encoder);

#   undef GET_PROP
  }

  return S_OK;
}

static void AppendMultiSignature(
    CObjectVector<CByteBuffer>& signatures,
    const unsigned char* multi_signature,
    size_t size
) {
  while (size > 0) {
    unsigned length = *multi_signature++;
    size--;

    if (length > size) {
      return;
    }

    signatures.AddNew().CopyFrom(multi_signature, length);

    multi_signature += length;
    size -= length;
  }
}

static HRESULT LoadFormats() {
  UInt32 format_number = 0;

  RETURN_SAME_IF_NOT_ZERO(get_number_of_formats(&format_number));

  for(UInt32 i = 0; i < format_number; i++) {
    Format& format = formats.AddNew();

#   define GET_PROP(METHOD, PROP, VALUE, ASSIGNED)                              \
    if (METHOD(get_handler_property, i, PROP, VALUE, ASSIGNED) != S_OK) {       \
      formats.DeleteBack();                                                     \
      continue;                                                                 \
    }

    // ClassID is required
    bool has_class_id;
    GET_PROP(GetGUID, NArchive::NHandlerPropID::kClassID, format.class_id, has_class_id);
    if (!has_class_id) {
      formats.DeleteBack();
      continue;
    }

    // It's ok to assume all characters in format name are in ascii charset
    GET_PROP(GetAString, NArchive::NHandlerPropID::kName, format.name, format.has_name);

    bool has_signature;
    CByteBuffer signature;
    GET_PROP(GetUInt32, NArchive::NHandlerPropID::kSignatureOffset, format.signature_offset, has_signature);
    if (!has_signature) {
      format.signature_offset = 0;
    }
    GET_PROP(GetCByteBuffer, NArchive::NHandlerPropID::kSignature, signature, has_signature);
    if (has_signature) {
      format.signatures.AddNew().CopyFrom(signature, signature.Size());
    }
    GET_PROP(GetCByteBuffer, NArchive::NHandlerPropID::kMultiSignature, signature, has_signature);
    if (has_signature) {
      AppendMultiSignature(format.signatures, signature, signature.Size());
    }

#   undef GET_PROP
  }

  return S_OK;
}

HRESULT P7Zip::Initialize(const char* library_name) {
  if (initialized) {
    return S_OK;
  }

  RETURN_SAME_IF_NOT_ZERO(LoadLibrary(library_name));
  RETURN_SAME_IF_NOT_ZERO(LoadMethods());
  RETURN_SAME_IF_NOT_ZERO(LoadFormats());

  initialized = true;
  return S_OK;
}

static HRESULT ReadFully(CMyComPtr<IInStream>& stream, Byte* data, UInt32 size, UInt32* processedSize) {
  UInt32 read = 0;

  while (read < size) {
    RETURN_SAME_IF_NOT_ZERO(stream->Read(data + read, size - read, processedSize));

    if (*processedSize == 0) {
      // EOF
      break;
    }

    read += *processedSize;
  }

  *processedSize = read;
  return S_OK;
}

static HRESULT OpenInArchive(
    GUID& class_id,
    CMyComPtr<IInStream>& in_stream,
    BSTR password,
    CMyComPtr<IInArchive>& in_archive
) {
  RETURN_SAME_IF_NOT_ZERO(create_object(&class_id, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)));

  UInt64 newPosition = 0;
  HRESULT result = in_stream->Seek(0, STREAM_SEEK_SET, &newPosition);
  if (result != S_OK) {
    in_archive->Close();
    in_archive = nullptr;
    return result;
  }

  UInt64 maxCheckStartPosition = 1 << 22;
  CMyComPtr<ArchiveOpenCallback> callback(new ArchiveOpenCallback(password));
  result = in_archive->Open(in_stream, &maxCheckStartPosition, callback);
  if (result != S_OK) {
    in_archive->Close();
    in_archive = nullptr;
    return result;
  }

  return S_OK;
}

static HRESULT OpenInArchive(
    CMyComPtr<IInStream>& in_stream,
    BSTR password,
    CMyComPtr<IInArchive>& in_archive,
    AString& format_name
) {
  bool formats_checked[formats.Size()];
  memset(formats_checked, 0, formats.Size() * sizeof(bool));

  for (int i = 0; i < formats.Size(); i++) {
    Format& format = formats[i];

    // Skip format without signatures
    if (format.signatures.Size() == 0) {
      continue;
    }

    // Mark the format
    formats_checked[i] = true;

    // Check each signature
    for (int j = 0; j < format.signatures.Size(); j++) {
      CByteBuffer& signature = format.signatures[j];

      UInt32 processedSize;
      CByteBuffer bytes(signature.Size());

      CONTINUE_IF_NOT_ZERO(in_stream->Seek(0, STREAM_SEEK_SET, nullptr));
      CONTINUE_IF_NOT_ZERO(ReadFully(in_stream, bytes, static_cast<UInt32>(signature.Size()), &processedSize));
      if (processedSize != signature.Size() || bytes != signature) {
        continue;
      }

      // The signature matched, try to open it
      if (OpenInArchive(format.class_id, in_stream, password, in_archive) == S_OK) {
        format_name = format.name;
        return S_OK;
      }

      // Can't open archive in this format
      // Skip this format
      break;
    }
  }

  // Try other unchecked formats
  for (int i = 0; i < formats.Size(); i++) {
    if (!formats_checked[i]) {
      Format& format = formats[i];
      if (OpenInArchive(format.class_id, in_stream, password, in_archive) == S_OK) {
        format_name = format.name;
        return S_OK;
      }
    }
  }

  return E_UNKNOWN_FORMAT;
}

HRESULT P7Zip::OpenArchive(CMyComPtr<IInStream> in_stream, BSTR password, InArchive** archive) {
  CMyComPtr<IInArchive> in_archive = nullptr;
  AString format_name;

  HRESULT result = OpenInArchive(in_stream, password, in_archive, format_name);

  if (result == S_OK && in_archive != nullptr) {
    *archive = new InArchive(in_archive, format_name);
    return S_OK;
  }

  if (in_archive != nullptr) {
    in_archive->Close();
    in_archive = nullptr;
  }

  if (result != S_OK) {
    return result;
  } else {
    return E_INTERNAL;
  }
}
