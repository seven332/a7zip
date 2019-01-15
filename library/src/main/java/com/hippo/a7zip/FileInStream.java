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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

/**
 * Wraps {@link RandomAccessFile} as {@link InStream}.
 */
public class FileInStream implements InStream {

  private RandomAccessFile file;

  public void FileInStream(File file) throws FileNotFoundException {
    this.file = new RandomAccessFile(file, "r");
  }

  public void FileInStream(RandomAccessFile file) {
    this.file = file;
  }

  @Override
  public void seek(long pos) throws IOException {
    file.seek(pos);
  }

  @Override
  public long tell() throws IOException {
    return file.getFilePointer();
  }

  @Override
  public long size() throws IOException {
    return file.length();
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    return file.read(b, off, len);
  }

  @Override
  public void close() throws IOException {
    file.close();
  }
}
