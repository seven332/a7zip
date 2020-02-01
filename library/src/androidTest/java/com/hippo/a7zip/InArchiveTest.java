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
import static org.junit.Assert.fail;

import android.support.annotation.NonNull;
import android.util.Log;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.List;
import org.apache.commons.io.IOUtils;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class InArchiveTest extends BaseTestCase {

  @Rule
  public ExpectedException thrown = ExpectedException.none();

  private List<String> supportedFormats = Arrays.asList(A7ZipTestConfig.SUPPORTED_FORMATS);
  private List<String> getStreamSupportedFormats = Arrays.asList(A7ZipTestConfig.GET_STREAM_SUPPORTED_FORMATS);

  private void checkFormat(String format) {
    if (!supportedFormats.contains(format)) {
      thrown.expect(ArchiveException.class);
      thrown.expectMessage("Unknown archive format");
    }
  }

  @Test
  public void testZip() throws IOException, ArchiveException {
    checkFormat("zip");
    testArchive("archive.zip", "zip");
  }

  @Test
  public void test7z() throws IOException, ArchiveException {
    checkFormat("7z");
    testArchive("archive.7z", "7z");
  }

  @Test
  public void testRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    testArchive("archive.rar", "Rar");
  }

  @Test
  public void testRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    testArchive("archive.rar5", "Rar5");
  }

  @Test
  public void testTar() throws IOException, ArchiveException {
    checkFormat("tar");
    testArchive("archive.tar", "tar");
  }

  @Test
  public void testWim() throws IOException, ArchiveException {
    checkFormat("wim");
    testArchive("archive.wim", "wim");
  }

  @Test
  public void testCpio() throws IOException, ArchiveException {
    checkFormat("Cpio");
    testArchive("archive.cpio", "Cpio");
  }

  private void testArchive(String name, String format) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name)) {
      int size = archive.getNumberOfEntries();

      // Check format name
      assertEquals(format, archive.getFormatName());

      // Check path
      String[] paths = new String[size];
      for (int i = 0; i < size; i++) {
        paths[i] = archive.getEntryPath(i);
      }
      Arrays.sort(paths);

      Log.d("TAG", Arrays.toString(paths));
//      Log.d("TAG", "0: " + archive.getEntryPath(0));
//      Log.d("TAG", "1: " + archive.getEntryPath(1));
//      Log.d("TAG", "2: " + archive.getEntryPath(2));


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

        String content1 = getContentByExtractingEntry(archive, i);
        assertContent(path, content1);

        if (getStreamSupportedFormats.contains(format)) {
          String content2 = getContentByGettingEntryStream(archive, i);
          assertContent(path, content2);
        } else {
          try {
            archive.getEntryStream(i);
            fail();
          } catch (ArchiveException e) {
            assertEquals(e.getMessage(), "Not implemented");
          }
        }
      }
    }
  }

  private static String getContentByExtractingEntry(InArchive archive, int index)
      throws ArchiveException, UnsupportedEncodingException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    archive.extractEntry(index, os);
    return os.toString("UTF-8");
  }

  private static String getContentByGettingEntryStream(InArchive archive, int index)
      throws IOException, ArchiveException {
    InputStream stream = archive.getEntryStream(index);
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    IOUtils.copy(stream, os);
    stream.close();
    return os.toString("UTF-8");
  }

  private static void assertContent(String path, String content) {
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

  @Test
  public void testPathZip() throws IOException, ArchiveException {
    checkFormat("zip");
    testPath("path.zip");
  }

  @Test
  public void testPath7z() throws IOException, ArchiveException {
    checkFormat("7z");
    testPath("path.7z");
  }

  @Test
  public void testPathRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    testPath("path.rar");
  }

  @Test
  public void testPathRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    testPath("path.rar5");
  }

  private void testPath(String name) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name)) {
      assertEquals("\uD83E\uDD23测试.txt", archive.getEntryPath(0));
    }
  }

  @Test
  public void testPathZipGB18030() throws IOException, ArchiveException {
    checkFormat("zip");
    try (InArchive archive = openInArchiveFromAsset("path-gb18030.zip")) {
      assertEquals("新建文本文档.txt", archive.getEntryPath(0, Charset.forName("GB18030")));
    }
  }

  @Test
  public void testCommentZipGB18030() throws IOException, ArchiveException {
    checkFormat("zip");
    try (InArchive archive = openInArchiveFromAsset("comment-gb18030.zip")) {
      assertEquals("我是注释", archive.getArchiveStringProperty(PropID.COMMENT, Charset.forName("GB18030")));
    }
  }

  @Test
  public void testCommentZipUTF8() throws IOException, ArchiveException {
    checkFormat("zip");
    try (InArchive archive = openInArchiveFromAsset("comment-utf-8.zip")) {
      assertEquals("\uFEFF我是注释", archive.getArchiveStringProperty(PropID.COMMENT, Charset.forName("UTF-8")));
    }
  }

  @Test
  public void testCommentEntryZipGB18030() throws IOException, ArchiveException {
    checkFormat("zip");
    try (InArchive archive = openInArchiveFromAsset("comment-entry-gb18030.zip")) {
      assertEquals("我是注释", archive.getEntryStringProperty(0, PropID.COMMENT, Charset.forName("GB18030")));
    }
  }

  @Test
  public void testPasswordZip() throws IOException, ArchiveException {
    checkFormat("zip");
    assertPasswordArchive("password.zip", "123456");
    assertPasswordPathArchive("password.zip", "123456");
  }

  @Test
  public void testPassword7z() throws IOException, ArchiveException {
    checkFormat("7z");
    assertPasswordArchive("password.7z", "123456");
    assertPasswordPathArchive("password.7z", "123456");
  }

  @Test
  public void testPasswordRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    assertPasswordArchive("password.rar", "123456");
    assertPasswordPathArchive("password.rar", "123456");
  }

  @Test
  public void testPasswordRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    assertPasswordArchive("password.rar5", "123456");
    assertPasswordPathArchive("password.rar5", "123456");
  }

  private void assertPasswordArchive(String name, String password) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name)) {
      assertEquals("password.txt", archive.getEntryPath(0));

      ByteArrayOutputStream os = new ByteArrayOutputStream();
      archive.extractEntry(0, password, os);
      String content = os.toString("UTF-8");
      assertEquals("password", content);
    }
  }

  @Test
  public void testPasswordPath7z() throws IOException, ArchiveException {
    checkFormat("7z");
    assertPasswordPathArchive("password-path.7z", "123456");
  }

  @Test
  public void testPasswordPathRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    assertPasswordPathArchive("password-path.rar", "123456");
  }

  @Test
  public void testPasswordPathRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    assertPasswordPathArchive("password-path.rar5", "123456");
  }

  private void assertPasswordPathArchive(String name, String password) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name, null, password)) {
      assertEquals("password.txt", archive.getEntryPath(0));

      ByteArrayOutputStream os = new ByteArrayOutputStream();
      archive.extractEntry(0, os);
      String content = os.toString("UTF-8");
      assertEquals("password", content);
    }
  }

  @Test
  public void testCreatePasswordException7z() throws IOException, ArchiveException {
    checkFormat("7z");
    assertCreatePasswordException("password-path.7z", "654321");
  }

  @Test
  public void testCreatePasswordExceptionRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    assertCreatePasswordException("password-path.rar", "654321");
  }

  @Test
  public void testCreatePasswordExceptionRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    assertCreatePasswordException("password-path.rar5", "654321");
  }

  private void assertCreatePasswordException(String name, String wrongPassword) throws IOException, ArchiveException {
    try {
      openInArchiveFromAsset(name);
      fail("Expected a PasswordException to be thrown");
    } catch (PasswordException e) {
      assertEquals("No password", e.getMessage());
    }

    try {
      openInArchiveFromAsset(name, null, wrongPassword);
      fail("Expected a PasswordException to be thrown");
    } catch (PasswordException e) {
      assertEquals("Wrong password", e.getMessage());
    }
  }

  @Test
  public void testExtractPasswordExceptionZip() throws IOException, ArchiveException {
    checkFormat("zip");
    assertExtractPasswordException("password.zip", "654321");
  }

  @Test
  public void testExtractPasswordException7z() throws IOException, ArchiveException {
    checkFormat("7z");
    assertExtractPasswordException("password.7z", "654321");
  }

  @Test
  public void testExtractPasswordExceptionRar() throws IOException, ArchiveException {
    checkFormat("Rar");
    assertExtractPasswordException("password.rar", "654321");
  }

  @Test
  public void testExtractPasswordExceptionRar5() throws IOException, ArchiveException {
    checkFormat("Rar5");
    assertExtractPasswordException("password.rar5", "654321");
  }

  private void assertExtractPasswordException(String name, String wrongPassword) throws IOException, ArchiveException {
    try (InArchive archive = openInArchiveFromAsset(name)) {
      ByteArrayOutputStream os = new ByteArrayOutputStream();

      try {
        archive.extractEntry(0, os);
        fail("Expected a PasswordException to be thrown");
      } catch (PasswordException e) {
        assertEquals("No password", e.getMessage());
      }

      try {
        archive.extractEntry(0, wrongPassword, os);
        fail("Expected a PasswordException to be thrown");
      } catch (PasswordException e) {
        assertEquals("Wrong password", e.getMessage());
      }
    }
  }

  @Test
  public void testMultiVolumeZip() throws IOException, ArchiveException {
    checkFormat("zip");
    testArchive("multi-volume.zip.001", "zip");
  }

  private InArchive openInArchiveFromAsset(String name) throws IOException, ArchiveException {
    return openInArchiveFromAsset(name, null, null);
  }

  private InArchive openInArchiveFromAsset(String name, Charset charset, String password) throws IOException, ArchiveException {
    return InArchive.open(new FileSeekableInputStream(getAsset(name)), charset, password, name, new OpenVolumeInAssetCallback());
  }

  private static class OpenVolumeInAssetCallback implements InArchive.OpenVolumeCallback {
    @NonNull
    @Override
    public SeekableInputStream openVolume(String filename) throws ArchiveException {
      try {
        return new FileSeekableInputStream(getAsset(filename));
      } catch (IOException e) {
        throw new ArchiveException("Can't open asset: " + filename, e);
      }
    }
  }
}
