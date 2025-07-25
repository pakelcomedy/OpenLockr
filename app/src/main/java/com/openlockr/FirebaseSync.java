// app/src/main/java/com/openlockr/FirebaseSync.java
package com.openlockr;

import androidx.annotation.NonNull;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.firebase.FirebaseApp;
import com.google.firebase.firestore.DocumentSnapshot;
import com.google.firebase.firestore.FirebaseFirestore;
import com.google.firebase.firestore.SetOptions;

import java.util.HashMap;
import java.util.Map;

/**
 * Singleton helper to sync OpenLockr entries with Firebase Firestore.
 * Uses a collection named "entries", documents keyed by entry ID.
 */
public class FirebaseSync {

    public interface SyncCallback {
        /**
         * Called when an operation succeeds.
         *
         * @param cipher Base64 ciphertext (for downloads), or null for uploads.
         */
        void onSuccess(String cipher);

        /**
         * Called when an operation fails.
         *
         * @param e Exception describing the failure.
         */
        void onFailure(Exception e);
    }

    private static final String COLLECTION = "entries";
    private static FirebaseSync instance;
    private final FirebaseFirestore db;

    private FirebaseSync() {
        // Ensure Firebase is initialized
        FirebaseApp.initializeApp(App.getContext());
        db = FirebaseFirestore.getInstance();
    }

    /** Get the singleton instance */
    public static synchronized FirebaseSync getInstance() {
        if (instance == null) {
            instance = new FirebaseSync();
        }
        return instance;
    }

    /**
     * Uploads or updates an entry to Firestore.
     *
     * @param id      Unique entry ID (document key).
     * @param cipher  Base64-encoded ciphertext.
     * @param callback Callback for success/failure.
     */
    public void uploadEntry(@NonNull String id,
                            @NonNull String cipher,
                            final SyncCallback callback) {
        // Prepare document data
        Map<String, Object> data = new HashMap<>();
        data.put("cipher", cipher);
        data.put("timestamp", System.currentTimeMillis());

        db.collection(COLLECTION)
          .document(id)
          .set(data, SetOptions.merge())
          .addOnSuccessListener(new OnSuccessListener<Void>() {
              @Override
              public void onSuccess(Void unused) {
                  if (callback != null) {
                      callback.onSuccess(null);
                  }
              }
          })
          .addOnFailureListener(new OnFailureListener() {
              @Override
              public void onFailure(@NonNull Exception e) {
                  if (callback != null) {
                      callback.onFailure(e);
                  }
              }
          });
    }

    /**
     * Downloads an entry from Firestore by ID.
     *
     * @param id       Unique entry ID (document key).
     * @param callback Callback delivering plaintext ciphertext or error.
     */
    public void downloadEntry(@NonNull String id, final SyncCallback callback) {
        db.collection(COLLECTION)
          .document(id)
          .get()
          .addOnSuccessListener(new OnSuccessListener<DocumentSnapshot>() {
              @Override
              public void onSuccess(DocumentSnapshot snapshot) {
                  if (!snapshot.exists()) {
                      if (callback != null) {
                          callback.onFailure(
                              new Exception("Entry not found on server"));
                      }
                      return;
                  }
                  String cipher = snapshot.getString("cipher");
                  if (cipher == null) {
                      if (callback != null) {
                          callback.onFailure(
                              new Exception("Malformed entry data"));
                      }
                  } else {
                      if (callback != null) {
                          callback.onSuccess(cipher);
                      }
                  }
              }
          })
          .addOnFailureListener(new OnFailureListener() {
              @Override
              public void onFailure(@NonNull Exception e) {
                  if (callback != null) {
                      callback.onFailure(e);
                  }
              }
          });
    }
}
