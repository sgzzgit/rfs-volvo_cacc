/**
 * @file libconffile.h
 * @brief this library parses name=value style files and fills in applications' 
 *   structures. Link with -lconffile to use this library.
 *   See sample program main.c for details.
 */
#ifndef __LIBCONFIFILE_H__
#define __LIBCONFIFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

/**
 * ConfFileParamType_t - configuration parameter type
 *
 * The configuration parameter types can be of
 * - int
 * - unsigned int
 * - float
 * - string
 * - ip address
 * - psid
 * - bytestream (without spaces in between the bytes)
 * - double
 *
 *
 * In case of ip address, psid, bytestream the library validates the
 *   corresponding data field entered in the configuration file.
 */
typedef enum _ConfFileParamType_t{
    /**
     * use this for integer config param
     */
    CF_INT,
    /**
     * use this for unsigned integer config param
     */
    CF_UINT,
    /**
     * use this for float or double value types
     */
    CF_DOUBLE,
    /**
     * use this for string data config param
     */
    CF_STR,
    /**
     * use this for both IPv4 and IPv6 addresses
     */
    CF_IPADDR,
    /**
     * use this for PSID param
     */
    CF_PSID,
    /**
     * use this for HEX string param, the corresponding output field should be unsigned char *.
     * HEX string of form 010a0b02.. is parsed and converted to equivalent binary 
     * 0x010a0b02..
     */
    CF_HEX_STR,
} ConfFileParamType_t;
/**
 * ConfFileParseData_t is metadata structure that contains the information of
 * a configuration parameter.
 *
 * to read and parse a set of configuration parameters (variables), this structure
 * should be an array of structures.
 *
 */
typedef struct _ConfFileParseData_t {
    /**
     * Configuration variable name in the configuration file
     */
    char *param_name;

    /**
     * Configuration param type. It describe about the type of
     * the configuration parameter and the type of the output
     * location of it. (the caller's structure where the parameter
     * resides)
     */
    ConfFileParamType_t param_type;

    /**
     * App specific custom parser function for this variable. If this is
     * specified, the library will not do any parsing for this parameter
     */
    int (*custom_parser)(char *filepath, int line,
                    struct _ConfFileParseData_t *data, void *base_ptr,
                    const char *value, char *minval, char *maxval);

    /**
     * App specific custom writer function.
     */
    char * (*custom_writer)(struct _ConfFileParseData_t *data);

    /*
     * Variable specific parameters for the parser.
     */
    /**
     * offset from base_ptr where the parameter value
     * should be stored
     *
     * base_ptr is passed in the conffile_parse(buffer) function(s)
     */
    int     value_offset;
    /**
     * offset from base_ptr where the parameter's min value
     *  should be stored. -1 if not needed
     */
    int     min_offset;
    /**
     * offset from base_ptr  where the parameter's max value
     * should be stored. -1 if not needed
     */
    int     max_offset;
    /**
     * default value, if the variable is not present in the configuration file
     */
    double  default_value;
    /**
     * default min value, if the variable is not present in the configuration file
     */
    double  default_min;
    /** default max value if the variable is not present in the configuration file
     *
     */
    double  default_max;
    /**
     * increments to be used to validate the value between
     *  min and max. Set to 0 if the value just needs to be
     *  somewhere between min and max instead of specific
     *  values between min and max. Only used for CF_INT and
     *  CF_DOUBLE types
     */
    double  increment;
    /**
     * default value for str type params if the variable is not present in the configuration file
     */
    char    *default_str_value;
}ConfFileParseData_t;

/**
 * conffile_type - configuration paramter comments type.
 * This is used to ignore the comments in configuration file.
 * \n There are two types of comments that are supported in the configuration files.
 * \n\n They are '#' and ';'. \n\n
 * Both can be ORe'd to ignore comments of both types.
 */
typedef enum {
    /**
     * use for the comment of type #
     */
    CONFFILE_PARSE_TYPE_HASH        = 0x1,
    /**
     * use for the comment of type ;
     */
    CONFFILE_PARSE_TYPE_SEMICOLON   = 0x2,
} conffile_type;

/**
 * @brief parses the configuration file and fills configuration data in the memory pointed by base_ptr
 *
 * The conffile_parse parses the user specified configuration file containing the data of
 * the form \n
 * <B> ConfigParam = data; </B>
 * \n
 * and fills the memory pointed to by base_ptr as per
 * offsets mentioned in parse_data, the lib parses the comments
 * based on the comment type.
 * To parse a set of configuration parameters, a structure must be passed, of which the
 * size of the parse_data will be the total number of variables in the structure plus
 * a null terminated structure member at the end of the set. (An array of parse_data
 * structures must be passed to read a set of configuration variables).
 * \param filepath - configuration filename with complete path
 * \param parse_data - meta data, a pointer of ConfFileParseData_t
 * \param base_ptr - user specified configuration data structure. 
 * \param cmt_type - comment type . one of _HASH, or _SEMICOLON
 *
 * \return 0 on success and a number other than 0 on failure.
 */
int conffile_parse ( char *filepath, ConfFileParseData_t *parse_data,
                        void *base_ptr, conffile_type cmt_type);

/**
 * @brief parse the config data from a buffer instead of file
 *
 * The conffile_parse_buffer parse the buffer, instead of a file
 * it takes the buffer which contains the configuration data
 * of the form \n 
 * 	<B> ConfigParameter = data; </B>
 * \n and then it parses the things and
 * gets the value and assigns to the variable of the given offset
 * that wants this ConfigInfo.
 *
 * The rules/procedure to parse using this function are just the
 * same as that of conffile_parse.
 *
 *
 * \param buffer - config data buffer to parse from
 * \param parse_data - metadata, a pointer of ConfFileParseData_t type
 * \param base_ptr - a struct to copy the parsed values into
 * \param type - comment type . one of HASH, or SEMICOLON
 *
 *
 *  \return - Returns 0 on sucess and other value than 0 on failure.
              make user that you called the conffile_free function
              before you do other things after failure, otherwise
              program may leak memory.
 */
int conffile_parse_buffer(char *buffer, ConfFileParseData_t *parse_data,
                          void *base_ptr, conffile_type type);

/**
 * @brief Free the base_ptr
 *
 * Free the configuration file parameters that are allocated by the
 * conffile_parse. This function should be called even in case of 
   Failures to free up the allocated memory and in case if the 
   data copied into the base_ptr is of no use to the caller.

 * \param parse_data - a pointer of ConfFileParseData_t, this is 
 *                     a metadata pointer that describe about a
 *                     configuration variable.
 *
 * \param base - user specified configuration data structure\n\n
 * \return - none
 */
void conffile_free(ConfFileParseData_t *parse_data, void *base);

/**
 * @brief 
 *              The conffile_free_bin frees up the meomry allocate by the conffile_parse_buffer
 *              function.
 */
#define conffile_free_buffer conffile_free

#ifdef __cplusplus
}
#endif

#endif // __LIBCONFIFILE_H__
