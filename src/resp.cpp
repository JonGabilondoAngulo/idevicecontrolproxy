#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "resp.h"
#include "errors.h"


static stringstream::pos_type stream_length(stringstream & stream)
{
    stream.seekg(0, ios::end);
    return stream.tellg();
}
static void stream_add_separator(stringstream & stream)
{
    if (stream_length(stream)) {
        stream << ",";
    }
}


int resp_build(const char * status, const char * cmd,  const stringstream * cmd_output, const char * err, stringstream & response)
{
    response << "{";
    
    if (status) {
        response << "\"status\":" << "\"" << status << "\"";
    }
    if (cmd) {
        stream_add_separator(response);
        response << "\"cmd\":" << "\"" << cmd << "\"";
    }

    if (cmd_output) {
        stream_add_separator(response);
        response << "\"response\":" << "\"" << cmd_output->str() << "\"";
    }
    
    if (err) {
        stream_add_separator(response);
        response << "\"error\":" << "\"" << err << "\"";
    }

    response << "}";
    
    return IDCP_SUCCESS;
}






