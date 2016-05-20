package net.sf.sevenzipjbinding.junit;

import android.content.Context;
import android.content.res.AssetManager;
import android.support.test.InstrumentationRegistry;
import android.test.RenamingDelegatingContext;

import net.sf.sevenzipjbinding.SevenZip;
import net.sf.sevenzipjbinding.SevenZipException;
import net.sf.sevenzipjbinding.SevenZipNativeInitializationException;
import net.sf.sevenzipjbinding.junit.tools.SevenZipDebug;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.Date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

/**
 * This is the base class for all JUnit test classes, that needs native library to be loaded. This class provides:<br>
 * - Initialization of the native library
 *
 * @author Boris Brodski
 * @since 4.65-1
 *
 */
public class JUnitNativeTestBase extends JUnitTestBase {
    protected interface RunnableThrowsException {
        public void run() throws Exception;
    }

    private static final int SINGLE_TEST_THREAD_COUNT = 4;//15;
    protected static final int SINGLE_TEST_REPEAT_COUNT = 3;//60;
    static final int SINGLE_TEST_TIMEOUT = 100000;

    private static int deadCPPObjectCount = 0;
    private static boolean initializeNativeLibrary = true;
    private static boolean nonDebugLibraryWasReported = false;
    private static boolean nonUseMyAssertLibraryWasReported = false;

    private static boolean initializeAssets = true;

    Context mMockContext;


    @Rule
    public CloseableRule closeableRule = new CloseableRule();

    @Rule
    public SevenZipExceptionStackTracePrinterRule stackTracePrinterRule = new SevenZipExceptionStackTracePrinterRule();

    /**
     * Initialize native SevenZipJBinding library for all JUnit tests
     *
     * @throws SevenZipNativeInitializationException
     *             in case initialization of SevenZipJBinding fails
     */
    @BeforeClass
    public static void initializeSevenZipJBinding() throws SevenZipNativeInitializationException {
        if (initializeNativeLibrary) {
            SevenZip.initSevenZipFromPlatformJAR();
            initializeNativeLibrary = false;
        }
    }

    public File getDataDir() {
        File file = mMockContext.getExternalFilesDir("testdata");
        Assert.assertNotNull(file);
        file.mkdirs();
        return file;
    }

    public void copy(AssetManager assetManager, String path) {
        String[] files;
        try {
            files = assetManager.list(path);
        } catch (IOException e) {
            throw new IllegalStateException("Failed to get asset file list.", e);
        }

        if (files != null && files.length > 0) {

            System.out.println(Arrays.toString(files));

            File dir = getDataDir();
            for (String filename : files) {

                String filePath = (path.length() == 0 ? "" : (path + "/")) + filename;

                System.out.println("Copy " + filePath);

                InputStream in;
                OutputStream out = null;

                try {
                    in = assetManager.open(filePath);
                } catch (IOException e) {
                    // It is a dir
                    copy(assetManager, filePath);
                    continue;
                }

                try {
                    File outFile = new File(dir, filePath);
                    outFile.getParentFile().mkdirs();
                    out = new FileOutputStream(outFile);
                    copyFile(in, out);
                } catch (IOException e) {
                    // Ignore
                } finally {
                    close(in);
                    close(out);
                }
            }
        }
    }

