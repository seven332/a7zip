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

import android.support.annotation.NonNull;

public class A7ZipExtract {

  public static final A7ZipLibrary LIBRARY = new A7ZipLibrary() {
    @Override
    String getMainLibraryName() {
      return "liba7zip-extract.so";
    }

    @NonNull
    @Override
    String[] getMinorLibraryNames() {
      return new String[] { "libp7zip-extract.so" };
    }
  };
}
