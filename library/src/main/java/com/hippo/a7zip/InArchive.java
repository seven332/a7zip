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

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import java.io.Closeable;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.Charset;

public class InArchive implements Closeable {

  private long nativePtr;
  @Nullable
  private Charset charset;
  @Nullable
  private String password;

  private InArchive(long nativePtr, @Nullable Charset charset, @Nullable String password) {
    this.nativePtr = nativePtr;
    this.charset = charset;
    this.password = password;
  }

  private void checkClosed() {
    if (nativePtr == 0) {
      throw new IllegalStateException("This InArchive is closed.");
    }
  }

  // Sometimes p7zip returns string in the original charset, sometimes in utf-16.
  // If it in unknown charset, each byte stores in each char,
  // so every char in the string is smaller than 0xFF.
  // But if every char in the string is smaller than 0xFF,
  // it's hard to tell the string is in utf-16 or the original charset.
  // TODO Let p7zip tell whether it have encoded the string.
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

  /**
   * Returns the format name of this archive.
   * Empty string if can't get the name.
   */
  @NonNull
  public String getFormatName() {
    checkClosed();
    return nativeGetFormatName(nativePtr);
  }

  /**
   * Returns the number of entries in this archive.
   * {@code -1} if get error.
   */
  public int getNumberOfEntries() {
    checkClosed();
    return nativeGetNumberOfEntries(nativePtr);
  }

  /**
   * Returns the type of the property for the archive.
   *
   * @param propID the id of the property
   * @return one of {@link PropType}
   */
  public PropType getArchivePropertyType(PropID propID) {
    int type = nativeGetArchivePropertyType(nativePtr, propID.ordinal());
    if (type >= 0 || type < PropType.values().length) {
      return PropType.values()[type];
    } else {
      return PropType.UNKNOWN;
    }
  }

  /**
   * Returns boolean property for the archive.
   *
   * @param propID the id of the property
   * @return the boolean property, {@code false} if get error
   */
  public boolean getArchiveBooleanProperty(PropID propID) {
    checkClosed();
    return nativeGetArchiveBooleanProperty(nativePtr, propID.ordinal());
  }

  /**
   * Returns int property for the archive.
   *
   * @param propID the id of the property
   * @return the int property, {@code 0} if get error
   */
  public int getArchiveIntProperty(PropID propID) {
    checkClosed();
    return nativeGetArchiveIntProperty(nativePtr, propID.ordinal());
  }

  /**
   * Returns long property for the archive.
   *
   * @param propID the id of the property
   * @return the long property, {@code 0} if get error
   */
  public long getArchiveLongProperty(PropID propID) {
    checkClosed();
    return nativeGetArchiveLongProperty(nativePtr, propID.ordinal());
  }

  /**
   * Returns string property for the archive.
   *
   * @param propID the id of the property
   * @return the string property, empty string if get error
   */
  @NonNull
  public String getArchiveStringProperty(PropID propID) {
    return getArchiveStringProperty(propID, charset);
  }

  /**
   * Returns string property for the archive.
   *
   * @param propID the id of the property
   * @param charset the charset of the string, {@code null} to let p7zip handle it
   * @return the string property, empty string if get error
   */
  @NonNull
  public String getArchiveStringProperty(PropID propID, @Nullable Charset charset) {
    checkClosed();
    String str = nativeGetArchiveStringProperty(nativePtr, propID.ordinal());
    str = applyCharsetToString(str, charset);
    return str != null ? str : "";
  }

  /**
   * Returns the type of the property for the archive.
   *
   * @param propID the id of the property
   * @return one of {@link PropType}
   */
  public PropType getEntryPropertyType(int index, PropID propID) {
    int type = nativeGetEntryPropertyType(nativePtr, index, propID.ordinal());
    if (type >= 0 || type < PropType.values().length) {
      return PropType.values()[type];
    } else {
      return PropType.UNKNOWN;
    }
  }

  /**
   * Returns boolean property for the entry.
   *
   * @param index the index of the entry
   * @param propID the id of the property
   * @return the boolean property, {@code false} if get error
   */
  public boolean getEntryBooleanProperty(int index, PropID propID) {
    checkClosed();
    return nativeGetEntryBooleanProperty(nativePtr, index, propID.ordinal());
  }

  /**
   * Returns int property for the entry.
   *
   * @param index the index of the entry
   * @param propID the id of the property
   * @return the int property, {@code 0} if get error
   */
  public int getEntryIntProperty(int index, PropID propID) {
    checkClosed();
    return nativeGetEntryIntProperty(nativePtr, index, propID.ordinal());
  }

