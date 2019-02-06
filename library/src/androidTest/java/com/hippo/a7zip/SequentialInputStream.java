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
import java.io.IOException;
import java.io.InputStream;

public class SequentialInputStream extends InputStream {

  private SequentialInStream stream;

  public SequentialInputStream(SequentialInStream stream) {
    this.stream = stream;
  }

  @Override
  public int read() throws IOException {
    byte[] b = new byte[1];
    if (stream.read(b, 0, 1) != 1) {
      return -1;
    }
    return b[0];
  }

  @Override
  public int read(@NonNull byte[] b) throws IOException {
    return stream.read(b, 0, b.length);
  }

  @Override
  public int read(@NonNull byte[] b, int off, int len) throws IOException {
    return stream.read(b, off, len);
  }
}
