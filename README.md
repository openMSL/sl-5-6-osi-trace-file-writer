# OSI SensorDara Binary Trace File Writer

[![Test Pipeline](https://github.com/Persival-GmbH/OSI-SensorData-Trace-File-Writer/actions/workflows/test_pipeline.yml/badge.svg)](https://github.com/Persival-GmbH/OSI-SensorData-Trace-File-Writer/actions/workflows/test_pipeline.yml)

This [FMU](https://fmi-standard.org/) is able to write binary OSI SensorData trace files.
The folder the trace files shall be written to has to be passed as FMI parameter _trace_path_.
The trace file player is build according to the [ASAM Open simulation Interface (OSI)](https://github.com/OpenSimulationInterface/open-simulation-interface) and the [OSI Sensor Model Packaging (OSMP)](https://github.com/OpenSimulationInterface/osi-sensor-model-packaging) examples.

## Installation

### Dependencies

Install `cmake` (at least version 3.10.2):

```bash
$ sudo apt-get install cmake
```

Install `protobuf` (at least version 3.0.0):

```bash
$ sudo apt-get install libprotobuf-dev protobuf-compiler
```

### Clone with submodules

```bash
$ git clone https://gitlab.com/persival-open-source/open-simulation-interface/osi-sensordata-trace-file-writer.git
$ cd osi-sensordata-trace-file-writer
$ git submodule update --init
```

### Build

```bash
$ mkdir -p build
$ cd build
$ cmake ..
$ cmake --build .
```
