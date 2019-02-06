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

import java.io.IOException;

class NativeInStream implements InStream {

  private long nativePtr;

  private NativeInStream(long nativePtr) {
    this.nativePtr = nativePtr;
  }

  private void checkClosed() {
    if (nativePtr == 0) {
      throw new IllegalStateException("This NativeInStream is closed.");
    }
  }

  @Override
  public void seek(long pos) throws IOException {
    checkClosed();
    nativeSeek(nativePtr, pos);
  }

  @Override
  public long tell() throws IOException {
    checkClosed();
    return nativeTell(nativePtr);
  }

  @Override
  public long size() throws IOException {
    checkClosed();
    return nativeSize(nativePtr);
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    checkClosed();
    return nativeRead(nativePtr, b, off, len);
  }

  @Override
  public void close() throws IOException {
    if (nativePtr != 0) {
      nativeClose(nativePtr);
      nativePtr = 0;
    }
  }

  private static native void nativeSeek(long nativePtr, long pos) throws IOException;

  private static native long nativeTell(long nativePtr) throws IOException;

  private static native long nativeSize(long nativePtr) throws IOException;

  private static native int nativeRead(long nativePtr, byte[] b, int off, int len) throws IOException;

  private static native void nativeClose(long nativePtr) throws IOException;
}