  /**
   * Returns long property for the entry.
   *
   * @param index the index of the entry
   * @param propID the id of the property
   * @return the long property, {@code 0} if get error
   */
  public long getEntryLongProperty(int index, PropID propID) {
    checkClosed();
    return nativeGetEntryLongProperty(nativePtr, index, propID.ordinal());
  }

  /**
   * Returns string property for the entry.
   *
   * @param index the index of the entry
   * @param propID the id of the property
   * @return the string property, empty string if get error
   */
  @NonNull
  public String getEntryStringProperty(int index, PropID propID) {
    return getEntryStringProperty(index, propID, charset);
  }

  /**
   * Returns string property for the entry.
   *
   * @param index the index of the entry
   * @param propID the id of the property
   * @param charset the charset of the string, {@code null} to let p7zip handle it
   * @return the string property, empty string if get error
   */
  @NonNull
  public String getEntryStringProperty(int index, PropID propID, @Nullable Charset charset) {
    checkClosed();
    String str = nativeGetEntryStringProperty(nativePtr, index, propID.ordinal());
    str = applyCharsetToString(str, charset);
    return str != null ? str : "";
  }

  /**
   * Returns the path of the entry.
   *
   * @param index the index of the entry
   * @return the path, empty string if get error
   */
  @NonNull
  public String getEntryPath(int index) {
    return getEntryPath(index, charset);
  }

  /**
   * Returns the path of the entry.
   *
   * @param index the index of the entry
   * @param charset the charset of the string, {@code null} to let p7zip handle it
   * @return the path, empty string if get error
   */
  @NonNull
  public String getEntryPath(int index, @Nullable Charset charset) {
    return getEntryStringProperty(index, PropID.PATH, charset);
  }

  /**
   * Extracts the context of the entry into the output stream.
   *
   * @param index the index of the entry
   * @param os the output steam to receive the content,
   *           it will be closed at the end of this method
   * @throws ArchiveException if get error
   */
  public void extractEntry(int index, @NonNull OutputStream os) throws ArchiveException {
    extractEntry(index, password, os);
  }

  /**
   * Extracts the context of the entry into the output stream.
   *
   * @param index the index of the entry
   * @param password the password of the entry
   * @param os the output steam to receive the content,
   *           it will be closed at the end of this method
   * @throws ArchiveException if get error
   */
  @SuppressWarnings("ThrowFromFinallyBlock")
  public void extractEntry(int index, String password, @NonNull OutputStream os) throws ArchiveException {
    try {
      checkClosed();
      nativeExtractEntry(nativePtr, index, password, os);
    } finally {
      try {
        os.close();
      } catch (IOException e) {
        throw new ArchiveException("Catch IOException while closing the OutputStream", e);
      }
    }
  }

  @Override
  public void close() {
    if (nativePtr != 0) {
      nativeClose(nativePtr);
      nativePtr = 0;
    }
  }

  /**
   * Returns {@code true} if the archive is closed.
   */
  public boolean isClosed() {
    return nativePtr == 0;
  }

  @NonNull
  public static InArchive create(InStream stream) throws ArchiveException {
    return create(stream, null, null);
  }

  @NonNull
  public static InArchive create(
      InStream stream,
      @Nullable Charset charset,
      @Nullable String password
  ) throws ArchiveException {
    long nativePtr = nativeCreate(stream, password);

    if (nativePtr == 0) {
      // It should not be 0
      throw new ArchiveException("a7zip is buggy");
    }

    return new InArchive(nativePtr, charset, password);
  }

  private static native long nativeCreate(InStream stream, String password) throws ArchiveException;

  private static native String nativeGetFormatName(long nativePtr);

  private static native int nativeGetNumberOfEntries(long nativePtr);

  private static native int nativeGetArchivePropertyType(long nativePtr, int propID);

  private static native boolean nativeGetArchiveBooleanProperty(long nativePtr, int propID);

  private static native int nativeGetArchiveIntProperty(long nativePtr, int propID);

  private static native long nativeGetArchiveLongProperty(long nativePtr, int propID);

  @Nullable
  private static native String nativeGetArchiveStringProperty(long nativePtr, int propID);

  private static native int nativeGetEntryPropertyType(long nativePtr, int index, int propID);

  private static native boolean nativeGetEntryBooleanProperty(long nativePtr, int index, int propID);

  private static native int nativeGetEntryIntProperty(long nativePtr, int index, int propID);

  private static native long nativeGetEntryLongProperty(long nativePtr, int index, int propID);

  @Nullable
  private static native String nativeGetEntryStringProperty(long nativePtr, int index, int propID);

  private static native void nativeExtractEntry(long nativePtr, int index, String password, OutputStream os) throws ArchiveException;

  private static native void nativeClose(long nativePtr);
}
