package net.sf.sevenzipjbinding.junit.snippets;

import net.sf.sevenzipjbinding.ArchiveFormat;
import net.sf.sevenzipjbinding.IInArchive;
import net.sf.sevenzipjbinding.SevenZip;
import net.sf.sevenzipjbinding.impl.RandomAccessFileInStream;
import net.sf.sevenzipjbinding.junit.JUnitNativeTestBase;

import org.junit.Test;

import java.io.File;
import java.io.RandomAccessFile;

import static org.junit.Assert.assertEquals;

public class GetNumberOfItemInArchive extends JUnitNativeTestBase {
    @Test
    public void snippetRunner() throws Exception {
        assertEquals(4, getNumberOfItemsInArchive(new File(getDataDir(), "snippets/simple.zip").getPath()));
    }

    /* BEGIN_SNIPPET(GetNumberOfItemsInArchive) */
    private int getNumberOfItemsInArchive(String archiveFile) throws Exception {
        IInArchive archive;
        RandomAccessFile randomAccessFile;

        randomAccessFile = new RandomAccessFile(archiveFile, "r");

        archive = SevenZip.openInArchive(ArchiveFormat.ZIP, // null - autodetect
                new RandomAccessFileInStream(//
                        randomAccessFile));

        int numberOfItems = archive.getNumberOfItems();

        archive.close();
        randomAccessFile.close();

        return numberOfItems;
    }
    /* END_SNIPPET */
}
