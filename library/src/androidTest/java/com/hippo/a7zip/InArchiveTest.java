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

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import android.support.test.InstrumentationRegistry;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;
import okio.Okio;
import org.junit.BeforeClass;
import org.junit.Test;

public class InArchiveTest extends BaseTestCase {

  @BeforeClass
  public static void beforeClass() {
    A7Zip.initialize(InstrumentationRegistry.getContext());
  }

  @Test
  public void testZip() throws IOException, ArchiveException {
    testArchive("zip.zip");
  }

  @Test
  public void test7z() throws IOException, ArchiveException {
    testArchive("7z.7z");
  }

  @Test
  public void testRar() throws IOException, ArchiveException {
    testArchive("rar.rar");
  }

  @Test
  public void testRar5() throws IOException, ArchiveException {
    testArchive("rar5.rar");
  }

  private void testArchive(String name) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name)) {
      int size = archive.getNumberOfEntries();

      // Check path
      String[] paths = new String[size];
      for (int i = 0; i < size; i++) {
        paths[i] = archive.getEntryPath(i);
      }
      Arrays.sort(paths);
      assertArrayEquals(new String[] {
          "dump.txt",
          "empty.txt",
          "folder",
          "folder/dump.txt",
          "folder/empty.txt",
      }, paths);

      // Check content
      for (int i = 0; i < size; i++) {
        String path = archive.getEntryPath(i);
        if ("folder".equals(path)) {
          assertTrue(archive.getEntryBooleanProperty(i, PropID.IS_DIR));
          continue;
        }
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        archive.extractEntry(i, os);
        String content = new String(os.toByteArray(), "UTF-8");
        switch (path) {
          case "dump.txt":
          case "folder/dump.txt":
            assertEquals("dump", content);
            break;
          default:
            assertEquals("", content);
            break;
        }
      }
    }
  }

  private InArchive openInArchiveFromAsset(String name) throws IOException, ArchiveException {
    return InArchive.create(Okio.store(getAsset(name)));
  }
}
