# A7Zip

An Android wrapper for 7-Zip (P7ZIP).

Writing archives is not supported.

## Download

1. Add the JitPack repository to your root build.gradle.

```gradle
allprojects {
    repositories {
        ...
        maven { url 'https://jitpack.io' }
    }
}
```

2. Add A7Zip dependency to your application build.gradle.

```gradle
dependencies {
    implementation "com.github.seven332.a7zip:extract:${version}"
    // implementation "com.github.seven332.a7zip:extract-lite:${version}"
}
```

## Usage

1. Load native libraries.

```java
A7Zip.initialize(context);
```

2. Open an archive.

```java
InArchive archive = InArchive.open(new File("archive.7z"));
```

3. Operate the archive.

```java
// How many entries are there in the archive?
int number = archive.getNumberOfEntries();

for (int i = 0; i < number; i++) {
    // Get the path of the entry
    String path = archive.getEntryPath(i);
    // Extract the entry
    archive.extractEntry(i, new FileOutputStream(path));
}

// Close the archive
archive.close();
```

## Variant

7-Zip supports so many formats that you might not need them all. The codecs are in native library, ProGuard can't shrink it. Unused codecs increase the size of the final APK. So A7Zip has some variants to reduce the drawback.

Variant | groupId:artifactId | Note
---|---|---
extract-lite | com.github.seven332.a7zip:extract-lite | Open 7z, Rar, Rar5, Zip
extract | com.github.seven332.a7zip:extract | Open all formats 7-Zip supported
