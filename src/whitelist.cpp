#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "whitelist.h"
#include "jsmn/jsmn.h"

#define IDCP_WHITELIST_MAX_TOKENS 256

static jsmn_parser whitelist_p;
static jsmntok_t t[IDCP_WHITELIST_MAX_TOKENS];
static int r = 0;
static char *source = NULL;

int whitelist_load_whitelist(char ** source, long * len);
int whitelist_create_parser(char * source, long len);


static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}


int whitelist_init( )
{
    int err = IDCP_SUCCESS;
    
    /* Load the whitelist.json content */
    long len;
    err = whitelist_load_whitelist(&source, &len);
    if (err) {
        return err;
    }
    
    /* Create parser with whitelist */
    err = whitelist_create_parser(source, len);
    
	return err;
}

int whitelist_terminate( )
{
    if (source) {
        free(source);
        source = NULL;
    }
    return IDCP_SUCCESS;
}


int whitelist_load_whitelist(char ** content, long * len)
{
    int err = IDCP_SUCCESS;
    size_t newLen;
    long bufsize;
    
    FILE * fp = fopen("whitelist.json", "r");
    if (!fp) {
        return IDCP_WHITELIST_MISSING;
    }
    
    if (fseek(fp, 0L, SEEK_END) != 0) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Get the size of the file. */
    bufsize = ftell(fp);
    if (bufsize == -1) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Allocate our buffer to that size. */
    *content = (char*)malloc(sizeof(char) * (bufsize + 1));
    *len = bufsize;
    
    /* Go back to the start of the file. */
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
        goto CLEANUP;
    }
    
    /* Read the entire file into memory. */
    newLen = fread(*content, sizeof(char), bufsize, fp);
    if ( ferror( fp ) != 0 ) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
    } else {
        (*content)[newLen++] = '\0'; /* Just to be safe. */
    }
    
CLEANUP:
    
    if (fclose(fp)) {
        err = IDCP_UNKNOWN_ERROR_READING_WHITELIST;
    }
    return err;
}

int whitelist_create_parser(char * source, long len)
{
    int err = IDCP_SUCCESS;
    int i = 1;
    
    jsmn_init(&whitelist_p);
    r = jsmn_parse(&whitelist_p, source, len, t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        return IDCP_ERROR_PARSING_WHITELIST ;
    }
    
    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        return IDCP_ERROR_PARSING_WHITELIST;
    }
    
    /* Validate the format/syntax of whitelist.
     {"cmds" : ["cmd1",...]}
     */
    for (i = 1; i < r; i++) {
        if (jsoneq(source, &t[i], "cmds") == 0) {
            int j;
            if (t[i+1].type != JSMN_ARRAY) {
                err = IDCP_ERROR_PARSING_WHITELIST; // We expect cmds to be an array of strings
                break;
            }
            for (j = 0; j < t[i+1].size; j++) {
                jsmntok_t *g = &t[i+j+2];
                if (g->type != JSMN_STRING) {
                    err = IDCP_ERROR_PARSING_WHITELIST;
                    break;
                }
            }
            i += t[i+1].size + 1;
        } else {
            err = IDCP_ERROR_PARSING_WHITELIST;
            break;
        }
    }
    
    return err;
}

int whitelist_validate_cmd(const char * cmd)
{
    int i = 1;
    int j;
    int err = IDCP_FAILURE;
    
    if (r == 0 || i >= r) {
        return IDCP_WHITELIST_PARSER_NOT_INITIALISED;
    }
    
    /* Loop over all commands to find match */
    if (jsoneq(source, &t[i], "cmds") != 0) {
        return err;
    }
    if (t[i+1].type != JSMN_ARRAY) {
        return err;
    }
    for (j = 0; j < t[i+1].size; j++) {
        jsmntok_t *g = &t[i+j+2];
        if (jsoneq(source, g, cmd) == 0) {
            err = IDCP_SUCCESS;
            break;
        }
    }
    
    return err;
}




