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

#ifndef __A7ZIP_BLACK_HOLE_H__
#define __A7ZIP_BLACK_HOLE_H__

#include <Common/MyCom.h>
#include <7zip/IStream.h>

#include "Log.h"

namespace a7zip {

class BlackHoleOutStream :
    public ISequentialOutStream,
    public CMyUnknownImp
{
 public:
  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

}

#endif //__A7ZIP_BLACK_HOLE_H__
