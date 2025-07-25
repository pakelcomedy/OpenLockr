```
OpenLockr/
├── README.md
├── build.gradle
├── settings.gradle
├── local.properties
├── gradlew
├── gradlew.bat
├── gradle/
│   └── wrapper/
│       ├── gradle-wrapper.jar
│       └── gradle-wrapper.properties
├── .vscode/
│   ├── launch.json
│   └── tasks.json
├── app/
│   ├── build.gradle
│   └── src/
│       └── main/
│           ├── AndroidManifest.xml
│           ├── java/
│           │   └── com/
│           │       └── openlockr/
│           │           ├── MainActivity.java
│           │           ├── NativeBridge.java
│           │           └── FirebaseSync.java
│           ├── res/
│           │   ├── layout/
│           │   │   └── activity_main.xml
│           │   ├── values/
│           │   │   ├── strings.xml
│           │   │   └── colors.xml
│           │   └── drawable/
│           │       └── ic_launcher.xml
│           └── jniLibs/
└── native/
    ├── CMakeLists.txt
    └── src/
        ├── core.c
        ├── core.h
        ├── crypto/
        │   ├── aes.c
        │   └── aes.h
        ├── storage/
        │   ├── localdb.c
        │   └── localdb.h
        ├── sync/
        │   ├── firestore_sync.c
        │   └── firestore_sync.h
        └── utils/
            ├── base64.c
            └── base64.h
```