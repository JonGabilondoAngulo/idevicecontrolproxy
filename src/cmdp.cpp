#include <libwebsockets.h>
#include <string>
#include <stdio.h>
#include "errors.h"
#include "whitelist.h"
#include "resp.h"
#include "jsmn/jsmn.h"


static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

static int cmdp_parse(const char *in, size_t len, string & cmd, string & args);
static int cmdp_execute(const string & cmd_string, stringstream &output);



int cmdp_process(const char *in, size_t len, stringstream & response)
{
    int err = IDCP_SUCCESS;
    string cmd;
    string args;
    string cmdline;
    stringstream cmd_output;

    if (in == NULL) {
        err = IDCP_INTERNAL_ERROR;
        resp_build("failure", NULL, NULL, "Internal Error. Bad argument.", response);
        return err;
    }

    // Parse arrived JSON, get cmd and args
    err = cmdp_parse(in, len, cmd, args);
    if (err) {
        resp_build("failure", NULL, NULL, "Command JSON has incorrect format.", response);
        return err;
    }

    // Validate command. Whitelisted.
    err = whitelist_validate_cmd(cmd.c_str());
    if (err) {
        resp_build("failure", cmd.c_str(), NULL, "Command not in whitelist.", response);
        return err;
    }
    
    // Build command line
    cmdline = cmd;
    if (args.length()) {
        cmdline += " " + args;
    }
    
    // Excute
    err = cmdp_execute(cmdline, cmd_output);
    if (err) {
        resp_build("failure", cmd.c_str(), NULL, "Error executing command.", response);
        return err;
    }
    
    // Build reponse
    resp_build("success", cmdline.c_str(), &cmd_output, NULL, response);
    
	return err;
}

int cmdp_parse(const char *in, size_t len, string & cmd, string & args)
{
    int err = IDCP_SUCCESS;

    jsmn_parser p;
    jsmntok_t t[10]; /* We expect no more than 10 tokens */
    int i;
    int r;
    
    jsmn_init(&p);
    r = jsmn_parse(&p, in, len, t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        return IDCP_ERROR_PARSING_COMMAND;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        return IDCP_ERROR_PARSING_COMMAND;
    }
    
    if (r < 2) {
        return IDCP_COMMAND_INCORRECT_FORMAT;
    }
    
    int value_len;
    
    for (i = 1; i < r; i++) {
        value_len = t[i+1].end-t[i+1].start;
        if (jsoneq(in, &t[i], "cmd") == 0) {
            cmd =  string(in + t[i+1].start, value_len);
        } else if (jsoneq(in, &t[i], "args") == 0) {
            args = string(in + t[i+1].start, value_len);
        }
        i++;
    }
    
    // It is possible that will have no args
    if (cmd.length() == 0) {
        err = IDCP_COMMAND_INCORRECT_FORMAT;
    }
    
    return err;
}


int cmdp_execute(const string & cmd_string, stringstream &output) //char ** output, unsigned long * cmd_output_len)
{
    int err = IDCP_SUCCESS;
    
    FILE *fp = popen(cmd_string.c_str(), "r");
    if (fp) {

        char readbuf[1024];
        
        // read the output
        do {
            if (fgets(readbuf, 1024, fp) == NULL) {
                break;
            } else {
                output << readbuf;
          }
        } while(!feof(fp));
        
        pclose(fp);
    } else {
        err = IDCP_UNKNOWN_ERROR_EXECUTING_SHELL_COMMAND;
    }

    return err;
}




