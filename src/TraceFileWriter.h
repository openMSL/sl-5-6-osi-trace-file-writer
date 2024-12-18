//
// Copyright 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
// Copyright 2022 Persival GmbH
// Copyright 2023 BMW AG
// SPDX-License-Identifier: MPL-2.0
//

#pragma once

#include <filesystem>
#include <string>

#include "osi-utilities/tracefile/Writer.h"
#include "osi_sensordata.pb.h"

enum class FileFormat : u_int8_t
{
    kUnknown = 0,       /**< unknown file format (error */
    MCAP,               /**< .mcap trace file format */
    OSI,                /**< .osi trace file format*/
    TXTH,               /**< .txth trace file format */
};

class TraceFileWriter
{
 public:
   void Init(const std::string& trace_path, std::string protobuf_version, std::string custom_name, std::string message_type, FileFormat file_format);
    bool Step(const void * data, int size);
    void Term() const;

 private:
    FileFormat file_format_ = FileFormat::kUnknown;
    std::unique_ptr<osi3::TraceFileWriter> writer_;
    std::function<bool(const void*, int)> serialized_writer_function_;
    std::function<bool(const void*, int)> writer_function_consecutive_;

    std::filesystem::path path_trace_folder_;
    std::filesystem::path path_trace_temp_;
    std::string start_time_;
    int num_frames_ = 0;
    std::string osi_version_;
    std::string protobuf_version_;
    std::string custom_name_;
    std::string type_;
    void SetFileName();
    void SetupDeserializedWriterFunction();
    template <class T>
    void setupForMessageType();
    void SetupWriter();

    const std::unordered_map<FileFormat, std::string> kFileNameMessageTypeMap = {{FileFormat::kUnknown, ".unknown"},
                                                                                {FileFormat::MCAP, ".mcap"},
                                                                                {FileFormat::OSI, ".osi"},
                                                                                {FileFormat::TXTH, ".txth"}};

    /* Private File-based Logging just for Debugging */
#ifdef PRIVATE_LOG_PATH
    static ofstream private_log_file;
#endif

    static void FmiVerboseLogGlobal(const char *format, ...)
    {
#ifdef VERBOSE_FMI_LOGGING
#ifdef PRIVATE_LOG_PATH
        va_list ap;
        va_start(ap, format);
        char buffer[1024];
        if (!private_log_file.is_open())
            private_log_file.open(PRIVATE_LOG_PATH, ios::out | ios::app);
        if (private_log_file.is_open())
        {
#ifdef _WIN32
            vsnprintf_s(buffer, 1024, format, ap);
#else
            vsnprintf(buffer, 1024, format, ap);
#endif
            private_log_file << "OSMPDummySensor"
                             << "::Global:FMI: " << buffer << endl;
            private_log_file.flush();
        }
#endif
#endif
    }

    void InternalLog(const char *category, const char *format, va_list arg)
    {
#if defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING)
        char buffer[1024];
#ifdef _WIN32
        vsnprintf_s(buffer, 1024, format, arg);
#else
        vsnprintf(buffer, 1024, format, arg);
#endif
#ifdef PRIVATE_LOG_PATH
        if (!private_log_file.is_open())
            private_log_file.open(PRIVATE_LOG_PATH, ios::out | ios::app);
        if (private_log_file.is_open())
        {
            private_log_file << "OSMPDummySensor"
                             << "::" << instanceName << "<" << ((void*)this) << ">:" << category << ": " << buffer << endl;
            private_log_file.flush();
        }
#endif
#ifdef PUBLIC_LOGGING
        if (loggingOn && loggingCategories.count(category))
            functions.logger(functions.componentEnvironment, instanceName.c_str(), fmi2OK, category, buffer);
#endif
#endif
    }

    void FmiVerboseLog(const char *format, ...)
    {
#if defined(VERBOSE_FMI_LOGGING) && (defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING))
        va_list ap;
        va_start(ap, format);
        internal_log("FMI", format, ap);
        va_end(ap);
#endif
    }

    /* Normal Logging */
    void NormalLog(const char *category, const char *format, ...)
    {
#if defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING)
        va_list ap;
        va_start(ap, format);
        internal_log(category, format, ap);
        va_end(ap);
#endif
    }
};