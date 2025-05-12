//
// Copyright 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
// Copyright 2022 Persival GmbH
// Copyright 2023 BMW AG
// SPDX-License-Identifier: MPL-2.0
//

#include "TraceFileWriter.h"

#include "filesystem"
#include <ctime>
#include <fstream>
#include <utility>

#include "osi-utilities/tracefile/writer/MCAPTraceFileWriter.h"
#include "osi-utilities/tracefile/writer/SingleChannelBinaryTraceFileWriter.h"
#include "osi-utilities/tracefile/writer/TXTHTraceFileWriter.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"

void TraceFileWriter::Init(const std::string& trace_path, std::string protobuf_version, std::string custom_name, std::string message_type, FileFormat file_format, bool omit_timestamp)
{
    omit_timestamp_ = omit_timestamp;
    path_trace_folder_ = std::filesystem::path(trace_path);
    protobuf_version_ = std::move(protobuf_version);
    custom_name_ = std::move(custom_name);  // might be empty
    type_ = std::move(message_type);
    file_format_ = file_format;
    SetFileName();
    SetupWriter();
    SetupDeserializedWriterFunction();
}

bool TraceFileWriter::Step(const void* data, int size)
{
    num_frames_++;
    return serialized_writer_function_(data, size);
}

void TraceFileWriter::SetFileName()
{
    time_t curr_time{};
    char buf[80];
    time(&curr_time);
    const tm* date_time = localtime(&curr_time);
    strftime(buf, 20, "%Y%m%dT%H%M%SZ", date_time);
    if (omit_timestamp_) {
        start_time_ = "00000000T000000Z";
    } else {
        start_time_ = std::string(buf);
    }

    auto trace_file_name = start_time_ + "_" + type_;
    if (!custom_name_.empty())
    {
        trace_file_name += "_" + custom_name_;
    }

    trace_file_name += kFileNameMessageTypeMap.at(file_format_);

    path_trace_temp_ = path_trace_folder_ / trace_file_name;
}

void TraceFileWriter::SetupDeserializedWriterFunction()
{
    if (type_ == "sv")
    {
        setupForMessageType<osi3::SensorView>();
    }
    else if (type_ == "sd")
    {
        setupForMessageType<osi3::SensorData>();
    }
    else if (type_ == "gt")
    {
        setupForMessageType<osi3::GroundTruth>();
    }
    else
    {
        throw std::runtime_error("Unknown message type: " + type_);
    }
}

template <typename T>
void TraceFileWriter::setupForMessageType()
{
    if (file_format_ == FileFormat::MCAP)
    {
        auto mcap_writer = dynamic_cast<osi3::MCAPTraceFileWriter*>(writer_.get());

        writer_function_consecutive_ = [mcap_writer](const void* data, int size) {
            T msg;
            msg.ParseFromArray(data, size);
            return mcap_writer->WriteMessage(msg, "sl-5-6-osi-trace-file-writer");
        };
    }
    else if (file_format_ == FileFormat::OSI)
    {
        auto binary_writer = dynamic_cast<osi3::SingleChannelBinaryTraceFileWriter*>(writer_.get());
        writer_function_consecutive_ = [binary_writer](const void* data, int size) {
            T msg;
            msg.ParseFromArray(data, size);
            return binary_writer->WriteMessage(msg);
        };
    }
    else if (file_format_ == FileFormat::TXTH)
    {
        auto txth_writer = dynamic_cast<osi3::TXTHTraceFileWriter*>(writer_.get());
        writer_function_consecutive_ = [txth_writer](const void* data, int size) {
            T msg;
            msg.ParseFromArray(data, size);
            return txth_writer->WriteMessage(msg);
        };
    }

    // for the first time we receive a message, we need to extract the OSI version to add
    // it to the mcap channel metadata (and thus create the channel on the first message)
    // and add the OSI version the trace file name in the termination step
    serialized_writer_function_ = [this](const void* data, int size) {
        T msg;
        msg.ParseFromArray(data, size);
        auto version = msg.version();
        this->osi_version_ = std::to_string(version.version_major()) + std::to_string(version.version_minor()) + std::to_string(version.version_patch());
        // create mcap channel if mcap reader
        if (this->file_format_ == FileFormat::MCAP)
        {
            auto mcap_writer = dynamic_cast<osi3::MCAPTraceFileWriter*>(writer_.get());
            std::unordered_map<std::string, std::string> channel_metadata = {
                {"net.asam.osi.trace.channel.description", "Channel added via openMSL sl-5-6-osi-trace-file-writer"},
                {"net.asam.osi.trace.channel.osi_version",  // in the current implementation of asam-osi-utilities this will be overwritten. This must be changed in the
                                                            // asam-osi-utilities library
                 std::to_string(version.version_major()) + "." + std::to_string(version.version_minor()) + "." + std::to_string(version.version_patch())}};
            mcap_writer->AddChannel("sl-5-6-osi-trace-file-writer", T::descriptor(), channel_metadata);
        }
        // while this is the first call, we still need to write the data to the file
        auto write_success = this->writer_function_consecutive_(data, size);
        // replace the writer function for the next calls
        this->serialized_writer_function_ = this->writer_function_consecutive_;
        return write_success;
    };
}

void TraceFileWriter::SetupWriter()
{
    if (file_format_ == FileFormat::MCAP)
    {
        auto writer = std::make_unique<osi3::MCAPTraceFileWriter>();
        writer->Open(path_trace_temp_);
        writer->AddFileMetadata(osi3::MCAPTraceFileWriter::PrepareRequiredFileMetadata());
        writer_ = std::move(writer);
    }
    else if (file_format_ == FileFormat::OSI)
    {
        auto writer = std::make_unique<osi3::SingleChannelBinaryTraceFileWriter>();
        writer->Open(path_trace_temp_);
        writer_ = std::move(writer);
    }
    else if (file_format_ == FileFormat::TXTH)
    {
        auto writer = std::make_unique<osi3::TXTHTraceFileWriter>();
        writer->Open(path_trace_temp_);
        writer_ = std::move(writer);
    }
    else
    {
        throw std::runtime_error("Unknown file format");
    }
}

void TraceFileWriter::Term() const
{
    writer_->Close();

    // rename file based on number of frames
    std::filesystem::path path_trace_final_ = path_trace_folder_ / (start_time_ + "_" + type_ + "_" + osi_version_ + "_" + protobuf_version_ + "_" + std::to_string(num_frames_));
    if (!custom_name_.empty())
    {
        path_trace_final_ += "_" + custom_name_;
    }
    path_trace_final_ += kFileNameMessageTypeMap.at(file_format_);

    std::filesystem::rename(path_trace_temp_, path_trace_final_);
}
