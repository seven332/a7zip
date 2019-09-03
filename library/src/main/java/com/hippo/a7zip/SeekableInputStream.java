package com.hippo.a7zip;

import java.io.IOException;
import java.io.InputStream;

public abstract class SeekableInputStream extends InputStream implements InternalSeekableInputStream {
    /**
     * Sets the position, measured from the beginning,
     * at which the next read occurs. The offset may be
     * set beyond the end of the file.
     */
    public abstract void seek(long pos) throws IOException;

    /**
     * Returns current position, measured from the beginning.
     */
    public abstract long tell() throws IOException;

    /**
     * Returns the size.
     */
    public abstract long size() throws IOException;
}
