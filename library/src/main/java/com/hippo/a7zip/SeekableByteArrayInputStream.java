package com.hippo.a7zip;

import java.io.IOException;

public class SeekableByteArrayInputStream extends SeekableInputStream {
    private byte[] bytes;
    private int pos;

    public SeekableByteArrayInputStream(byte[] bytes) {
        this.bytes = bytes;
        pos = 0;
    }

    @Override
    public void seek(long pos) throws IOException {
        if (pos > bytes.length) {
            throw new IOException();
        }

        this.pos = (int) pos;
    }

    @Override
    public long tell() {
        return pos;
    }

    @Override
    public long size() {
        return bytes.length;
    }

    @Override
    public int read() throws IOException {
        byte[] buffer = new byte[1];

        return read(buffer) == 1 ? buffer[0] : -1;
    }

    @Override
    public int read(byte[] b, int off, int len) {
        len = pos + len > bytes.length ? bytes.length - pos : len;
        for (int i = 0; i < len; i++) {
            b[off + i] = bytes[pos + i];
        }
        pos += len;
        return len;
    }

    @Override
    public void close() {
        bytes = null;
    }
}
