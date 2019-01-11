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
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.util.Arrays;

public class ByteArrayOutStream implements OutStream {

  /**
   * The maximum size of array to allocate.
   * Some VMs reserve some header words in an array.
   * Attempts to allocate larger arrays may result in
   * OutOfMemoryError: Requested array size exceeds VM limit
   */
  private static final int MAX_ARRAY_SIZE = Integer.MAX_VALUE - 8;

  private byte buf[];
  private int pos = 0;
  private int size = 0;

  public ByteArrayOutStream() {
    this(32);
  }

  public ByteArrayOutStream(int size) {
    if (size < 0) {
      throw new IllegalArgumentException("Negative initial size: " + size);
    }
    buf = new byte[size];
  }

  private void ensureCapacity(int minCapacity) {
    // overflow-conscious code
    if (minCapacity - buf.length > 0) {
      grow(minCapacity);
    }
  }

  private void grow(int minCapacity) {
    // overflow-conscious code
    int oldCapacity = buf.length;
    int newCapacity = oldCapacity << 1;
    if (newCapacity - minCapacity < 0)
      newCapacity = minCapacity;
    if (newCapacity - MAX_ARRAY_SIZE > 0)
      newCapacity = hugeCapacity(minCapacity);
    buf = Arrays.copyOf(buf, newCapacity);
  }

  private static int hugeCapacity(int minCapacity) {
    if (minCapacity < 0) {
      // overflow
      throw new OutOfMemoryError();
    }
    return (minCapacity > MAX_ARRAY_SIZE) ? Integer.MAX_VALUE : MAX_ARRAY_SIZE;
  }

  @Override
  public void write(byte[] b, int off, int len) {
    if ((off < 0) || (off > b.length) || (len < 0) || ((off + len) - b.length > 0)) {
      throw new IndexOutOfBoundsException();
    }

    ensureCapacity(pos + len);
    System.arraycopy(b, off, buf, pos, len);
    pos += len;
    if (size < pos) {
      size = pos;
    }
  }

  @Override
  public void seek(long pos) {
    if (pos < 0 || pos > Integer.MAX_VALUE) {
      throw new IndexOutOfBoundsException();
    }

    this.pos = (int) pos;
  }

  @Override
  public long tell() {
    return pos;
  }

  @Override
  public long size() {
    return size;
  }

  @Override
  public void truncate(long size) {
    if (size < 0 || size > Integer.MAX_VALUE) {
      throw new IllegalArgumentException("Invalid size: " + size);
    }

    ensureCapacity((int) size);
    this.size = (int) size;
    if (this.pos > this.size) {
      this.pos = this.size;
    }
  }

  @Override
  public void close() { }

  @NonNull
  @Override
  public String toString() {
    return new String(buf, 0, size);
  }

  @NonNull
  public String toString(String charsetName) throws UnsupportedEncodingException {
    return new String(buf, 0, size, charsetName);
  }

  @NonNull
  public String toString(Charset charset) {
    return new String(buf, 0, size, charset);
  }
}
