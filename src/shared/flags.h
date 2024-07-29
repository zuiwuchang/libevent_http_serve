// https://github.com/zuiwuchang/c-flags

// MIT License

// Copyright (c) 2024 zuiwuchang

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PPP_C_FLAGS__FLAGS_H
#define PPP_C_FLAGS__FLAGS_H
#include <stdint.h>
#include <stdlib.h>

#define PPP_C_FLAGS_VERSION (1 * 1000 * 1000 + 0 * 1000 + 0)

#ifndef PPP_C_FLAGS_DISABLE_VERIFY
#define PPP_C_FLAGS_DISABLE_VERIFY 0
#endif

#ifndef PPP_C_FLAGS_DISABLE_PRINT
#define PPP_C_FLAGS_DISABLE_PRINT 0
#endif

#if PPP_C_FLAGS_DISABLE_VERIFY < 1
#define __PPP_C_FLAGS_HAS_VERIFY 1
#endif
#if PPP_C_FLAGS_DISABLE_PRINT < 1
#define __PPP_C_FLAGS_HAS_PRINT 1
#endif

#define PPP_C_FLAGS_ERROR_DUPLICATE_FLAG 1
#define PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT 2
#define PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND 3
#define PPP_C_FLAGS_ERROR_INVALID_FLAG 4
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT 5
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP 6
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP 7
#define PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE 8
#define PPP_C_FLAGS_ERROR_INVALID_COMMAND 9
#define PPP_C_FLAGS_ERROR_MALLOC_FLAG 10
#define PPP_C_FLAGS_ERROR_MALLOC_COMMAND 11
#define PPP_C_FLAGS_ERROR_MALLOC_ARGV 12
#define PPP_C_FLAGS_ERROR_INVALID_ARGUMENT 13
#define PPP_C_FLAGS_ERROR_UNKNOW_FLAG 14
#define PPP_C_FLAGS_ERROR_UNKNOW_SHORT_FLAG 15

/**
 * Get version string
 */
const char *ppp_c_flags_version();
/**
 * Get error string information
 */
const char *ppp_c_flags_error(int err);

#define PPP_C_FLAGS_DEFINE_ARRAY(type, name) \
    typedef struct                           \
    {                                        \
        type *p;                             \
        size_t cap;                          \
        size_t len;                          \
    } ppp_c_flags_##name##_array_t;

#define PPP_C_FLAGS_TYPE_BOOL 1
#define PPP_C_FLAGS_TYPE_INT8 2
#define PPP_C_FLAGS_TYPE_INT16 3
#define PPP_C_FLAGS_TYPE_INT32 4
#define PPP_C_FLAGS_TYPE_INT64 5
#define PPP_C_FLAGS_TYPE_UINT8 6
#define PPP_C_FLAGS_TYPE_UINT16 7
#define PPP_C_FLAGS_TYPE_UINT32 8
#define PPP_C_FLAGS_TYPE_UINT64 9
#define PPP_C_FLAGS_TYPE_FLOAT32 10
#define PPP_C_FLAGS_TYPE_FLOAT64 11
#define PPP_C_FLAGS_TYPE_STRING 12

#define PPP_C_FLAGS_TYPE_BOOL_ARRAY 31
#define PPP_C_FLAGS_TYPE_INT8_ARRAY 32
#define PPP_C_FLAGS_TYPE_INT16_ARRAY 33
#define PPP_C_FLAGS_TYPE_INT32_ARRAY 34
#define PPP_C_FLAGS_TYPE_INT64_ARRAY 35
#define PPP_C_FLAGS_TYPE_UINT8_ARRAY 36
#define PPP_C_FLAGS_TYPE_UINT16_ARRAY 37
#define PPP_C_FLAGS_TYPE_UINT32_ARRAY 38
#define PPP_C_FLAGS_TYPE_UINT64_ARRAY 39
#define PPP_C_FLAGS_TYPE_FLOAT32_ARRAY 40
#define PPP_C_FLAGS_TYPE_FLOAT64_ARRAY 41
#define PPP_C_FLAGS_TYPE_STRING_ARRAY 42

#define PPP_C_FLAGS_BOOL uint8_t
#define PPP_C_FLAGS_INT8 int8_t
#define PPP_C_FLAGS_INT16 int16_t
#define PPP_C_FLAGS_INT32 int32_t
#define PPP_C_FLAGS_INT64 int64_t
#define PPP_C_FLAGS_UINT8 uint8_t
#define PPP_C_FLAGS_UINT16 uint16_t
#define PPP_C_FLAGS_UINT32 uint32_t
#define PPP_C_FLAGS_UINT64 uint64_t
#define PPP_C_FLAGS_FLOAT32 float
#define PPP_C_FLAGS_FLOAT64 double
#define PPP_C_FLAGS_STRING const char *

#define PPP_C_FLAGS_BOOL_ARRAY ppp_c_flags_bool_array_t
#define PPP_C_FLAGS_INT8_ARRAY ppp_c_flags_int8_array_t
#define PPP_C_FLAGS_INT16_ARRAY ppp_c_flags_int16_array_t
#define PPP_C_FLAGS_INT32_ARRAY ppp_c_flags_int32_array_t
#define PPP_C_FLAGS_INT64_ARRAY ppp_c_flags_int64_array_t
#define PPP_C_FLAGS_UINT8_ARRAY ppp_c_flags_uint8_array_t
#define PPP_C_FLAGS_UINT16_ARRAY ppp_c_flags_uint16_array_t
#define PPP_C_FLAGS_UINT32_ARRAY ppp_c_flags_uint32_array_t
#define PPP_C_FLAGS_UINT64_ARRAY ppp_c_flags_uint64_array_t
#define PPP_C_FLAGS_FLOAT32_ARRAY ppp_c_flags_float32_array_t
#define PPP_C_FLAGS_FLOAT64_ARRAY ppp_c_flags_float64_array_t
#define PPP_C_FLAGS_STRING_ARRAY ppp_c_flags_string_array_t
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_BOOL, bool);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_INT8, int8);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_INT16, int16);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_INT32, int32);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_INT64, int64);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_UINT8, uint8);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_UINT16, uint16);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_UINT32, uint32);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_UINT64, uint64);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_FLOAT32, float32);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_FLOAT64, float64);
PPP_C_FLAGS_DEFINE_ARRAY(PPP_C_FLAGS_STRING, string);

