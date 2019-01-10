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

public class A7Zip {

  /**
   * Initializes A7Zip. This method loads native libraries
   * with {@link System#loadLibrary(String)} directly.
   */
  public static void initialize() {
    System.loadLibrary("p7zip");
    System.loadLibrary("a7zip");
  }

  /**
   * Initializes A7Zip. This method loads native libraries
   * with custom library loader.
   */
  public static void initialize(LibraryLoader loader) {
    loader.loadLibrary("p7zip");
    loader.loadLibrary("a7zip");
  }

  /**
   * Custom library loader.
   */
  public interface LibraryLoader {

    /**
     * Loads the native library. Using {@link System#loadLibrary(String)}
     * directly may cause {@link UnsatisfiedLinkError} on some
     * devices. Some libraries can fix it,
     * like <a href="https://github.com/KeepSafe/ReLinker">ReLinker</a>
     * and <a href="https://github.com/facebook/SoLoader">SoLoader</a>.
     * The method could be a wrapper.
     *
     * @param library the name of the library
     */
    void loadLibrary(String library);
  }
}
