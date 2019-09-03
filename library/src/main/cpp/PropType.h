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

#ifndef __A7ZIP_PROP_TYPE_H__
#define __A7ZIP_PROP_TYPE_H__

namespace a7zip {

    enum PropTypeEnum {
        PT_UNKNOWN,
        PT_EMPTY,
        PT_BOOL,
        PT_INT,
        PT_LONG,
        PT_FLOAT,
        PT_DOUBLE,
        PT_STRING,
    };

    typedef unsigned short PropType;

}

#endif //__A7ZIP_PROP_TYPE_H__