typedef struct ppp_c_flags_flag
{
    // next flag
    struct ppp_c_flags_flag *_next;
    void *_value;
    uint8_t _set_value;
    uint8_t _type;

    ppp_c_flags_bool_array_t _default;

    const char *_name;
    size_t _name_len;
    char _short_name;
    const char *_describe;

    void *_handler;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    // Verify value, if return not 0 indicates that the value is illegal
    int (*verify)(struct ppp_c_flags_flag *flag, uint8_t value_type, void *old_value, void *new_value);
#endif

#ifdef __PPP_C_FLAGS_HAS_PRINT
    // Used to set how to print values ​​on the console
    int (*print)(struct ppp_c_flags_flag *flag, uint8_t value_type, void *value);
#endif
} ppp_c_flags_flag_t;

typedef struct ppp_c_flags_command
{
    struct ppp_c_flags_command *_parent;
    struct ppp_c_flags_command *_next;
    struct ppp_c_flags_command *_child;
    struct ppp_c_flags_flag *_flag;

    const char *_name;
    size_t _name_len;
    const char *_describe;

    int (*handler)(struct ppp_c_flags_command *command, int argc, char **argv, void *userdata);
    void *userdata;
} ppp_c_flags_command_t;

typedef void *(*ppp_c_flags_malloc_f)(size_t n);
typedef void (*ppp_c_flags_free_f)(void *p);
typedef int (*ppp_c_flags_command_handler_f)(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata);

/**
 * Returns the name of the filepath, usually using ppp_c_flags_base_name(argv[0]) to create the root command
 */
const char *ppp_c_flags_base_name(const char *filepath);

/**
 * ParseUint64 function ported from golang standard library
 * ok: return 0;
 * overflow: return 1;
 * syntax: return -1;
 */
int ppp_c_flags_parse_uint64(
    const char *s, size_t s_len,
    int base, int bit_size,
    uint64_t *output);
/**
 * ParseInt64 function ported from golang standard library
 * ok: return 0;
 * overflow: return 1;
 * syntax: return -1;
 */
int ppp_c_flags_parse_int64(
    const char *s, size_t s_len,
    int base, int bit_size,
    int64_t *output);
/**
 * ParseBool function ported from golang standard library
 * case "1", "t", "T", "true", "TRUE", "True": return 1;
 * case "0", "f", "F", "false", "FALSE", "False": return 0;
 * elsecase return -1;
 */
int ppp_c_flags_parse_bool(const char *s, const size_t s_len);

/**
 * Set how to allocate and release memory
 */
void ppp_c_flags_alloctor(ppp_c_flags_malloc_f m, ppp_c_flags_free_f f);

/**
 * Create a command
 */
ppp_c_flags_command_t *ppp_c_flags_command_create_with_len(
    const char *name, const size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Create a command
 */
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Destroy all resources
 */
void ppp_c_flags_command_destroy(ppp_c_flags_command_t *c);

/**
 * Add subcommand
 */
ppp_c_flags_command_t *ppp_c_flags_add_command_with_len(
    ppp_c_flags_command_t *parent,
    const char *name, size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);

/**
 * Add subcommand
 */
ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err);
/**
 * Add a flag
 */
ppp_c_flags_flag_t *ppp_c_flags_add_flag_with_len(
    ppp_c_flags_command_t *command,
    const char *name, const size_t name_len, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err);

/**
 * Add a flag
 */
ppp_c_flags_flag_t *ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err);

#define PPP_C_FLAGS_ADD_FLAG(command, name, short_name, describe, value, value_type, err, fail) \
    if (!ppp_c_flags_add_flag(command, name, short_name, describe, value, value_type, err))     \
    {                                                                                           \
        printf("Add flags fail: --%s %s\n", name, ppp_c_flags_error(*(err)));                     \
        goto fail;                                                                              \
    }
#define PPP_C_FLAGS_SET_ADD_FLAG(flag, command, name, short_name, describe, value, value_type, err, fail) \
    falg = ppp_c_flags_add_flag(command, name, short_name, describe, value, value_type, err);             \
    if (!falg)                                                                                            \
    {                                                                                                     \
        printf("Add flags fail: --%s %s\n", name, ppp_c_flags_error(*(err)));                               \
        goto fail;                                                                                        \
    }
#define PPP_C_FLAGS_VAR_ADD_FLAG(flag, command, name, short_name, describe, value, value_type, err, fail)         \
    ppp_c_flags_flag_t *falg = ppp_c_flags_add_flag(command, name, short_name, describe, value, value_type, err); \
    if (!falg)                                                                                                    \
    {                                                                                                             \
        printf("Add flags fail: --%s %s\n", name, ppp_c_flags_error(*(err)));                                       \
        goto fail;                                                                                                \
    }

/**
 * Instructions for using the output command
 */
void ppp_c_flags_print(ppp_c_flags_command_t *command);

/**
 * Parse parameters and execute command callback
 */
int ppp_c_flags_execute(
    ppp_c_flags_command_t *root,
    int argc, char **argv,
    uint8_t allow_unknow, int *handler_result);

#endif