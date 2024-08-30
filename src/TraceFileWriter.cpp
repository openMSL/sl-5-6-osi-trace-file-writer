//
// Copyright 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
// Copyright 2022 Persival GmbH
// Copyright 2023 BMW AG
// SPDX-License-Identifier: MPL-2.0
//

#include "TraceFileWriter.h"

#include <ctime>
#include <fstream>
#include <utility>

#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"

void TraceFileWriter::Init(std::string trace_path, std::string protobuf_version, std::string custom_name, std::string type)
{
    trace_path_ = std::move(trace_path);
    if (!trace_path_.empty() && trace_path_.back() != '/')
    {
        trace_path_ += '/';
    }
    protobuf_version_ = std::move(protobuf_version);
    custom_name_ = std::move(custom_name);
    type_ = std::move(type);
    SetFileName();
}

osi3::SensorData TraceFileWriter::Step(osi3::SensorData sensor_data)
{
    num_frames_++;
    osi_version_ =
        std::to_string(sensor_data.version().version_major()) + std::to_string(sensor_data.version().version_minor()) + std::to_string(sensor_data.version().version_patch());
    typedef unsigned int MessageSizeT;
    std::ofstream bin_file(trace_path_ + trace_file_name_, std::ios::binary | std::ios_base::app);

    std::string osi_msg_string_only = sensor_data.SerializeAsString();
    MessageSizeT message_size = osi_msg_string_only.size();
    char character[4];
    memcpy(character, (char*)&message_size, sizeof(MessageSizeT));

    std::string osi_msg_string;
    osi_msg_string += character[0];
    osi_msg_string += character[1];
    osi_msg_string += character[2];
    osi_msg_string += character[3];
    osi_msg_string += osi_msg_string_only;

    bin_file.write(osi_msg_string.c_str(), static_cast<long>(osi_msg_string.length()));
    bin_file.close();
    osi_msg_string.clear();

    return sensor_data;
}

void TraceFileWriter::SetFileName()
{
    time_t curr_time{};
    char buf[80];
    time(&curr_time);
    const tm* date_time = localtime(&curr_time);
    strftime(buf, 20, "%Y%m%dT%H%M%SZ", date_time);

    start_time_ = std::string(buf);

    trace_file_name_ = start_time_ + "_" + type_;
    if (!custom_name_.empty())
    {
        trace_file_name_ += "_" + custom_name_;
    }
    trace_file_name_ += ".osi";
}

void TraceFileWriter::Term()
{
    const std::string filename_tmp = trace_path_ + trace_file_name_;
    std::string filename_final = trace_path_ + start_time_ + "_" + type_ + "_" + osi_version_ + "_" + protobuf_version_ + "_" + std::to_string(num_frames_);
    if (!custom_name_.empty())
    {
        filename_final += "_" + custom_name_;
    }
    filename_final += ".osi";
    std::rename(filename_tmp.c_str(), filename_final.c_str());
}
