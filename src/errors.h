

enum idcp_err {
    IDCP_SUCCESS = 0,
    IDCP_FAILURE = -1,
    IDCP_INTERNAL_ERROR = -1000,
    IDCP_ERROR_PARSING_WHITELIST = -1001,
    IDCP_WHITELIST_MISSING = -1002,
    IDCP_WHITELIST_PARSER_NOT_INITIALISED = -1002,
    IDCP_UNKNOWN_ERROR_READING_WHITELIST = -1003,
    IDCP_UNKNOWN_ERROR_EXECUTING_SHELL_COMMAND = -1004,
    IDCP_ERROR_PARSING_COMMAND = -1005,
    IDCP_COMMAND_INCORRECT_FORMAT = -1006
};


