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

#include "InArchive.h"

#include <Windows/PropVariant.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

#include "Log.h"
#include "Utils.h"

using namespace a7zip;

class ArchiveExtractCallback :
    public IArchiveExtractCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp
{
 public:
  ArchiveExtractCallback(CMyComPtr<ISequentialOutStream> out_stream);

 public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

  STDMETHOD(SetTotal)(UInt64 total);
  STDMETHOD(SetCompleted)(const UInt64 *completeValue);

  STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode);
  STDMETHOD(PrepareOperation)(Int32 askExtractMode);
  STDMETHOD(SetOperationResult)(Int32 opRes);

  STDMETHOD(CryptoGetTextPassword)(BSTR *password);

 private:
  CMyComPtr<ISequentialOutStream> out_stream;
};

ArchiveExtractCallback::ArchiveExtractCallback(CMyComPtr<ISequentialOutStream> out_stream) {
  this->out_stream = out_stream;
}

HRESULT ArchiveExtractCallback::SetTotal(UInt64 total) {
  // Ignored
  return S_OK;
}

HRESULT ArchiveExtractCallback::SetCompleted(const UInt64 *completeValue) {
  // Ignored
  return S_OK;
}

HRESULT ArchiveExtractCallback::GetStream(
    UInt32,
    ISequentialOutStream** outStream,
    Int32 askExtractMode
) {
  if (askExtractMode != NArchive::NExtract::NAskMode::kExtract) {
    return S_OK;
  }

  *outStream = out_stream.Detach();

  return S_OK;
}

HRESULT ArchiveExtractCallback::PrepareOperation(Int32 askExtractMode) {
  // Ignored
  return S_OK;
}

HRESULT ArchiveExtractCallback::SetOperationResult(Int32 opRes) {
  // Ignored
  return S_OK;
}

HRESULT ArchiveExtractCallback::CryptoGetTextPassword(BSTR *password) {
  // TODO
  return S_OK;
}

InArchive::InArchive(CMyComPtr<IInArchive> in_archive, AString format_name) {
  this->in_archive = in_archive;
  this->format_name = format_name;
}

InArchive::~InArchive() {
  this->in_archive->Close();
}

const AString& InArchive::GetFormatName() {
  return this->format_name;
}

HRESULT InArchive::GetNumberOfEntries(UInt32& number) {
  return this->in_archive->GetNumberOfItems(&number);
}

HRESULT InArchive::GetEntryPath(UInt32 index, BSTR* name) {
  NWindows::NCOM::CPropVariant prop;
  RETURN_SAME_IF_NOT_ZERO(this->in_archive->GetProperty(index, kpidPath, &prop));

  if (prop.vt == VT_BSTR) {
    *name = ::SysAllocString(prop.bstrVal);
    return S_OK;
  } else {
    return E_INCONSISTENT_PROP_TYPE;
  }
}

HRESULT InArchive::ExtractEntry(UInt32 index, CMyComPtr<ISequentialOutStream> out_stream) {
  CMyComPtr<ArchiveExtractCallback> callback(new ArchiveExtractCallback(out_stream));
  return this->in_archive->Extract(&index, 1, false, callback);
}
