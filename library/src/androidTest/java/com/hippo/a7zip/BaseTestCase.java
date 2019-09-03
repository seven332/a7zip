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

import androidx.test.platform.app.InstrumentationRegistry;

import org.apache.commons.io.IOUtils;
import org.junit.AfterClass;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

public class BaseTestCase {

    private static final List<File> tempCopies = new ArrayList<>();

    protected static File getAsset(String path) throws IOException {
        File of = File.createTempFile("copyFile", new File(path).getName());

        InputStream is = null;
        OutputStream os = null;
        try {
            is = InstrumentationRegistry.getInstrumentation().getTargetContext().getAssets().open(path);
            os = new FileOutputStream(of);
            IOUtils.copy(is, os);
        } finally {
            IOUtils.closeQuietly(is);
            IOUtils.closeQuietly(os);
        }

        synchronized (tempCopies) {
            tempCopies.add(of);
        }

        return of;
    }

    @AfterClass
    public static void clearTempCopies() {
        synchronized (tempCopies) {
            for (File f : tempCopies) {
                delete(f);
            }
            tempCopies.clear();
        }
    }

    private static void delete(File file) {
        if (file == null) {
            return;
        }

        File[] files = file.listFiles();
        if (files != null) {
            for (File f : files) {
                delete(f);
            }
        }

        file.delete();
    }
}
