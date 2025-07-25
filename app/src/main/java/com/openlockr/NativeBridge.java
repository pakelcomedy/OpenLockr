// app/src/main/java/com/openlockr/NativeBridge.java
package com.openlockr;

/**
 * JNI bridge between Java/Kotlin and native OpenLockr core (C).
 * Exposes synchronous core functions and wraps async loadEntry with a callback.
 */
public class NativeBridge {
    static {
        System.loadLibrary("openlockr");
    }

    /**
     * Callback interface for asynchronous loadEntry().
     */
    public interface LoadCallback {
        /**
         * Called when loadEntry succeeded.
         *
         * @param plain UTF‑8 decrypted plaintext.
         */
        void onSuccess(String plain);

        /**
         * Called when loadEntry failed.
         *
         * @param errorCode One of OLKR_ERR_* codes from native core.
         */
        void onFailure(int errorCode);
    }

    /** Initialize native core with master password. Must be called before others. */
    public int init(String masterPassword) {
        return nativeInit(masterPassword);
    }

    /** Encrypt plaintext → Base64 ciphertext. */
    public String lock(String plain) {
        return nativeLock(plain);
    }

    /** Decrypt Base64 ciphertext → plaintext. */
    public String unlock(String cipher) {
        return nativeUnlock(cipher);
    }

    /** Save an entry (id + Base64 cipher) locally and sync to Firestore. */
    public int saveEntry(String id, String b64Cipher) {
        return nativeSaveEntry(id, b64Cipher);
    }

    /**
     * Asynchronously load & decrypt an entry by id.
     * Attempts local DB first, then Firestore if missing.
     */
    public void loadEntry(String id, LoadCallback callback) {
        new Thread(() -> {
            // Call native synchronous load
            PlainResult result = nativeLoadEntry(id);
            if (result.errorCode == 0) {
                callback.onSuccess(result.plain);
            } else {
                callback.onFailure(result.errorCode);
            }
        }).start();
    }

    /** Clean up native core (closes DB, zeroes key material). */
    public void cleanup() {
        nativeCleanup();
    }

    // --------------------
    // Native method declarations
    // --------------------

    // Returns OLKR_OK or OLKR_ERR_*
    private native int nativeInit(String masterPassword);

    // Returns malloc'd Java String or null on error
    private native String nativeLock(String plain);

    // Returns malloc'd Java String or null on error
    private native String nativeUnlock(String cipher);

    // Returns OLKR_OK or OLKR_ERR_*
    private native int nativeSaveEntry(String id, String b64Cipher);

    // Synchronous native load: returns PlainResult holding code + string
    private native PlainResult nativeLoadEntry(String id);

    // Clean up native state
    private native void nativeCleanup();

    /**
     * Helper class for receiving both error code and plaintext
     * from nativeLoadEntry.
     */
    public static class PlainResult {
        public int    errorCode;
        public String plain;
        // JNI side must fill these two fields
    }
}