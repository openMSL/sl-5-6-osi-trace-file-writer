//
// Copyright 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
// Copyright 2022 Persival GmbH
// Copyright 2023 BMW AG
// SPDX-License-Identifier: MPL-2.0
//

#include "TraceFileWriter.h"

#include <ctime>
#include <fstream>
#include <iostream>

#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"

void TraceFileWriter::Init(std::string trace_path, std::string protobuf_version, std::string custom_name)
{
    trace_path_ = std::move(trace_path);
    if (!trace_path_.empty() && trace_path_.back() != '/')
    {
        trace_path_ += '/';
    }
    protobuf_version_ = protobuf_version;
    custom_name_ = custom_name;
    SetFileName();
}

osi3::SensorData TraceFileWriter::Step(osi3::SensorData sensor_data)
{
    num_frames_++;
    osi_version_ = std::to_string(sensor_data.version().version_major()) + std::to_string(sensor_data.version().version_minor()) + std::to_string(sensor_data.version().version_patch());
    typedef unsigned int MessageSizeT;
    std::ofstream bin_file(trace_path_ + trace_file_name_, std::ios::binary | std::ios_base::app);

    std::string osi_msg_string_only = sensor_data.SerializeAsString();
    MessageSizeT message_size = osi_msg_string_only.size();
    char character[4];
    memcpy(character, (char *) &message_size, sizeof(MessageSizeT));

    std::string osi_msg_string;
    osi_msg_string += character[0];
    osi_msg_string += character[1];
    osi_msg_string += character[2];
    osi_msg_string += character[3];
    osi_msg_string += osi_msg_string_only;

    bin_file.write(osi_msg_string.c_str(), long(osi_msg_string.length()));
    bin_file.close();
    osi_msg_string.clear();

    return sensor_data;
}
void TraceFileWriter::SetFileName()
{
    time_t curr_time{};
    struct tm *detl = nullptr;
    char buf[80];
    time(&curr_time);
    detl = localtime(&curr_time);
    strftime(buf, 20, "%Y%m%dT%H%M%SZ", detl);

    start_time_ = std::string(buf);

    trace_file_name_ = start_time_ + "_sd_tmp.osi";
}
void TraceFileWriter::Term()
{
    std::string filename_tmp = trace_path_ + trace_file_name_;
    std::string filename_final = trace_path_ + start_time_ + "_sd_" + osi_version_ + "_" + protobuf_version_ + "_" + std::to_string(num_frames_);
    if (!custom_name_.empty())
    {
        filename_final += "_" + custom_name_;
    }
    filename_final += ".osi";
    std::rename(filename_tmp.c_str(), filename_final.c_str());
}

