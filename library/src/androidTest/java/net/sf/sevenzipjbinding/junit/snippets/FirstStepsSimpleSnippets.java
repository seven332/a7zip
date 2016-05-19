package net.sf.sevenzipjbinding.junit.snippets;

import net.sf.sevenzipjbinding.IInArchive;
import net.sf.sevenzipjbinding.SevenZip;
import net.sf.sevenzipjbinding.SevenZipException;
import net.sf.sevenzipjbinding.impl.RandomAccessFileInStream;
import net.sf.sevenzipjbinding.junit.JUnitNativeTestBase;

import org.junit.Test;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.RandomAccessFile;

import static org.junit.Assert.assertNotNull;

public class FirstStepsSimpleSnippets extends JUnitNativeTestBase {

    private IInArchive inArchive;
    private RandomAccessFile randomAccessFile;

    @Test
    public void testOpenArchiveSnippet() throws Exception {
        openArchive(new File(getDataDir(), "snippets/simple.zip").getPath());
        assertNotNull(inArchive);
        inArchive.close();
        randomAccessFile.close();
    }

    /* BEGIN_SNIPPET(SimpleOpen) */
    public void openArchive(String archiveFilename) //
            throws SevenZipException, FileNotFoundException {

        /*f*/randomAccessFile/* */= new RandomAccessFile(archiveFilename, "r");

        /*f*/inArchive/* */= SevenZip.openInArchive(null, // Choose format automatically
                new RandomAccessFileInStream(randomAccessFile));
    }
    /* END_SNIPPET */
}
