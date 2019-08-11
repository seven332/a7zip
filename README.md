# A7Zip

An Android wrapper for 7-Zip (P7ZIP).

Only single volume archive extraction is supported now.

## Usage

1. Load native libraries.

```java
A7Zip.loadLibrary(A7ZipExtract.LIBRARY);
//A7Zip.loadLibrary(A7ZipExtractLite.LIBRARY);
```

2. Open an archive.

```java
InStream stream = new FileInStream("archive.7z");
InArchive archive = InArchive.open(stream);
```

3. Operate the archive.

```java
// How many entries are there in the archive?
int number = archive.getNumberOfEntries();

for (int i = 0; i < number; i++) {
    // Get the path of the entry
    String path = archive.getEntryPath(i);
    // Extract the entry
    archive.extractEntry(i, new IoSequentialOutStream(new FileOutputStream(path)));
}

// Close the archive
archive.close();
```

## Variant

7-Zip supports so many formats that you might not need them all. The codecs are in native library, ProGuard can't shrink it. Unused codecs increase the size of the final APK. So A7Zip has some variants to reduce the drawback.

Variant | Note
---|---
extract-lite | Open 7z, Rar, Rar5, Zip
extract | Open all formats 7-Zip supported
