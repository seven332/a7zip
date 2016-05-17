package com.hippo.a7zip.example;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import net.sf.sevenzipjbinding.ArchiveFormat;
import net.sf.sevenzipjbinding.IInArchive;
import net.sf.sevenzipjbinding.IOutCreateArchive;
import net.sf.sevenzipjbinding.IOutCreateCallback;
import net.sf.sevenzipjbinding.IOutItemBase;
import net.sf.sevenzipjbinding.ISequentialInStream;
import net.sf.sevenzipjbinding.SevenZip;
import net.sf.sevenzipjbinding.SevenZipException;
import net.sf.sevenzipjbinding.impl.OutItemFactory;
import net.sf.sevenzipjbinding.impl.RandomAccessFileInStream;
import net.sf.sevenzipjbinding.impl.RandomAccessFileOutStream;
import net.sf.sevenzipjbinding.simple.ISimpleInArchive;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Random;

public class MainActivity extends AppCompatActivity implements Runnable {

    private LogView mLogView;
    private Random mRandom;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mLogView = (LogView) findViewById(R.id.log);
        mRandom = new Random(System.currentTimeMillis());

        // Start test
        new Thread(this).start();
    }

    // Test body
    @Override
    public void run() {
        try {
            test();
            mLogView.println("PASSED");
        } catch (Throwable e) {
            mLogView.println(Log.ERROR, "FAILED", e);
        }

        mLogView.println("Clean up");
        deleteFile(new File(getFilesDir(), "test"));
    }

    private void test() throws Exception {
        mLogView.println("load lib");
        SevenZip.initSevenZipFromPlatformJAR();

        SevenZip.Version version = SevenZip.getSevenZipVersion();
        mLogView.println("7-Zip version: " + version.version);
        mLogView.println("7-Zip date: " + version.date);
        mLogView.println("7-Zip copyright: " + version.copyright);
        mLogView.println("7-Zip-JBinding version: " + SevenZip.getSevenZipJBindingVersion());

        mLogView.println("Create test dir");
        File testDir = createDir(getFilesDir(), "test");

        mLogView.println("Create random file dir");
        File randomDir = createDir(testDir, "random");
        mLogView.println("Create random file");
        File randomFile = createRandomFile(randomDir);

        for (ArchiveFormat format : ArchiveFormat.values()) {

            //if (format == ArchiveFormat.SEVEN_ZIP) {
            //    continue;
            //}

            testArchiveFormat(format, testDir, randomFile);
        }
    }

    private void testArchiveFormat(ArchiveFormat format, File testDir,
            File randomFile) throws IOException {
        mLogView.println("======== Test " + format.getMethodName() + " ========");

        if (!format.isOutArchiveSupported()) {
            mLogView.println(format.getMethodName() + " doesn't support out archive, skip test");
            return;
        }

        mLogView.println("Create test dir");
        File dir = createDir(testDir, format.getMethodName());

        mLogView.println("Create archive");
        File archive = new File(dir, "archive");
        RandomAccessFile raf = new RandomAccessFile(archive, "rw");
        IOutCreateArchive oArchive = SevenZip.openOutArchive(format);
        oArchive.createArchive(new RandomAccessFileOutStream(raf), 1, new CreateCallback(randomFile));
        oArchive.close();
        raf.close();

        mLogView.println("Extract archive");
        raf = new RandomAccessFile(archive, "r");
        IInArchive iArchive = SevenZip.openInArchive(format, new RandomAccessFileInStream(raf));
        ISimpleInArchive isArchive = iArchive.getSimpleInterface();
        File file = new File(dir, "file");
        isArchive.getArchiveItem(0).extractSlow(new RandomAccessFileOutStream(new RandomAccessFile(file, "rw")));

        mLogView.println("Compare file");
        compareFiles(randomFile, file);
    }

    class CreateCallback implements IOutCreateCallback<IOutItemBase> {

        private final File mRandomFile;

        public CreateCallback(File randomFile) {
            mRandomFile = randomFile;
        }

        @Override
        public void setOperationResult(boolean operationResultOk) throws SevenZipException {

        }

        @Override
        public IOutItemBase getItemInformation(int index, OutItemFactory<IOutItemBase> outItemFactory) throws SevenZipException {
            IOutItemBase outItem = outItemFactory.createOutItem();

            outItem.setDataSize(mRandomFile.length());
            setPropertyPath(outItem, mRandomFile.getName());

            // To get u+rw permissions on linux, if extracting with "unzip"
            // outItem.setPropertyAttributes(Integer.valueOf(0x81808000));
            return outItem;
        }

        public void setPropertyPath(IOutItemBase base, String name) {
            try {
                Method method = base.getClass().getMethod("setPropertyPath", String.class);
                method.invoke(base, name);
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public ISequentialInStream getStream(int index) throws SevenZipException {
            try {
                return new RandomAccessFileInStream(new RandomAccessFile(mRandomFile, "r"));
            } catch (FileNotFoundException e) {
                throw new SevenZipException(e);
            }
        }

        @Override
        public void setTotal(long total) throws SevenZipException {

        }

        @Override
        public void setCompleted(long complete) throws SevenZipException {

        }
    }

    private File createDir(File parent, String dirname) throws IOException {
        File dir = new File(parent, dirname);
        if (!dir.isDirectory() && !dir.mkdirs()) {
            throw new IOException("Can't create test folder");
        }
        return dir;
    }

    private File createRandomFile(File dir) throws IOException {
        File file = new File(dir, randomFilename());
        OutputStream os = new FileOutputStream(file);
        byte[] buffer = new byte[1024 * 8]; // 8KB

        for (int i = 10 + mRandom.nextInt(500); i > 0; i--) {
            mRandom.nextBytes(buffer);
            os.write(buffer);
        }
        os.flush();
        os.close();

        return file;
    }

    private String randomFilename() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10; i++) {
            sb.append(randomChar());
        }
        return sb.toString();
    }

    private char randomChar() {
        return (char) ('A' + mRandom.nextInt('Z' - 'A'));
    }

    private void compareFiles(File file1, File file2) throws IOException {
        InputStream is1 = new BufferedInputStream(new FileInputStream(file1));
        InputStream is2 = new BufferedInputStream(new FileInputStream(file2));

        int ch = is1.read();
        while (-1 != ch) {
            int ch2 = is2.read();
            if (ch != ch2) {
                throw new IOException("file1 != file2");
            }
            ch = is1.read();
        }

        int ch2 = is2.read();
        if (ch2 != -1) {
            throw new IOException("file1 != file2");
        }
    }

    private boolean deleteFile(File file) {
        if (file == null) {
            return false;
        }
        boolean success = true;
        File[] files = file.listFiles();
        if (files != null) {
            for (File f : files) {
                success &= deleteFile(f);
            }
        }
        success &= file.delete();
        return success;
    }
}
