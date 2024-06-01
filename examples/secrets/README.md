## Secrets Management Demo
In this demo, the kernel task will save two secrets `key1` and `key2` in the NVS flash memory for two tasks. The user tasks will then retrieve the secrets and print them on the console.
One can also see that both tasks use the same keys but the secrets are different for each task. That is because they use two different namespaces. It is thus impossible for a task to access the secrets of another task.

### Setting Up ESP-IDF

```
$ cd $IDF_PATH
$ git checkout v4.4.3
$ git submodule update --init --recursive
$ ./install.sh
$ source ./export.sh
```

#### Apply patch on IDF:

```
$ git apply -v <path/to/privilege-separation>/idf-patches/privilege-separation_support_v4.4.3.patch
```

### Hardware

- ESP32-C3 based development board
- ESP32-S3 based development board

### Build and Flash

Build is separated in two parts: Protected app and User app.

- Protected app is a standalone app and is only dependent on ESP-IDF
- User app relies on certain information and libraries derived from the protected build

```
$ idf.py set-target { esp32c3 | esp32s3 }
$ idf.py build
$ idf.py flash monitor
```
