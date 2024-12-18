# SL-5-6 OSI Trace File Writer

This OSI Trace File Writer is a [FMU](https://fmi-standard.org/) designed to generate trace files from multiple OSI message types including `SensorData`, `SensorView`, and `GroundTruth`. 
It offers different output format options to meet various needs.

The trace file writer is build according to
the [ASAM Open simulation Interface (OSI)](https://github.com/OpenSimulationInterface/open-simulation-interface) and
the [OSI Sensor Model Packaging (OSMP)](https://github.com/OpenSimulationInterface/osi-sensor-model-packaging) examples.

## FMI Parameters

| Parameter        | Description                                                                                                                                                                                                                                                               |
|------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| trace_path       | Path, where to put the generated trace file                                                                                                                                                                                                                               |
| protobuf_version | Protobuf version, with which the OSI messages are serialized as string, e.g. "2112" for v21.12 (see [Naming Convention](https://opensimulationinterface.github.io/osi-antora-generator/asamosi/latest/interface/architecture/trace_file_naming.html))                     |
| custom_name      | Custom name as a suffix for the trace file name (see [Naming Convention](https://opensimulationinterface.github.io/osi-antora-generator/asamosi/latest/interface/architecture/trace_file_naming.html))                                                                    |
| message_type     | OSI message type string according to the [Naming Convention](https://opensimulationinterface.github.io/osi-antora-generator/asamosi/latest/interface/architecture/trace_file_naming.html). <br>Currently supports: SensorData (sd), SensorView (sv), and GroundTruth (gt) |
| file_format      | Format of the output trace file. Allowed values: mcap, osi, or txth                                                                                                                                                                                                      |


## Installation

### Dependencies

Install `cmake` (at least version 3.10.2):

```bash
sudo apt-get install cmake
```

Install `protobuf` (at least version 3.0.0):

```bash
sudo apt-get install libprotobuf-dev protobuf-compiler
```

Install `libzstd` and `liblz4`:

```bash
sudo apt-get install libzstd-dev liblz4-dev
```

### Clone with submodules

```bash
git clone --recurse-submodules https://github.com/openMSL/sl-5-6-osi-trace-file-writer.git
cd sl-5-6-osi-trace-file-writer
```

### Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```
