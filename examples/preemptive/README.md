## Preemptive Scheduler Demo

In this demo, we have three user tasks executing addition operation of different sizes (1M, 2M, 3M). Once a task completes its computation, it directly yields to the dispatcher with a task notification. The dispatcher then takes back the hand, saves the task context, and wake up the next task.

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
