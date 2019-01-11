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

class A7ZipLoader {

  private static boolean loaderLoaded = false;
  private static long nativeLibrary = 0;

  static synchronized void loadLibrary(A7ZipLibrary library, A7ZipLibraryLoader loader) {
    // Ensure loader is loaded
    if (!loaderLoaded) {
      loader.loadLibrary("liba7zip-loader.so");
      loaderLoaded = true;
    }

    // Unload current native library
    if (nativeLibrary != 0) {
      try {
        nativeUnloadLibrary(nativeLibrary);
      } finally {
        nativeLibrary = 0;
      }
    }

    for (String libname : library.getMinorLibraryNames()) {
      loader.loadLibrary(libname);
    }
    loader.loadLibrary(library.getMainLibraryName());

    nativeLibrary = nativeLoadLibrary(library.getMainLibraryName());
  }

  private static native long nativeLoadLibrary(String libname);

  private static native void nativeUnloadLibrary(long nativePtr);
}
