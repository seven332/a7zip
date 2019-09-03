package com.hippo.a7zip;

import java.io.IOException;
import java.io.OutputStream;

class NativeOutputStream extends OutputStream implements InternalOutputStream {
    private OutputStream stream;

    NativeOutputStream(OutputStream stream) {
        this.stream = stream;
    }

    @Override
    public void close() throws IOException {
        stream.close();
    }

    @Override
    public void write(int b) throws IOException {
        stream.write(b);
    }

    /**
     * The same as {@link java.io.OutputStream#write(byte[], int, int)}.
     */
    public void write(byte[] b, int off, int len) throws IOException {
        stream.write(b, off, len);
    }
}
