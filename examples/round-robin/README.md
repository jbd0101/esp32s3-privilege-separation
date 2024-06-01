## Round-Robin Scheduler Demo

In this demo, we have three user tasks executing addition operation of different sizes (1M, 2M, 3M). 
Each task loops over the addition operation and wait to be stopped by the dispatcher task in the kernel world.
The dispatcher gives 2000ms to each task to perform its computations. Once the time is exceeded, the dispatcher then takes back the hand, saves the task context, and wake up the next task.

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
