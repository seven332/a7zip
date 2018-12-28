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

import androidx.annotation.Nullable;
import java.io.Closeable;
import java.nio.charset.Charset;
import okio.BufferedStore;
import okio.Okio;
import okio.Store;

public class InArchive implements Closeable {

  private long nativePtr;

  private InArchive(long nativePtr) {
    this.nativePtr = nativePtr;
  }

  private void checkClose() {
    if (nativePtr == 0) {
      throw new IllegalStateException("This InArchive is closed.");
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

  /**
   * TODO
   */
  @Nullable
  public String getArchiveStringProperty(PropID propID) throws ArchiveException {
    return getArchiveStringProperty(propID, null);
  }

  /**
   * TODO
   */
  @Nullable
  public String getArchiveStringProperty(PropID propID, Charset charset) throws ArchiveException {
    checkClose();
    String str = nativeGetArchiveStringProperty(nativePtr, propID.ordinal());
    return applyCharsetToString(str, charset);
  }

  /**
   * TODO
   */
  @Nullable
  public String getEntryStringProperty(int index, PropID propID) throws ArchiveException {
    return getEntryStringProperty(index, propID, null);
  }

  // DEL
  public String getEntryStringProperty(int index, int propID, Charset charset) throws ArchiveException {
    checkClose();
    String str = nativeGetEntryStringProperty(nativePtr, index, propID);
    return applyCharsetToString(str, charset);
  }

  /**
   * TODO
   */
  @Nullable
  public String getEntryStringProperty(int index, PropID propID, Charset charset) throws ArchiveException {
    checkClose();
    String str = nativeGetEntryStringProperty(nativePtr, index, propID.ordinal());
    return applyCharsetToString(str, charset);
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

  public static InArchive create(Store store) throws ArchiveException {
    if (store instanceof BufferedStore) {
      return create((BufferedStore) store);
    } else {
      return create(Okio.buffer(store));
    }
  }

  public static InArchive create(BufferedStore store) throws ArchiveException {
    long nativePtr = nativeCreate(store);

    if (nativePtr == 0) {
      // It should not be 0
      throw new ArchiveException("a7zip is buggy");
    }

    return new InArchive(nativePtr);
  }

  private static native long nativeCreate(BufferedStore store) throws ArchiveException;

  private static native String nativeGetFormatName(long nativePtr);

  private static native int nativeGetNumberOfEntries(long nativePtr);

  private static native String nativeGetArchiveStringProperty(long nativePtr, int propID) throws ArchiveException;

  private static native String nativeGetEntryStringProperty(long nativePtr, int index, int propID) throws ArchiveException;

  private static native String nativeGetEntryPath(long nativePtr, int index);

  private static native void nativeClose(long nativePtr);
}
