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

package com.hippo.a7zip;

import java.io.Closeable;
import java.nio.charset.Charset;
import okio.BufferedStore;
import okio.Okio;
import okio.Store;

public class Archive implements Closeable {

  private long nativePtr;

  private Archive(long nativePtr) {
    this.nativePtr = nativePtr;
  }

  private void checkClose() {
    if (nativePtr == 0) {
      throw new IllegalStateException("This Archive is closed");
    }
  }

  private String applyCharsetToString(String str, Charset charset) {
    if (str == null || charset == null) {
      return str;
    }

    int length = str.length();
    for (int i = 0; i < length; i++) {
      char c = str.charAt(i);
      if (c > 0xFF) {
        // It's not a pure byte list, can't apply charset
        return str;
      }
    }

    byte[] bytes = new byte[length];
    for (int i = 0; i < length; i++) {
      bytes[i] = (byte) str.charAt(i);
    }

    return new String(bytes, charset);
  }

  public String getFormatName() {
    checkClose();
    return nativeGetFormatName(nativePtr);
  }

  public int getNumberOfEntries() {
    checkClose();
    return nativeGetNumberOfEntries(nativePtr);
  }

  public String getEntryPath(int index) {
    return getEntryPath(index, null);
  }

  public String getEntryPath(int index, Charset charset) {
    checkClose();
    String path = nativeGetEntryPath(nativePtr, index);
    return applyCharsetToString(path, charset);
  }

  @Override
  public void close() {
    if (nativePtr != 0) {
      nativeClose(nativePtr);
      nativePtr = 0;
    }
  }

  public static Archive create(Store store) throws ArchiveException {
    if (store instanceof BufferedStore) {
      return create((BufferedStore) store);
    } else {
      return create(Okio.buffer(store));
    }
  }

  public static Archive create(BufferedStore store) throws ArchiveException {
    long nativePtr = nativeCreate(store);

    if (nativePtr == 0) {
      return null;
    }

    return new Archive(nativePtr);
  }

  private static native long nativeCreate(BufferedStore store) throws ArchiveException;

  private static native String nativeGetFormatName(long nativePtr);

  private static native int nativeGetNumberOfEntries(long nativePtr);

  private static native String nativeGetEntryPath(long nativePtr, int index);

  private static native void nativeClose(long nativePtr);
}
