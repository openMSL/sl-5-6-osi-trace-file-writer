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

void TraceFileWriter::Init(std::string trace_path)
{
    trace_path_ = std::move(trace_path);
    if (!trace_path_.empty() && trace_path_.back() != '/')
    {
        trace_path_ += '/';
    }
    SetFileName();
}

osi3::SensorData TraceFileWriter::Step(osi3::SensorData sensor_data)
{
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

    bin_file.write(osi_msg_string.c_str(), osi_msg_string.length());
    bin_file.close();
    osi_msg_string.clear();

    return sensor_data;
}
void TraceFileWriter::SetFileName()
{
    time_t curr_time;
    struct tm *detl;
    char buf[80];
    time( &curr_time );
    detl = localtime( &curr_time );
    strftime(buf, 20, "%Y%m%dT%H%M%SZ", detl);

    std::string start_time = std::string(buf);

    trace_file_name_ = start_time + "_sd_350_300_0000.osi";
}