    @Before
    public void copyAssets() {
        mMockContext = new RenamingDelegatingContext(InstrumentationRegistry.getInstrumentation().getTargetContext(), "test_");

        if (!initializeAssets) {
            return;
        }

        initializeAssets = false;

        AssetManager assetManager = mMockContext.getAssets();
        Assert.assertNotNull(assetManager);

        copy(assetManager, "bug");
        copy(assetManager, "encoding");
        copy(assetManager, "multiple-files");
        copy(assetManager, "simple");
        copy(assetManager, "snippets");
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1){
            out.write(buffer, 0, read);
        }
    }

    private static void close(Closeable closeable) {
        if (closeable != null) {
            try {
                closeable.close();
            } catch (IOException e) {
                // Ignore
            }
        }
    }

    @After
    public void afterTest() {
        closeableRule.closeAll();
        try {
            int objectCount = SevenZipDebug.getCPPObjectCount();
            int newDeadObjectCount = objectCount - deadCPPObjectCount;
            if (newDeadObjectCount != 0) {
                SevenZipDebug.printCPPObjects();
                deadCPPObjectCount = objectCount;
            }
            assertEquals("Not all CPP Objects was freed", 0, newDeadObjectCount);
        } catch (UnsatisfiedLinkError e) {
            if (!nonDebugLibraryWasReported) {
                System.out.println("WARNING! SevenZip native libraray was build"
                        + " without object tracing debug function.");
                nonDebugLibraryWasReported = true;
            }
        }
        try {
            int threadCount = SevenZipDebug.getAttachedThreadCount();
            assertEquals("Not all attached thread was detached from VM", 0, threadCount);
        } catch (UnsatisfiedLinkError e) {
            if (!nonUseMyAssertLibraryWasReported) {
                System.out.println("WARNING! SevenZip native libraray was build without support for MY_ASSERTs.");
                nonUseMyAssertLibraryWasReported = true;
            }
        }
    }

    protected void runMultithreaded(final RunnableThrowsException runnable,
            final Class<? extends Exception> exceptionToBeExpected) throws Exception {
        runMultithreaded(runnable, exceptionToBeExpected, SINGLE_TEST_THREAD_COUNT, SINGLE_TEST_TIMEOUT
                * SINGLE_TEST_REPEAT_COUNT);
    }

    protected void runMultithreaded(final RunnableThrowsException runnable,
            final Class<? extends Exception> exceptionToBeExpected, int threadCount, int threadTimeout)
            throws Exception {
        final int[] threadsFinished = new int[] { threadCount };
        final Throwable[] firstThrowableArray = new Throwable[] { null };
        final Throwable[] firstExpectedThrowableArray = new Throwable[] { null };
        for (int i = 0; i < threadCount; i++) {
            new Thread(new Runnable() {
                public void run() {
                    try {
                        runnable.run();
                        if (exceptionToBeExpected != null) {
                            throw new Exception("Expected exception wasn't thrown: "
                                    + exceptionToBeExpected.getCanonicalName());
                        }
                    } catch (Throwable e) {
                        synchronized (firstThrowableArray) {
                            boolean wasExceptionExpected = exceptionToBeExpected != null
                                    && exceptionToBeExpected.isAssignableFrom(e.getClass());
                            if (firstThrowableArray[0] == null //
                                    && !wasExceptionExpected) {
                                firstThrowableArray[0] = e;
                            }
                            if (wasExceptionExpected) {
                                firstExpectedThrowableArray[0] = e;
                            }
                        }
                    } finally {
                        synchronized (JUnitNativeTestBase.this) {
                            try {
                                threadsFinished[0]--;
                                JUnitNativeTestBase.this.notify();
                            } catch (Throwable throwable) {
                                throwable.printStackTrace();
                            }
                        }
                    }
                }
            }).start();
        }
        long start = System.currentTimeMillis();
        synchronized (this) {
            while (true) {
                try {
                    if (threadsFinished[0] == 0) {
                        break;
                    }
                    wait(threadCount * threadTimeout);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (System.currentTimeMillis() - start > threadTimeout) {
                    fail("Time out");
                }
            }
        }
        Throwable firstThrowable = firstThrowableArray[0];
        if (firstThrowable != null) {
            if (firstThrowable instanceof SevenZipException) {
                throw (SevenZipException) firstThrowable;
            }
            System.out.println(firstThrowable);
            throw new RuntimeException("Exception in underlying thread", firstThrowable);
        }
        Throwable firstExpectedThrowable = firstExpectedThrowableArray[0];
        if (firstExpectedThrowable != null) {
            if (firstExpectedThrowable instanceof Exception) {
                throw (Exception) firstExpectedThrowable;
            }
            throw (Error) firstExpectedThrowable;
        }
    }

    protected final void testSingleOrMultithreaded(boolean multithreaded, final RunnableThrowsException test)
            throws Exception {
        if (multithreaded) {
            runMultithreaded(new RunnableThrowsException() {
                public void run() throws Exception {
                    testSingleOrMultithreaded(false, test);
                }
            }, null);
        } else {
            for (int i = 0; i < SINGLE_TEST_REPEAT_COUNT; i++) {
                test.run();
            }
        }
    }

    /**
     * Add closeable to be closed automatically at the end of the test.
     *
     * @param closeable
     *            closeable
     */
    public void addCloseable(Closeable closeable) {
        closeableRule.addCloseable(closeable);
    }

    /**
     * Remove closeable from the list.
     *
     * @param closeable
     *            closeable to remove
     */
    public void removeCloseable(Closeable closeable) {
        closeableRule.removeCloseable(closeable);
    }

    /**
     * Add closeable to be closed automatically at the end of the test.
     *
     * @param <T>
     *            Concrete type of closeable
     * @param closeable
     *            closeable
     * @return <code>closeable</code> parameter allowing call chains
     */
    public <T extends Closeable> T closeLater(T closeable) {
        closeableRule.addCloseable(closeable);
        return closeable;
    }

    public static Date getDate(int period) {
        return new Date(new Date().getTime() - RANDOM.nextInt(period) - period);
    }

    @SuppressWarnings("unchecked")
    protected <T extends Throwable> T getExceptionCauseByClass(Class<T> exceptionClass, Throwable e) {
        Throwable cause = e;
        while (cause != null) {
            if (exceptionClass.isInstance(cause)) {
                return (T) cause;
            }
            cause = cause.getCause();
        }
        e.printStackTrace();
        throw new AssertionError("Exception " + exceptionClass.getName() + " is missing as a cause of the exception: "
                + e.getMessage());
    }

}
