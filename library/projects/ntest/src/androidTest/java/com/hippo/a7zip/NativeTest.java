/*
 * Copyright 2020 Hippo Seven
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

import android.support.test.InstrumentationRegistry;
import android.util.Log;
import com.getkeepsafe.relinker.ReLinker;
import java.io.File;
import java.io.IOException;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.junit.BeforeClass;
import org.junit.Test;

public class NativeTest {

  private static final String LOG_TAG = NativeTest.class.getSimpleName();

  @BeforeClass
  public static void beforeClass() {
    ReLinker.loadLibrary(InstrumentationRegistry.getContext(), "a7zip-test");
  }

  @SuppressWarnings("deprecation")
  @Test
  public void testNative() throws IOException {
    File tempDir = InstrumentationRegistry.getContext().getCacheDir();
    File logFile = new File(tempDir, "native_test.xml");

    int code = nativeTest(logFile.getCanonicalPath());
    String error = readXmlLog(logFile);

    if (code != 0) {
      if (error.isEmpty()) {
        error = "Can't read error log";
      }
      throw new NativeTestFailure(error);
    } else if (!error.isEmpty()) {
      Log.d(LOG_TAG, error);
    }
  }

  private String readXmlLog(File logFile) throws IOException {
    StringBuilder builder = new StringBuilder();

    Document document = Jsoup.parse(logFile, "utf-8");

    Elements elements = document.getElementsByTag("testsuites");
    if (elements != null && !elements.isEmpty()) {
      Element element = elements.first();
      builder.append("tests: ").append(element.attr("tests"))
          .append(", failures: ").append(element.attr("failures"))
          .append(", disabled: ").append(element.attr("disabled"))
          .append(", errors: ").append(element.attr("errors")).append("\n\n");
    }

    for (Element testSuite : document.getElementsByTag("testsuite")) {
      String testSuiteName = testSuite.attr("name");
      for (Element testCase : testSuite.getElementsByTag("testcase")) {
        String testCaseName = testCase.attr("name");
        for (Element failure : testCase.getElementsByTag("failure")) {
          String message = failure.attr("message");
          builder.append("TestSuite: ").append(testSuiteName).append(", TestCase: ").append(testCaseName).append("\n");
          builder.append(message).append("\n\n");
        }
      }
    }

    return builder.toString();
  }

  private static native int nativeTest(String logFile);

  private static class NativeTestFailure extends IllegalStateException {
    public NativeTestFailure(String message) {
      super(message);
    }
  }
}
