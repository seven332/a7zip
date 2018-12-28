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

import androidx.annotation.NonNull;
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

  /**
   * Returns the number of entries in this archive.
   * {@code -1} if get error.
   */
  public int getNumberOfEntries() {
    checkClose();
    return nativeGetNumberOfEntries(nativePtr);
  }

  public PropType getArchivePropertyType(PropID propID) {
    int type = nativeGetArchivePropertyType(nativePtr, propID.ordinal());
    if (type >= 0 || type < PropType.values().length) {
      return PropType.values()[type];
    } else {
      return PropType.UNKNOWN;
    }
  }

  /**
   * TODO
   */
  @NonNull
  public String getArchiveStringProperty(PropID propID) throws ArchiveException {
    return getArchiveStringProperty(propID, null);
  }

  /**
   * TODO
   */
  @NonNull
  public String getArchiveStringProperty(PropID propID, Charset charset) throws ArchiveException {
    checkClose();
    String str = nativeGetArchiveStringProperty(nativePtr, propID.ordinal());
    return applyCharsetToString(str, charset);
  }

  public PropType getEntryPropertyType(int index, PropID propID) {
    int type = nativeGetEntryPropertyType(nativePtr, index, propID.ordinal());
    if (type >= 0 || type < PropType.values().length) {
      return PropType.values()[type];
    } else {
      return PropType.UNKNOWN;
    }
  }

  /**
   * TODO
   */
  @NonNull
  public String getEntryStringProperty(int index, PropID propID) throws ArchiveException {
    return getEntryStringProperty(index, propID, null);
  }

  /**
   * TODO
   */
  @NonNull
  public String getEntryStringProperty(int index, PropID propID, Charset charset) throws ArchiveException {
    checkClose();
    String str = nativeGetEntryStringProperty(nativePtr, index, propID.ordinal());
    return applyCharsetToString(str, charset);
  }

  public String getEntryPath(int index) throws ArchiveException {
    return getEntryPath(index, null);
  }

  public String getEntryPath(int index, Charset charset) throws ArchiveException {
    PropID propID = PropID.PATH;
    PropType propType = getEntryPropertyType(index, propID);
    if (propType == PropType.STRING) {
      return getEntryStringProperty(index, propID, charset);
    }
    return null;
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

  private static native int nativeGetArchivePropertyType(long nativePtr, int propID);

  private static native String nativeGetArchiveStringProperty(long nativePtr, int propID) throws ArchiveException;

  private static native int nativeGetEntryPropertyType(long nativePtr, int index, int propID);

  private static native String nativeGetEntryStringProperty(long nativePtr, int index, int propID) throws ArchiveException;

  private static native void nativeClose(long nativePtr);
}
