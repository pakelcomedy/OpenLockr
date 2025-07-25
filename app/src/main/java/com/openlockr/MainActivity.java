// app/src/main/java/com/openlockr/MainActivity.java
package com.openlockr;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.MainThread;
import androidx.appcompat.app.AppCompatActivity;

/**
 * MainActivity for OpenLockr.
 * - Encrypt & save entries via native OpenLockr core + Firestore upload
 * - Load & decrypt entries from local or Firestore
 */
public class MainActivity extends AppCompatActivity {

    private EditText editTextId;
    private EditText editTextInput;
    private TextView textViewOutput;
    private Button buttonEncryptSave;
    private Button buttonLoadDecrypt;

    private NativeBridge nativeBridge;
    private FirebaseSync firebaseSync;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize UI references
        editTextId       = findViewById(R.id.editTextId);
        editTextInput    = findViewById(R.id.editTextInput);
        textViewOutput   = findViewById(R.id.textViewOutput);
        buttonEncryptSave = findViewById(R.id.buttonEncryptSave);
        buttonLoadDecrypt = findViewById(R.id.buttonLoadDecrypt);

        // Initialize native core with a master password (for demo purposes use fixed or prompt)
        nativeBridge = new NativeBridge();
        int initCode = nativeBridge.init("MySecureMasterPassword!");
        if (initCode != 0) {
            Toast.makeText(this, "Native init failed: " + initCode, Toast.LENGTH_LONG).show();
        }

        // Initialize FirebaseSync singleton
        firebaseSync = FirebaseSync.getInstance();

        // Set up button: encrypt, save locally, and upload to Firestore
        buttonEncryptSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String id    = editTextId.getText().toString().trim();
                String plain = editTextInput.getText().toString();
                if (id.isEmpty() || plain.isEmpty()) {
                    Toast.makeText(MainActivity.this, "ID and input are required", Toast.LENGTH_SHORT).show();
                    return;
                }
                // Encrypt via native
                String cipher = nativeBridge.lock(plain);
                if (cipher == null) {
                    Toast.makeText(MainActivity.this, "Encryption failed", Toast.LENGTH_SHORT).show();
                    return;
                }
                // Save locally & upload via native
                int saveCode = nativeBridge.saveEntry(id, cipher);
                if (saveCode != 0) {
                    Toast.makeText(MainActivity.this, "Save entry failed: " + saveCode, Toast.LENGTH_SHORT).show();
                    return;
                }
                // Also upload via FirebaseSync (for demo; nativeBridge may already have synced)
                firebaseSync.uploadEntry(id, cipher, new FirebaseSync.SyncCallback() {
                    @Override
                    public void onSuccess(String unused) {
                        runOnUiThread(() -> Toast.makeText(MainActivity.this, "Saved & uploaded", Toast.LENGTH_SHORT).show());
                    }

                    @Override
                    public void onFailure(Exception e) {
                        runOnUiThread(() -> Toast.makeText(MainActivity.this, "Upload failed: " + e.getMessage(), Toast.LENGTH_LONG).show());
                    }
                });
            }
        });

        // Set up button: download (if needed), load locally, decrypt, and display
        buttonLoadDecrypt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String id = editTextId.getText().toString().trim();
                if (id.isEmpty()) {
                    Toast.makeText(MainActivity.this, "ID is required", Toast.LENGTH_SHORT).show();
                    return;
                }
                // Try load & decrypt via native (which will fetch from Firestore if missing)
                nativeBridge.loadEntry(id, new NativeBridge.LoadCallback() {
                    @Override
                    public void onSuccess(String plain) {
                        runOnUiThread(() -> textViewOutput.setText(plain));
                    }

                    @Override
                    public void onFailure(int errorCode) {
                        runOnUiThread(() -> Toast.makeText(MainActivity.this,
                                "Load/decrypt failed: " + errorCode, Toast.LENGTH_LONG).show());
                    }
                });
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Clean up native core
        nativeBridge.cleanup();
    }
}
