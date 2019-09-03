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

package com.hippo.a7zip;

import androidx.test.platform.app.InstrumentationRegistry;

import com.getkeepsafe.relinker.ReLinker;

public class ReLinkerLibraryLoader implements A7ZipLibraryLoader {

    @Override
    public void loadLibrary(String libname) {
        ReLinker.loadLibrary(InstrumentationRegistry.getInstrumentation().getTargetContext(), libname);
    }
}
