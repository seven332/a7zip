package net.sf.sevenzipjbinding.junit.snippets;

import org.junit.Test;

import java.io.File;

import static org.junit.Assert.assertEquals;

/**
 * Tests {@link CompressGeneric} snippet.
 *
 * @author Boris Brodski
 * @since 9.20-2.00
 */
public class UpdateAlterItemsTest extends SnippetTest {
    private static final String SYSTEM_PROPERTY_TMP = "java.io.tmpdir";
    /* BEGIN_OUTPUT(UpdateAlterItems) */
    String expected7z = "Update successful\n";
    /* END_OUTPUT */

    private String getExpectedOutput(String expected) {
        expected = expected.replace("\n", NEW_LINE);
        expected = expected.replace('/', File.separatorChar);
        return expected;
    }

    @Test
    public void testCompress7z() {
        String tmpDir = System.getProperty(SYSTEM_PROPERTY_TMP);
        File archiveFile = new File(tmpDir, "updated-alter-items.7z");

        beginSnippetTest();
        UpdateAlterItems.main(new String[] { new File(getDataDir(), "snippets/to-update.7z").getPath(), archiveFile.getAbsolutePath() });
        String output = endSnippetTest();
        assertEquals(getExpectedOutput(expected7z), output);
    }
}
