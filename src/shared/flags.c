#include "flags.h"
#include "string.h"
#include <stdio.h>
#include <errno.h>

static char __ppp_c_flags_version[1 + 3 + 1 + 3 + 1 + 3 + 1] = {0};
const char *ppp_c_flags_version()
{
    char *s = __ppp_c_flags_version;
    if (!s[0])
    {
        uint32_t version = PPP_C_FLAGS_VERSION;
        sprintf(s + 1,
                "%u.%u.%u",
                (version / 1000 / 1000) % 1000,
                (version / 1000) % 1000,
                (version) % 1000);
        s[0] = 1;
    }
    return s + 1;
}
#define PPP_C_FLAGS_VERIFY_FLAG(flag, new_value, err)                   \
    if (flag->verify)                                                   \
    {                                                                   \
        err = flag->verify(flag, flag->_type, flag->_value, new_value); \
        if (err)                                                        \
        {                                                               \
            return err;                                                 \
        }                                                               \
    }
#define PPP_C_FLAGS_STATE_NONE 0
#define PPP_C_FLAGS_STATE_FLAGS 1
#define PPP_C_FLAGS_STATE_FLAGS_SHORT 2

#define PPP_C_FLAGS_LOGER(c) (c | ('x' - 'X'))

static ppp_c_flags_malloc_f ppp_c_flags_malloc = malloc;
static ppp_c_flags_free_f ppp_c_flags_free = free;

static int ppp_c_flags_gropw(ppp_c_flags_flag_t *flag, size_t argc, size_t size)
{
    ppp_c_flags_bool_array_t *p = flag->_value;
    ppp_c_flags_bool_array_t temp = *p;
    p = &temp;
    if (!flag->_set_value)
    {
        p->len = 0;
    }

    size_t n = 1;
    size_t available;
    size_t cap;
    if (p->cap)
    {
        available = p->cap - p->len;
        cap = p->cap;
    }
    else
    {
        available = 0;
        n += p->len;
        cap = 64;
    }
    if (available < n)
    {
        n -= available;
        // calculate capacity
        if (!p->cap && n < 64)
        {
            cap = 64;
        }
        else
        {
            size_t c = p->len + n;
            cap *= 2;
            if (c >= cap)
            {
                cap = c;
            }
        }
        argc++;
        argc += p->len;
        if (cap > argc)
        {
            cap = argc;
        }
        size_t min = p->len + 1;
        if (cap < min)
        {
            cap = min;
        }

        void *ptr = ppp_c_flags_malloc(cap * size);
        if (!ptr)
        {
            return -1;
        }
        memcpy(ptr, p->p, size * cap);
        if (p->cap)
        {
            ppp_c_flags_free(p->p);
        }
        p->p = ptr;
        p->cap = cap;
    }
    p = flag->_value;
    *p = temp;
    flag->_set_value = 1;
    return 0;
}
typedef struct
{
    const char *name;
    size_t name_len;

    void (*print_default)(ppp_c_flags_flag_t *flag);
    int (*set_flag)(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n);
} ppp_c_flags_flag_handler_t;
#define PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, type, format) \
    type val = *(type *)(&flag->_default);                    \
    if (val)                                                  \
    {                                                         \
        printf("<default: " format ">", val);                 \
    }
#define PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, type, format)   \
    type val = *(type *)(&flag->_default);                   \
    if (val)                                                 \
    {                                                        \
        if (flag->print)                                     \
        {                                                    \
            printf("<default: ");                            \
            flag->print(flag, flag->_type, &flag->_default); \
            printf(">");                                     \
        }                                                    \
        else                                                 \
        {                                                    \
            printf("<default: " format ">", val);            \
        }                                                    \
    }

static void ppp_c_flags_handler_bool_print_default(ppp_c_flags_flag_t *flag)
{
    if (*(PPP_C_FLAGS_BOOL *)(&flag->_default))
    {
#ifdef __PPP_C_FLAGS_HAS_PRINT
        if (flag->print)
        {
            printf("<default: ");
            flag->print(flag, flag->_type, &flag->_default);
            printf(">");
        }
        else
        {
            printf("<default: true>");
        }
#else
        printf("<default: true>");
#endif
    }
}
static int ppp_c_flags_handler_bool_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    PPP_C_FLAGS_BOOL new_value;
    switch (ppp_c_flags_parse_bool(s, s_len))
    {
    case 0:
        new_value = 0;
        break;
    case 1:
        new_value = 1;
        break;
    default:
        return -1;
    }
    int err;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_BOOL *)flag->_value = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_bool = {
    .name = "bool",
    .name_len = 4,
    .print_default = ppp_c_flags_handler_bool_print_default,
    .set_flag = ppp_c_flags_handler_bool_set_flag,
};
static void ppp_c_flags_handler_int8_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT8, "%d")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT8, "%d")
#endif
}
static int ppp_c_flags_handler_int8_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 8,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT8 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_INT8 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int8 = {
    .name = "int8",
    .name_len = 4,
    .print_default = ppp_c_flags_handler_int8_print_default,
    .set_flag = ppp_c_flags_handler_int8_set_flag,
};
static void ppp_c_flags_handler_int16_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT16, "%d")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT16, "%d")
#endif
}
static int ppp_c_flags_handler_int16_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 16,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT16 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_INT16 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int16 = {
    .name = "int16",
    .name_len = 5,
    .print_default = ppp_c_flags_handler_int16_print_default,
    .set_flag = ppp_c_flags_handler_int16_set_flag,
};
static void ppp_c_flags_handler_int32_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT32, "%d")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT32, "%d")
#endif
}
static int ppp_c_flags_handler_int32_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 32,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT32 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_INT32 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int32 = {
    .name = "int32",
    .name_len = 5,
    .print_default = ppp_c_flags_handler_int32_print_default,
    .set_flag = ppp_c_flags_handler_int32_set_flag,
};

static void ppp_c_flags_handler_int64_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT64, "%ld")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT64, "%ld")
#endif
}
static int ppp_c_flags_handler_int64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t new_value;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 64,
        &new_value);
    if (err)
    {
        return err;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(int64_t *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int64 = {
    .name = "int64",
    .name_len = 5,
    .print_default = ppp_c_flags_handler_int64_print_default,
    .set_flag = ppp_c_flags_handler_int64_set_flag,
};
static void ppp_c_flags_handler_uint8_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT8, "%u")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT8, "%u")
#endif
}
static int ppp_c_flags_handler_uint8_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 8,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT8 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_UINT8 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint8 = {
    .name = "uint8",
    .name_len = 5,
    .print_default = ppp_c_flags_handler_uint8_print_default,
    .set_flag = ppp_c_flags_handler_uint8_set_flag,
};
static void ppp_c_flags_handler_uint16_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT16, "%u")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT16, "%u")
#endif
}
static int ppp_c_flags_handler_uint16_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 16,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT16 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_UINT16 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint16 = {
    .name = "uint16",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_uint16_print_default,
    .set_flag = ppp_c_flags_handler_uint16_set_flag,
};
static void ppp_c_flags_handler_uint32_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT32, "%u")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT32, "%u")
#endif
}
static int ppp_c_flags_handler_uint32_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 32,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT32 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_UINT32 *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint32 = {
    .name = "uint32",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_uint32_print_default,
    .set_flag = ppp_c_flags_handler_uint32_set_flag,
};
static void ppp_c_flags_handler_uint64_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT64, "%lu")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT64, "%lu")
#endif
}
static int ppp_c_flags_handler_uint64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t new_value;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 64,
        &new_value);
    if (err)
    {
        return err;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(uint64_t *)(flag->_value) = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint64 = {
    .name = "uint64",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_uint64_print_default,
    .set_flag = ppp_c_flags_handler_uint64_set_flag,
};
static void ppp_c_flags_handler_float32_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_FLOAT32, "%g")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_FLOAT32, "%g")
#endif
}
static int ppp_c_flags_handler_float32_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err = errno;
    errno = 0;
    char *end = 0;
    double v = strtod(s, &end);
    if (errno)
    {
        return -1;
    }
    errno = err;
    if (end && end[0] != 0)
    {
        return -1;
    }
    float new_value = v;
    if (v != new_value)
    {
        return -1;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_FLOAT32 *)flag->_value = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_float32 = {
    .name = "float32",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_float32_print_default,
    .set_flag = ppp_c_flags_handler_float32_set_flag,
};
static void ppp_c_flags_handler_float64_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC(flag, PPP_C_FLAGS_FLOAT64, "%g")
#else
    PPP_C_FLAGS_PRINT_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_FLOAT64, "%g")
#endif
}
static int ppp_c_flags_handler_float64_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err = errno;
    errno = 0;
    char *end = 0;
    double new_value = strtod(s, &end);
    if (errno)
    {
        return -1;
    }
    errno = err;
    if (end && end[0] != 0)
    {
        return -1;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    *(PPP_C_FLAGS_FLOAT64 *)flag->_value = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_float64 = {
    .name = "float64",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_float64_print_default,
    .set_flag = ppp_c_flags_handler_float64_set_flag,
};
static void ppp_c_flags_handler_string_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_STRING s = *(PPP_C_FLAGS_STRING *)(&flag->_default);
    if (s && s[0] != 0)
    {
        printf("<default: %s>", s);
    }
}
static int ppp_c_flags_handler_string_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &s, err)
#endif
    *(PPP_C_FLAGS_STRING *)flag->_value = s;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_string = {
    .name = "string",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_string_print_default,
    .set_flag = ppp_c_flags_handler_string_set_flag,
};
#define PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, type, format) \
    type *vals = (void *)(&flag->_default);                         \
    switch (vals->len)                                              \
    {                                                               \
    case 0:                                                         \
        break;                                                      \
    case 1:                                                         \
        printf("<default: [" format "]", vals->p[0]);               \
        break;                                                      \
    default:                                                        \
        printf("<default: [");                                      \
        for (size_t i = 0; i < vals->len; i++)                      \
        {                                                           \
            if (i)                                                  \
            {                                                       \
                printf(", " format, vals->p[i]);                    \
            }                                                       \
            else                                                    \
            {                                                       \
                printf(format, vals->p[i]);                         \
            }                                                       \
        }                                                           \
        printf("]>");                                               \
        break;                                                      \
    }
#define PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, type, format) \
    type *vals = (void *)(&flag->_default);                      \
    if (flag->print && vals->len)                                \
    {                                                            \
        printf("<default: [");                                   \
        flag->print(flag, flag->_type, &flag->_default);         \
        printf("]");                                             \
    }                                                            \
    else                                                         \
    {                                                            \
        switch (vals->len)                                       \
        {                                                        \
        case 0:                                                  \
            break;                                               \
        case 1:                                                  \
            printf("<default: [" format "]", vals->p[0]);        \
            break;                                               \
        default:                                                 \
            printf("<default: [");                               \
            for (size_t i = 0; i < vals->len; i++)               \
            {                                                    \
                if (i)                                           \
                {                                                \
                    printf(", " format, vals->p[i]);             \
                }                                                \
                else                                             \
                {                                                \
                    printf(format, vals->p[i]);                  \
                }                                                \
            }                                                    \
            printf("]>");                                        \
            break;                                               \
        }                                                        \
    }

static void ppp_c_flags_handler_bool_array_print_default(ppp_c_flags_flag_t *flag)
{
    PPP_C_FLAGS_BOOL_ARRAY *vals = &flag->_default;
#ifdef __PPP_C_FLAGS_HAS_PRINT
    if (flag->print && vals->len)
    {
        printf("<default: [");
        flag->print(flag, flag->_type, &flag->_default);
        printf("]");
        return;
    }
#endif
    switch (vals->len)
    {
    case 0:
        break;
    case 1:
        if (vals->p[0])
        {
            printf("<default: [true]");
        }
        else
        {
            printf("<default: [false]");
        }
        break;
    default:
        printf("<default: [");
        for (size_t i = 0; i < vals->len; i++)
        {
            if (i)
            {
                printf(",%s", vals->p[i] ? "true" : "false");
            }
            else
            {
                printf("%s", vals->p[i] ? "true" : "false");
            }
        }
        printf("]>");
        break;
    }
}
static int ppp_c_flags_handler_bool_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    PPP_C_FLAGS_BOOL new_value;
    switch (ppp_c_flags_parse_bool(s, s_len))
    {
    case 0:
        new_value = 0;
        break;
    case 1:
        new_value = 1;
        break;
    default:
        return -1;
    }
    int err;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_BOOL_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_BOOL)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_bool_array = {
    .name = "[]bool",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_bool_array_print_default,
    .set_flag = ppp_c_flags_handler_bool_array_set_flag,
};

static void ppp_c_flags_handler_int8_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT8_ARRAY, "%d")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT8_ARRAY, "%d")
#endif
}
static int ppp_c_flags_handler_int8_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 8,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT8 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_INT8_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_INT8)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int8_array = {
    .name = "[]int8",
    .name_len = 6,
    .print_default = ppp_c_flags_handler_int8_array_print_default,
    .set_flag = ppp_c_flags_handler_int8_array_set_flag,
};
static void ppp_c_flags_handler_int16_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT16_ARRAY, "%d")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT16_ARRAY, "%d")
#endif
}
static int ppp_c_flags_handler_int16_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 16,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT16 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_INT16_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_INT16)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int16_array = {
    .name = "[]int16",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_int16_array_print_default,
    .set_flag = ppp_c_flags_handler_int16_array_set_flag,
};
static void ppp_c_flags_handler_int32_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT32_ARRAY, "%d")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT32_ARRAY, "%d")
#endif
}
static int ppp_c_flags_handler_int32_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t v0;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 32,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_INT32 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_INT32_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_INT32)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int32_array = {
    .name = "[]int32",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_int32_array_print_default,
    .set_flag = ppp_c_flags_handler_int32_array_set_flag,
};
static void ppp_c_flags_handler_int64_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_INT64_ARRAY, "%ld")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_INT64_ARRAY, "%ld")
#endif
}
static int ppp_c_flags_handler_int64_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int64_t new_value;
    int err = ppp_c_flags_parse_int64(
        s, s_len,
        0, 64,
        &new_value);
    if (err)
    {
        return err;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_INT64_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_INT64)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_int64_array = {
    .name = "[]int64",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_int64_array_print_default,
    .set_flag = ppp_c_flags_handler_int64_array_set_flag,
};

static void ppp_c_flags_handler_uint8_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT8_ARRAY, "%u")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT8_ARRAY, "%u")
#endif
}
static int ppp_c_flags_handler_uint8_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 8,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT8 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_UINT8_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_UINT8)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint8_array = {
    .name = "[]uint8",
    .name_len = 7,
    .print_default = ppp_c_flags_handler_uint8_array_print_default,
    .set_flag = ppp_c_flags_handler_uint8_array_set_flag,
};
static void ppp_c_flags_handler_uint16_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT16_ARRAY, "%u")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT16_ARRAY, "%u")
#endif
}
static int ppp_c_flags_handler_uint16_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 16,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT16 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_UINT16_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_UINT16)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint16_array = {
    .name = "[]uint16",
    .name_len = 8,
    .print_default = ppp_c_flags_handler_uint16_array_print_default,
    .set_flag = ppp_c_flags_handler_uint16_array_set_flag,
};
static void ppp_c_flags_handler_uint32_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT32_ARRAY, "%u")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT32_ARRAY, "%u")
#endif
}
static int ppp_c_flags_handler_uint32_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t v0;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 32,
        &v0);
    if (err)
    {
        return err;
    }
    PPP_C_FLAGS_UINT32 new_value = v0;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_UINT32_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_UINT32)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint32_array = {
    .name = "[]uint32",
    .name_len = 8,
    .print_default = ppp_c_flags_handler_uint32_array_print_default,
    .set_flag = ppp_c_flags_handler_uint32_array_set_flag,
};
static void ppp_c_flags_handler_uint64_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_UINT64_ARRAY, "%lu")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_UINT64_ARRAY, "%lu")
#endif
}
static int ppp_c_flags_handler_uint64_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    uint64_t new_value;
    int err = ppp_c_flags_parse_uint64(
        s, s_len,
        0, 64,
        &new_value);
    if (err)
    {
        return err;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_UINT64_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_UINT64)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_uint64_array = {
    .name = "[]uint64",
    .name_len = 8,
    .print_default = ppp_c_flags_handler_uint64_array_print_default,
    .set_flag = ppp_c_flags_handler_uint64_array_set_flag,
};

static void ppp_c_flags_handler_float32_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_FLOAT32_ARRAY, "%g")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_FLOAT32_ARRAY, "%g")
#endif
}
static int ppp_c_flags_handler_float32_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err = errno;
    errno = 0;
    char *end = 0;
    double v = strtod(s, &end);
    if (errno)
    {
        return -1;
    }
    errno = err;
    if (end && end[0] != 0)
    {
        return -1;
    }
    float new_value = v;
    if (v != new_value)
    {
        return -1;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif
    PPP_C_FLAGS_FLOAT32_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_FLOAT32)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_float32_array = {
    .name = "[]float32",
    .name_len = 9,
    .print_default = ppp_c_flags_handler_float32_array_print_default,
    .set_flag = ppp_c_flags_handler_float32_array_set_flag,
};
static void ppp_c_flags_handler_float64_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_FLOAT64_ARRAY, "%g")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_FLOAT64_ARRAY, "%g")
#endif
}
static int ppp_c_flags_handler_float64_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err = errno;
    errno = 0;
    char *end = 0;
    double new_value = strtod(s, &end);
    if (errno)
    {
        return -1;
    }
    errno = err;
    if (end && end[0] != 0)
    {
        return -1;
    }
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &new_value, err)
#endif

    PPP_C_FLAGS_FLOAT64_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_FLOAT64)))
    {
        return -1;
    }
    vals->p[vals->len++] = new_value;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_float64_array = {
    .name = "[]float64",
    .name_len = 9,
    .print_default = ppp_c_flags_handler_float64_array_print_default,
    .set_flag = ppp_c_flags_handler_float64_array_set_flag,
};
static void ppp_c_flags_handler_string_array_print_default(ppp_c_flags_flag_t *flag)
{
#ifdef __PPP_C_FLAGS_HAS_PRINT
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC(flag, PPP_C_FLAGS_STRING_ARRAY, "%s")
#else
    PPP_C_FLAGS_PRINT_ARRAY_DEFAULT_FUNC_NO(flag, PPP_C_FLAGS_STRING_ARRAY, "%s")
#endif
}
static int ppp_c_flags_handler_string_array_set_flag(ppp_c_flags_flag_t *flag, const char *s, size_t s_len, size_t n)
{
    int err;
#ifdef __PPP_C_FLAGS_HAS_VERIFY
    PPP_C_FLAGS_VERIFY_FLAG(flag, &s, err)
#endif

    PPP_C_FLAGS_STRING_ARRAY *vals = (void *)flag->_value;
    if (ppp_c_flags_gropw(flag, n, sizeof(PPP_C_FLAGS_STRING)))
    {
        return -1;
    }
    vals->p[vals->len++] = s;
    return 0;
}
static ppp_c_flags_flag_handler_t ppp_c_flags_handler_string_array = {
    .name = "[]string",
    .name_len = 8,
    .print_default = ppp_c_flags_handler_string_array_print_default,
    .set_flag = ppp_c_flags_handler_string_array_set_flag,
};

static void ppp_c_flags_print_usage(ppp_c_flags_command_t *command);
const char *ppp_c_flags_error(int err)
{
    switch (err)
    {
    case PPP_C_FLAGS_ERROR_DUPLICATE_FLAG:
        return "duplicate flag name";
    case PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT:
        return "duplicate flag short name";
    case PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND:
        return "duplicate subcommand name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG:
        return "invalid flag name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT:
        return "invalid flag short name";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP:
        return "unable to use reserved flag --help";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP:
        return "unable to use reserved short flag -h";
    case PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE:
        return "invalid flag type";
    case PPP_C_FLAGS_ERROR_INVALID_COMMAND:
        return "invalid command name";
    case PPP_C_FLAGS_ERROR_MALLOC_FLAG:
        return "malloc flag fial";
    case PPP_C_FLAGS_ERROR_MALLOC_COMMAND:
        return "malloc command fial";
    case PPP_C_FLAGS_ERROR_MALLOC_ARGV:
        return "malloc argv fail";
    case PPP_C_FLAGS_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case PPP_C_FLAGS_ERROR_UNKNOW_FLAG:
        return "unknow flag";
    case PPP_C_FLAGS_ERROR_UNKNOW_SHORT_FLAG:
        return "unknow short flag";
    }
    return "unknow error";
}
const char *ppp_c_flags_base_name(const char *filepath)
{
    size_t len = strlen(filepath);
    if (len > 2)
    {
        for (size_t i = len - 1; i > 0; i--)
        {
            switch (filepath[i])
            {
            case '/':
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
            case '\\':
#endif
                return filepath + i + 1;
            }
        }
    }
    return filepath;
}

// underscoreOK reports whether the underscores in s are allowed.
// Checking them in this one function lets all the parsers skip over them simply.
// Underscore must appear only between digits or between a base prefix and a digit.
uint8_t ppp_c_flags_underscore_ok(const char *s, size_t s_len)
{
    // saw tracks the last character (class) we saw:
    // ^ for beginning of number,
    // 0 for a digit or base prefix,
    // _ for an underscore,
    // ! for none of the above.
    char saw = '^';
    size_t i = 0;

    // Optional sign.
    if (s_len >= 1 && (s[0] == '-' || s[0] == '+'))
    {
        s++;
        s_len--;
    }

    // Optional base prefix.
    uint8_t hex = 0;
    if (s_len >= 2 && s[0] == '0')
    {
        switch (PPP_C_FLAGS_LOGER(s[1]))
        {
        case 'x':
            hex = 1;
        case 'b':
        case 'o':
            i = 2;
            saw = '0';
            break;
        }
    }

    // Number proper.
    for (; i < s_len; i++)
    {
        // Digits are always okay.
        if ('0' <= s[i] && s[i] <= '9' || hex && 'a' <= PPP_C_FLAGS_LOGER(s[i]) && PPP_C_FLAGS_LOGER(s[i]) <= 'f')
        {
            saw = '0';
            continue;
        }
        // Underscore must follow digit.
        if (s[i] == '_')
        {
            if (saw != '0')
            {
                return 0;
            }
            saw = '_';
            continue;
        }
        // Underscore must also be followed by digit.
        if (saw == '_')
        {
            return 0;
        }
        // Saw non-digit, non-underscore.
        saw = '!';
    }
    return saw != '_' ? 1 : 0;
}

int ppp_c_flags_parse_uint64(
    const char *s, size_t s_len,
    int base, int bit_size,
    uint64_t *output)
{
    if (!s || !s_len)
    {
        return -1;
    }
    uint8_t base0 = base == 0;
    const char *s0 = s;
    size_t s0_len = s_len;
    if (2 <= base && base <= 36)
    {
    }
    else if (!base)
    {
        base = 10;
        if (s[0] == '0')
        {
            if (s_len >= 3)
            {
                switch (PPP_C_FLAGS_LOGER(s[1]))
                {
                case 'b':
                    base = 2;
                    s += 2;
                    s_len -= 2;
                    break;
                case 'o':
                    base = 8;
                    s += 2;
                    s_len -= 2;
                    break;
                case 'x':
                    base = 16;
                    s += 2;
                    s_len -= 2;
                    break;
                default:
                    base = 8;
                    s++;
                    s_len--;
                    break;
                }
            }
            else
            {
                base = 8;
                s++;
                s_len--;
            }
        }
    }
    else
    {
        return -1;
    }
    if (!bit_size)
    {
        bit_size = 64;
    }
    else if (bit_size < 0 || bit_size > 64)
    {
        return -1;
    }
    // Cutoff is the smallest number such that cutoff*base > maxUint64.
    // Use compile-time constants for common cases.
    uint64_t cutoff = 18446744073709551615UL;
    switch (base)
    {
    case 10:
        cutoff /= 10;
        break;
    case 16:
        cutoff /= 16;
        break;
    default:
        cutoff /= base;
        break;
    }
    cutoff++;

    uint64_t maxVal = 1;
    switch (bit_size)
    {
    case 1:
        maxVal = 1UL;
        break;
    case 2:
        maxVal = 3UL;
        break;
    case 3:
        maxVal = 7UL;
        break;
    case 4:
        maxVal = 15UL;
        break;
    case 5:
        maxVal = 31UL;
        break;
    case 6:
        maxVal = 63UL;
        break;
    case 7:
        maxVal = 127UL;
        break;
    case 8:
        maxVal = 255UL;
        break;
    case 9:
        maxVal = 511UL;
        break;
    case 10:
        maxVal = 1023UL;
        break;
    case 11:
        maxVal = 2047UL;
        break;
    case 12:
        maxVal = 4095UL;
        break;
    case 13:
        maxVal = 8191UL;
        break;
    case 14:
        maxVal = 16383UL;
        break;
    case 15:
        maxVal = 32767UL;
        break;
    case 16:
        maxVal = 65535UL;
        break;
    case 17:
        maxVal = 131071UL;
        break;
    case 18:
        maxVal = 262143UL;
        break;
    case 19:
        maxVal = 524287UL;
        break;
    case 20:
        maxVal = 1048575UL;
        break;
    case 21:
        maxVal = 2097151UL;
        break;
    case 22:
        maxVal = 4194303UL;
        break;
    case 23:
        maxVal = 8388607UL;
        break;
    case 24:
        maxVal = 16777215UL;
        break;
    case 25:
        maxVal = 33554431UL;
        break;
    case 26:
        maxVal = 67108863UL;
        break;
    case 27:
        maxVal = 134217727UL;
        break;
    case 28:
        maxVal = 268435455UL;
        break;
    case 29:
        maxVal = 536870911UL;
        break;
    case 30:
        maxVal = 1073741823UL;
        break;
    case 31:
        maxVal = 2147483647UL;
        break;
    case 32:
        maxVal = 4294967295UL;
        break;
    case 33:
        maxVal = 8589934591UL;
        break;
    case 34:
        maxVal = 17179869183UL;
        break;
    case 35:
        maxVal = 34359738367UL;
        break;
    case 36:
        maxVal = 68719476735UL;
        break;
    case 37:
        maxVal = 137438953471UL;
        break;
    case 38:
        maxVal = 274877906943UL;
        break;
    case 39:
        maxVal = 549755813887UL;
        break;
    case 40:
        maxVal = 1099511627775UL;
        break;
    case 41:
        maxVal = 2199023255551UL;
        break;
    case 42:
        maxVal = 4398046511103UL;
        break;
    case 43:
        maxVal = 8796093022207UL;
        break;
    case 44:
        maxVal = 17592186044415UL;
        break;
    case 45:
        maxVal = 35184372088831UL;
        break;
    case 46:
        maxVal = 70368744177663UL;
        break;
    case 47:
        maxVal = 140737488355327UL;
        break;
    case 48:
        maxVal = 281474976710655UL;
        break;
    case 49:
        maxVal = 562949953421311UL;
        break;
    case 50:
        maxVal = 1125899906842623UL;
        break;
    case 51:
        maxVal = 2251799813685247UL;
        break;
    case 52:
        maxVal = 4503599627370495UL;
        break;
    case 53:
        maxVal = 9007199254740991UL;
        break;
    case 54:
        maxVal = 18014398509481983UL;
        break;
    case 55:
        maxVal = 36028797018963967UL;
        break;
    case 56:
        maxVal = 72057594037927935UL;
        break;
    case 57:
        maxVal = 144115188075855871UL;
        break;
    case 58:
        maxVal = 288230376151711743UL;
        break;
    case 59:
        maxVal = 576460752303423487UL;
        break;
    case 60:
        maxVal = 1152921504606846975UL;
        break;
    case 61:
        maxVal = 2305843009213693951UL;
        break;
    case 62:
        maxVal = 4611686018427387903UL;
        break;
    case 63:
        maxVal = 9223372036854775807UL;
        break;
    case 64:
        maxVal = 18446744073709551615UL;
        break;
    }

    uint8_t underscores = 0;
    uint64_t n = 0, n1;
    uint8_t c, d;
    for (size_t i = 0; i < s_len; i++)
    {
        c = s[i];
        if (c == '_' && base0)
        {
            underscores = 1;
            continue;
        }
        else if ('0' <= c && c <= '9')
        {
            d = c - '0';
        }
        else
        {
            d = PPP_C_FLAGS_LOGER(c);
            if ('a' <= d && d <= 'z')
            {
                d -= 'a';
                d += 10;
            }
            else
            {
                return -1;
            }
        }
        if (d >= base)
        {
            return -1;
        }

        if (n >= cutoff)
        {
            // n*base overflows
            if (output)
            {
                *output = maxVal;
            }
            return 1;
        }
        n *= base;

        n1 = n + d;
        if (n1 < n || n1 > maxVal)
        {
            // n+d overflows
            if (output)
            {
                *output = maxVal;
            }
            return 1;
        }
        n = n1;
    }

    if (underscores && !ppp_c_flags_underscore_ok(s0, s0_len))
    {
        return -1;
    }
    if (output)
    {
        *output = n;
    }
    return 0;
}

int ppp_c_flags_parse_int64(
    const char *s, size_t s_len,
    int base, int bit_size,
    int64_t *output)
{
    if (!s || !s_len)
    {
        return -1;
    }
    // Pick off leading sign.
    const char *s0 = s;
    size_t s0_len = s_len;
    uint8_t neg = 0;
    switch (s[0])
    {
    case '-':
        neg = 1;
    case '+':
        s++;
        s_len--;
        break;
    }

    // Convert unsigned and check range.
    uint64_t un = 0;
    int err = ppp_c_flags_parse_uint64(s, s_len, base, bit_size, &un);
    if (err && err != 1)
    {
        return err;
    }
    if (bit_size == 0)
    {
        bit_size = 63;
    }
    else
    {
        bit_size--;
    }
    uint64_t cutoff = 1;
    cutoff <<= bit_size;
    if (neg)
    {
        if (un > cutoff)
        {
            if (output)
            {
                *output = -(int64_t)(cutoff);
            }
            return 1;
        }
    }
    else if (un >= cutoff)
    {
        if (output)
        {
            *output = cutoff - 1;
        }
        return 1;
    }
    if (output)
    {
        *output = un;
        if (neg)
        {
            *output = -*output;
        }
    }
    return 0;
}
int ppp_c_flags_parse_bool(const char *s, const size_t s_len)
{
    switch (s_len)
    {
    case 1:
        switch (s[0])
        {
        case '1':
        case 't':
        case 'T':
            return 1;
        case '0':
        case 'f':
        case 'F':
            return 0;
        }
        break;
    case 4:
        if (!memcmp(s, "true", 4) ||
            !memcmp(s, "TRUE", 4) ||
            !memcmp(s, "True", 4))
        {
            return 1;
        }
        break;
    case 5:
        if (!memcmp(s, "false", 5) ||
            !memcmp(s, "FALSE", 5) ||
            !memcmp(s, "False", 5))
        {
            return 0;
        }
        break;
    }
    return -1;
}
void ppp_c_flags_alloctor(ppp_c_flags_malloc_f m, ppp_c_flags_free_f f)
{
    ppp_c_flags_malloc = m ? m : malloc;
    ppp_c_flags_free = f ? f : free;
}
static uint8_t ppp_c_flags_check_flag(const char *s, const size_t s_len)
{
    if (!s || !s_len)
    {
        return 0;
    }
    for (size_t i = 0; i < s_len; i++)
    {
        switch (s[i])
        {
        case '"':
        case '\'':
        case '\r':
        case '\n':
            return 0;
        }
    }

    return 1;
}
static uint8_t ppp_c_flags_check_command(const char *s, const size_t s_len)
{
    if (!s || !s_len)
    {
        return 0;
    }
    if (s[0] == '-')
    {
        return 0;
    }
    for (size_t i = 0; i < s_len; i++)
    {
        switch (s[i])
        {
        case '"':
        case '\'':
        case '\r':
        case '\n':
            return 0;
        }
    }

    return 1;
}
ppp_c_flags_command_t *ppp_c_flags_command_create_with_len(
    const char *name, const size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!ppp_c_flags_check_command(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    ppp_c_flags_command_t *c = ppp_c_flags_malloc(sizeof(ppp_c_flags_command_t));
    if (!c)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_COMMAND;
        }
        return 0;
    }
    c->_parent = 0;
    c->_next = 0;
    c->_child = 0;
    c->_flag = 0;

    c->_name = name;
    c->_name_len = name_len;
    c->_describe = describe;

    c->handler = handler;
    c->userdata = userdata;
    if (err)
    {
        *err = 0;
    }
    return c;
}
ppp_c_flags_command_t *ppp_c_flags_command_create(
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    return ppp_c_flags_command_create_with_len(
        name, strlen(name),
        describe,
        handler, userdata,
        err);
}
void ppp_c_flags_command_destroy(ppp_c_flags_command_t *c)
{
    if (c->_next)
    {
        ppp_c_flags_command_destroy(c->_next);
    }
    if (c->_child)
    {

        ppp_c_flags_command_destroy(c->_child);
    }

    // destroy flags
    struct ppp_c_flags_flag *flag = c->_flag;
    struct ppp_c_flags_flag *flag_next;
    PPP_C_FLAGS_BOOL_ARRAY *arrs;
    while (flag)
    {
        flag_next = flag->_next;
        switch (flag->_type)
        {
        case PPP_C_FLAGS_TYPE_BOOL_ARRAY:
        case PPP_C_FLAGS_TYPE_INT8_ARRAY:
        case PPP_C_FLAGS_TYPE_INT16_ARRAY:
        case PPP_C_FLAGS_TYPE_INT32_ARRAY:
        case PPP_C_FLAGS_TYPE_INT64_ARRAY:
        case PPP_C_FLAGS_TYPE_UINT8_ARRAY:
        case PPP_C_FLAGS_TYPE_UINT16_ARRAY:
        case PPP_C_FLAGS_TYPE_UINT32_ARRAY:
        case PPP_C_FLAGS_TYPE_UINT64_ARRAY:
        case PPP_C_FLAGS_TYPE_FLOAT32_ARRAY:
        case PPP_C_FLAGS_TYPE_FLOAT64_ARRAY:
        case PPP_C_FLAGS_TYPE_STRING_ARRAY:
            arrs = flag->_value;
            if (arrs->cap)
            {
                ppp_c_flags_free(arrs->p);
                arrs->p = 0;
                arrs->cap = 0;
                arrs->len = 0;
            }
            break;
        }
        ppp_c_flags_free(flag);
        flag = flag_next;
    }
    ppp_c_flags_free(c);
}
ppp_c_flags_command_t *ppp_c_flags_add_command_with_len(
    ppp_c_flags_command_t *parent,
    const char *name, size_t name_len,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!ppp_c_flags_check_command(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    ppp_c_flags_command_t *back = parent->_child;
    if (back)
    {
        while (back->_next)
        {
            if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND;
                }
                return 0;
            }
            back = back->_next;
        }
        if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_COMMAND;
            }
            return 0;
        }
    }

    ppp_c_flags_command_t *c = ppp_c_flags_malloc(sizeof(ppp_c_flags_command_t));
    if (!c)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_COMMAND;
        }
        return 0;
    }
    c->_parent = parent;
    c->_next = 0;
    c->_child = 0;
    c->_flag = 0;

    c->_name = name;
    c->_name_len = name_len;
    c->_describe = describe;

    c->handler = handler;
    c->userdata = userdata;

    if (back)
    {
        back->_next = c;
    }
    else
    {
        parent->_child = c;
    }
    if (err)
    {
        *err = 0;
    }
    return c;
}

ppp_c_flags_command_t *ppp_c_flags_add_command(
    ppp_c_flags_command_t *parent,
    const char *name,
    const char *describe,
    ppp_c_flags_command_handler_f handler, void *userdata,
    int *err)
{
    if (!name)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    size_t name_len = strlen(name);
    if (!name_len)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_COMMAND;
        }
        return 0;
    }
    return ppp_c_flags_add_command_with_len(
        parent,
        name, name_len,
        describe,
        handler, userdata,
        err);
}
ppp_c_flags_flag_t *ppp_c_flags_add_flag_with_len(
    ppp_c_flags_command_t *command,
    const char *name, const size_t name_len, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err)
{
    if (!ppp_c_flags_check_flag(name, name_len))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    else if (name_len == 4 && !memcmp(name, "help", 4))
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_HELP;
        }
        return 0;
    }
    else if (short_name == 'h')
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_SHORT_HELP;
        }
        return 0;
    }

    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_BOOL:
    case PPP_C_FLAGS_TYPE_INT8:
    case PPP_C_FLAGS_TYPE_INT16:
    case PPP_C_FLAGS_TYPE_INT32:
    case PPP_C_FLAGS_TYPE_INT64:
    case PPP_C_FLAGS_TYPE_UINT8:
    case PPP_C_FLAGS_TYPE_UINT16:
    case PPP_C_FLAGS_TYPE_UINT32:
    case PPP_C_FLAGS_TYPE_UINT64:
    case PPP_C_FLAGS_TYPE_FLOAT32:
    case PPP_C_FLAGS_TYPE_FLOAT64:
    case PPP_C_FLAGS_TYPE_STRING:

    case PPP_C_FLAGS_TYPE_BOOL_ARRAY:
    case PPP_C_FLAGS_TYPE_INT8_ARRAY:
    case PPP_C_FLAGS_TYPE_INT16_ARRAY:
    case PPP_C_FLAGS_TYPE_INT32_ARRAY:
    case PPP_C_FLAGS_TYPE_INT64_ARRAY:
    case PPP_C_FLAGS_TYPE_UINT8_ARRAY:
    case PPP_C_FLAGS_TYPE_UINT16_ARRAY:
    case PPP_C_FLAGS_TYPE_UINT32_ARRAY:
    case PPP_C_FLAGS_TYPE_UINT64_ARRAY:
    case PPP_C_FLAGS_TYPE_FLOAT32_ARRAY:
    case PPP_C_FLAGS_TYPE_FLOAT64_ARRAY:
    case PPP_C_FLAGS_TYPE_STRING_ARRAY:
        break;
    default:
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG_TYPE;
        }
        return 0;
    }
    struct ppp_c_flags_flag *back = command->_flag;
    if (back)
    {
        while (back->_next)
        {
            if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
                }
                return 0;
            }
            else if (short_name && short_name == back->_short_name)
            {
                if (err)
                {
                    *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
                }
                return 0;
            }
            back = back->_next;
        }
        if (name_len == back->_name_len && !memcmp(name, back->_name, name_len))
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG;
            }
            return 0;
        }
        else if (short_name && short_name == back->_short_name)
        {
            if (err)
            {
                *err = PPP_C_FLAGS_ERROR_DUPLICATE_FLAG_SHORT;
            }
            return 0;
        }
    }

    struct ppp_c_flags_flag *flag = ppp_c_flags_malloc(sizeof(struct ppp_c_flags_flag));
    if (!flag)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_MALLOC_FLAG;
        }
        return 0;
    }
    memset(flag, 0, sizeof(struct ppp_c_flags_flag));
    // flag->_next = 0;

    void *p = &flag->_default;
    switch (value_type)
    {
    case PPP_C_FLAGS_TYPE_BOOL:
        *(PPP_C_FLAGS_BOOL *)p = *(PPP_C_FLAGS_BOOL *)value;
        flag->_handler = &ppp_c_flags_handler_bool;
        break;
    case PPP_C_FLAGS_TYPE_INT8:
        *(PPP_C_FLAGS_INT8 *)p = *(PPP_C_FLAGS_INT8 *)value;
        flag->_handler = &ppp_c_flags_handler_int8;
        break;
    case PPP_C_FLAGS_TYPE_INT16:
        *(PPP_C_FLAGS_INT16 *)p = *(PPP_C_FLAGS_INT16 *)value;
        flag->_handler = &ppp_c_flags_handler_int16;
        break;
    case PPP_C_FLAGS_TYPE_INT32:
        *(PPP_C_FLAGS_INT32 *)p = *(PPP_C_FLAGS_INT32 *)value;
        flag->_handler = &ppp_c_flags_handler_int32;
        break;
    case PPP_C_FLAGS_TYPE_INT64:
        *(PPP_C_FLAGS_INT64 *)p = *(PPP_C_FLAGS_INT64 *)value;
        flag->_handler = &ppp_c_flags_handler_int64;
        break;
    case PPP_C_FLAGS_TYPE_UINT8:
        *(PPP_C_FLAGS_UINT8 *)p = *(PPP_C_FLAGS_UINT8 *)value;
        flag->_handler = &ppp_c_flags_handler_uint8;
        break;
    case PPP_C_FLAGS_TYPE_UINT16:
        *(PPP_C_FLAGS_UINT16 *)p = *(PPP_C_FLAGS_UINT16 *)value;
        flag->_handler = &ppp_c_flags_handler_uint16;
        break;
    case PPP_C_FLAGS_TYPE_UINT32:
        *(PPP_C_FLAGS_UINT32 *)p = *(PPP_C_FLAGS_UINT32 *)value;
        flag->_handler = &ppp_c_flags_handler_uint32;
        break;
    case PPP_C_FLAGS_TYPE_UINT64:
        *(PPP_C_FLAGS_UINT64 *)p = *(PPP_C_FLAGS_UINT64 *)value;
        flag->_handler = &ppp_c_flags_handler_uint64;
        break;
    case PPP_C_FLAGS_TYPE_FLOAT32:
        *(PPP_C_FLAGS_FLOAT32 *)p = *(PPP_C_FLAGS_FLOAT32 *)value;
        flag->_handler = &ppp_c_flags_handler_float32;
        break;
    case PPP_C_FLAGS_TYPE_FLOAT64:
        *(PPP_C_FLAGS_FLOAT64 *)p = *(PPP_C_FLAGS_FLOAT64 *)value;
        flag->_handler = &ppp_c_flags_handler_float64;
        break;
    case PPP_C_FLAGS_TYPE_STRING:
        *(PPP_C_FLAGS_STRING *)p = *(PPP_C_FLAGS_STRING *)value;
        flag->_handler = &ppp_c_flags_handler_string;
        break;

    case PPP_C_FLAGS_TYPE_BOOL_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_bool_array;
        break;
    case PPP_C_FLAGS_TYPE_INT8_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_int8_array;
        break;
    case PPP_C_FLAGS_TYPE_INT16_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_int16_array;
        break;
    case PPP_C_FLAGS_TYPE_INT32_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_int32_array;
        break;
    case PPP_C_FLAGS_TYPE_INT64_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_int64_array;
        break;
    case PPP_C_FLAGS_TYPE_UINT8_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_uint8_array;
        break;
    case PPP_C_FLAGS_TYPE_UINT16_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_uint16_array;
        break;
    case PPP_C_FLAGS_TYPE_UINT32_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_uint32_array;
        break;
    case PPP_C_FLAGS_TYPE_UINT64_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_uint64_array;
        break;
    case PPP_C_FLAGS_TYPE_FLOAT32_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_float32_array;
        break;
    case PPP_C_FLAGS_TYPE_FLOAT64_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_float64_array;
        break;
    case PPP_C_FLAGS_TYPE_STRING_ARRAY:
        *(PPP_C_FLAGS_BOOL_ARRAY *)p = *(PPP_C_FLAGS_BOOL_ARRAY *)value;
        flag->_handler = &ppp_c_flags_handler_string_array;
        break;
    }

    flag->_name = name;
    flag->_name_len = name_len;
    flag->_short_name = short_name;
    flag->_describe = describe;

    flag->_value = value;
    flag->_type = value_type;
    if (back)
    {
        back->_next = flag;
    }
    else
    {
        command->_flag = flag;
    }
    if (err)
    {
        *err = 0;
    }
    return flag;
}
ppp_c_flags_flag_t *ppp_c_flags_add_flag(
    ppp_c_flags_command_t *command,
    const char *name, char short_name,
    const char *describe,
    void *value, const int value_type,
    int *err)
{
    if (!name)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    size_t name_len = strlen(name);
    if (!name_len)
    {
        if (err)
        {
            *err = PPP_C_FLAGS_ERROR_INVALID_FLAG;
        }
        return 0;
    }
    return ppp_c_flags_add_flag_with_len(
        command,
        name, name_len, short_name,
        describe,
        value, value_type,
        err);
}
typedef struct
{
    ppp_c_flags_command_t *command;
    int argc;
    char **argv;
    size_t i;
    char *s;
    size_t s_len;
    int *handler_result;

    int state;
    int err;

    int handler_argc;
    char **handler_argv;
    int handler_cap;

    uint8_t input : 1;
    uint8_t allow_unknow : 1;
    uint8_t short_input : 1;
} ppp_c_flags_execute_args_t;

static int ppp_c_flags_call(ppp_c_flags_execute_args_t *args)
{
    if (args->command->handler)
    {
        int ret = args->command->handler(
            args->command,
            args->handler_argc, args->handler_argv,
            args->command->userdata);
        if (args->handler_result)
        {
            *args->handler_result = ret;
        }
        if (args->handler_argv)
        {
            ppp_c_flags_free(args->handler_argv);
        }
    }
    return 1;
}
static int ppp_c_flags_push_args(ppp_c_flags_execute_args_t *args, char *s)
{
    if (args->handler_argc + 1 > args->handler_cap)
    {
        size_t cap = args->handler_argc ? args->handler_argc * 2 : 32;
        if (cap < 32)
        {
            cap = 32;
        }
        size_t max = args->argc - args->i;
        if (cap > max)
        {
            cap = max;
        }
        if (args->handler_argv)
        {
            void *p = ppp_c_flags_malloc(sizeof(const char *) * cap);
            if (!p)
            {
                ppp_c_flags_free(args->handler_argv);
                printf("malloc argv fail: %s\n", s);
                args->err = PPP_C_FLAGS_ERROR_MALLOC_ARGV;
                return 1;
            }
            if (args->handler_argc)
            {
                memcpy(p, args->handler_argv, args->handler_argc);
            }
            free(args->handler_argv);
            args->handler_argv = p;
        }
        else
        {
            args->handler_argv = ppp_c_flags_malloc(sizeof(const char *) * cap);
            if (!args->handler_argv)
            {
                printf("malloc argv fail: %s\n", s);
                args->err = PPP_C_FLAGS_ERROR_MALLOC_ARGV;
                return 1;
            }
            args->handler_cap = cap;
        }
    }
    args->handler_argv[args->handler_argc++] = s;
    args->i++;
    args->input = 1;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}

static void ppp_c_flags_print_name(ppp_c_flags_command_t *command)
{
    if (command->_parent)
    {
        ppp_c_flags_print_name(command->_parent);
        printf(" %s", command->_name);
    }
    else
    {
        printf("%s", command->_name);
    }
}
static uint8_t ppp_c_flags_is_delimiter(const char c)
{
    switch (c)
    {
    case 0:
    case '\n':
    case '\r':
        return 1;
    }
    return 0;
}

static void ppp_c_flags_print_flag(char *buf, size_t len, ppp_c_flags_flag_t *flag)
{
    if (flag)
    {
        printf("%s ", flag->_name);
        len -= flag->_name_len;
        sprintf(buf + 2, "%lds", len - 1);
        printf(buf, ((ppp_c_flags_flag_handler_t *)(flag->_handler))->name);
    }
    else
    {
        printf("help ");
        len -= 4;
        sprintf(buf + 2, "%lds", len - 1);
        printf(buf, "bool");
    }
}

static void ppp_c_flags_print_usage(ppp_c_flags_command_t *command)
{
    size_t i;
    char buf[2 + 21 + 1] = {0};
    buf[0] = '%';
    buf[1] = '-';
    size_t min;
    printf("Usage:\n  ");
    ppp_c_flags_print_name(command);
    if (command->_child)
    {
        printf(" [flags]\n  ");
        ppp_c_flags_print_name(command);
        printf(" [command]\n\nAvailable Commands:\n");
        ppp_c_flags_command_t *c = command->_child;
        min = 0;
        while (c)
        {
            if (min < c->_name_len)
            {
                min = c->_name_len;
            }
            c = c->_next;
        }
        if (min < 5)
        {
            min = 5;
        }

        sprintf(buf + 2, "%lds", min);
        c = command->_child;
        while (c)
        {
            printf("  ");
            printf(buf, c->_name);
            if (c->_describe && !ppp_c_flags_is_delimiter(c->_describe[0]))
            {
                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(c->_describe[i]); i++)
                {
                    putchar(c->_describe[i]);
                }
                putchar('\n');
            }
            else
            {
                putchar('\n');
            }
            c = c->_next;
        }
        putchar('\n');
    }
    else
    {
        printf(" [flags]\n\n");
    }
    printf("Flags:\n");
    struct ppp_c_flags_flag *flag = command->_flag;

    min = 4 + 1 + 5;
    while (flag)
    {
        i = flag->_name_len + 1 + ((ppp_c_flags_flag_handler_t *)(flag->_handler))->name_len;
        if (min < i)
        {
            min = i;
        }
        flag = flag->_next;
    }
    if (min < 11)
    {
        min = 11;
    }
    flag = command->_flag;
    while (flag)
    {
        if (flag->_short_name)
        {
            printf("  -%c, --", flag->_short_name);
            ppp_c_flags_print_flag(buf, min, flag);

            if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
            {
                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(flag->_describe[i]); i++)
                {
                    putchar(flag->_describe[i]);
                }
                putchar(' ');
            }
            else
            {
                printf("   ");
            }
            ((ppp_c_flags_flag_handler_t *)(flag->_handler))->print_default(flag);
            putchar('\n');
        }
        else
        {
            printf("      --");
            ppp_c_flags_print_flag(buf, min, flag);
            if (flag->_describe && !ppp_c_flags_is_delimiter(flag->_describe[0]))
            {
                printf("   ");
                for (i = 0; !ppp_c_flags_is_delimiter(flag->_describe[i]); i++)
                {
                    putchar(flag->_describe[i]);
                }
                putchar(' ');
            }
            else
            {
                printf("   ");
            }
            ((ppp_c_flags_flag_handler_t *)(flag->_handler))->print_default(flag);
            putchar('\n');
        }
        flag = flag->_next;
    }
    printf("  -h, --");
    ppp_c_flags_print_flag(buf, min, 0);
    printf("   Help for %s\n", command->_name);

    if (command->_child)
    {
        printf("\nUse \"%s [command] --help\" for more information about a command.\n", command->_name);
    }
}
void ppp_c_flags_print(ppp_c_flags_command_t *command)
{
    if (command->_describe)
    {
        puts(command->_describe);
        putchar('\n');
    }
    ppp_c_flags_print_usage(command);
}

static int ppp_c_flags_next_none(ppp_c_flags_execute_args_t *args)
{
    if (args->i >= args->argc)
    {
        return ppp_c_flags_call(args);
    }
    args->s = args->argv[args->i];
    args->s_len = strlen(args->s);
    // --
    if (args->s_len > 1)
    {
        if (args->s[0] == '-')
        {
            if (args->s[1] == '-')
            {
                if (args->s_len > 2)
                {
                    args->s += 2;
                    args->s_len -= 2;
                    args->input = 1;
                    args->state = PPP_C_FLAGS_STATE_FLAGS;
                    return 0;
                }
            }
            else
            {
                args->s++;
                args->s_len--;
                args->input = 1;
                args->short_input = 0;
                args->state = PPP_C_FLAGS_STATE_FLAGS_SHORT;
                return 0;
            }
        }
    }
    // subcommand
    if (!args->input && args->command->_child)
    {
        ppp_c_flags_command_t *c = args->command->_child;
        while (c)
        {
            if (c->_name_len == args->s_len && !memcmp(args->s, c->_name, args->s_len))
            {
                args->command = c;
                args->i++;
                return 0;
            }
            c = c->_next;
        }
    }
    // argv
    return ppp_c_flags_push_args(args, args->s);
}
static int ppp_c_flags_next_flags_set_value(ppp_c_flags_execute_args_t *args, struct ppp_c_flags_flag *flag, uint8_t isshort)
{
    switch (flag->_type)
    {
    case PPP_C_FLAGS_TYPE_BOOL:
    case PPP_C_FLAGS_TYPE_BOOL_ARRAY:
        args->i++;
        args->state = PPP_C_FLAGS_STATE_NONE;
        ((ppp_c_flags_flag_handler_t *)flag->_handler)->set_flag(flag, "1", 1, args->argc - args->i);
        return 0;
    }
    if (args->i + 1 >= args->argc)
    {
        if (isshort)
        {
            printf("Error: flag needs an argument: -%c\n", flag->_short_name);
        }
        else
        {
            printf("Error: flag needs an argument: --%s\n", flag->_name);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->s = args->argv[args->i + 1];
    args->s_len = strlen(args->s);
    if (((ppp_c_flags_flag_handler_t *)(flag->_handler))->set_flag(flag, args->s, args->s_len, args->argc - args->i - 1))
    {
        if (isshort)
        {
            printf("Error: invalid argument for -%c: %s\n", flag->_short_name, args->s);
        }
        else
        {
            printf("Error: invalid argument for --%s: %s\n", flag->_name, args->s);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->i += 2;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}
static int ppp_c_flags_next_flags_set(ppp_c_flags_execute_args_t *args, struct ppp_c_flags_flag *flag, uint8_t isshort)
{
    switch (isshort)
    {
    case 0:
        args->s += (flag->_name_len + 1);
        args->s_len -= (flag->_name_len + 1);
        break;
    case 1:
        args->s += 2;
        args->s_len -= 2;
        break;
        // case 2:
    default:
        args->s++;
        args->s_len--;
        break;
    }
    if (((ppp_c_flags_flag_handler_t *)(flag->_handler))->set_flag(flag, args->s, args->s_len, args->argc - args->i - 1))
    {
        if (isshort)
        {
            printf("Error: invalid argument for -%c: %s\n", flag->_short_name, args->s);
        }
        else
        {
            printf("Error: invalid argument for --%s: %s\n", flag->_name, args->s);
        }
        ppp_c_flags_print_usage(args->command);
        return 1;
    }
    args->i++;
    args->state = PPP_C_FLAGS_STATE_NONE;
    return 0;
}
static int ppp_c_flags_next_flags(ppp_c_flags_execute_args_t *args)
{
    // --help
    if (args->s_len == 4 && !memcmp(args->s, "help", 4))
    {
        ppp_c_flags_print(args->command);
        return 1;
    }
    else if (args->s_len > 4 && !memcmp(args->s, "help=", 5))
    {
        args->s += 5;
        args->s_len -= 5;
        switch (ppp_c_flags_parse_bool(args->s, args->s_len))
        {
        case 0:
            args->i++;
            args->state = PPP_C_FLAGS_STATE_NONE;
            return 0;
        case 1:
            ppp_c_flags_print(args->command);
            return 1;
        default:
            printf("Error: invalid argument for --help: %s\n", args->s);
            ppp_c_flags_print_usage(args->command);
            args->err = PPP_C_FLAGS_ERROR_INVALID_ARGUMENT;
            return 1;
        }
    }
    else
    {
        struct ppp_c_flags_flag *flag = args->command->_flag;
        while (flag)
        {
            if (args->s_len == flag->_name_len &&
                !memcmp(args->s, flag->_name, flag->_name_len))
            {
                return ppp_c_flags_next_flags_set_value(args, flag, 0);
            }
            else if (args->s_len > flag->_name_len &&
                     args->s[flag->_name_len] == '=' &&
                     !memcmp(args->s, flag->_name, flag->_name_len))
            {
                return ppp_c_flags_next_flags_set(args, flag, 0);
            }
            flag = flag->_next;
        }
    }
    if (args->allow_unknow)
    {
        // argv
        return ppp_c_flags_push_args(args, args->s - 2);
    }
    printf("Error: unknown flag: --%s\n", args->s);
    ppp_c_flags_print_usage(args->command);
    args->err = PPP_C_FLAGS_ERROR_UNKNOW_FLAG;
    return 1;
}
static int ppp_c_flags_next_short(ppp_c_flags_execute_args_t *args)
{
    // -h
    if (args->s_len == 1 && args->s[0] == 'h')
    {
        ppp_c_flags_print(args->command);
        return 1;
    }
    else if (args->s_len > 1 && !memcmp(args->s, "h=", 2))
    {
        args->s += 2;
        args->s_len -= 2;
        switch (ppp_c_flags_parse_bool(args->s, args->s_len))
        {
        case 0:
            args->i++;
            args->state = PPP_C_FLAGS_STATE_NONE;
            return 0;
        case 1:
            ppp_c_flags_print(args->command);
            return 1;
        default:
            printf("Error: invalid argument for -h: %s\n", args->s);
            ppp_c_flags_print_usage(args->command);
            args->err = PPP_C_FLAGS_ERROR_INVALID_ARGUMENT;
            return 1;
        }
    }
    else
    {
        struct ppp_c_flags_flag *flag = args->command->_flag;
        while (flag)
        {
            if (flag->_short_name)
            {
                if (args->s_len == 1 &&
                    args->s[0] == flag->_short_name)
                {
                    return ppp_c_flags_next_flags_set_value(args, flag, 1);
                }
                else if (args->s_len > 1 &&
                         args->s[0] == flag->_short_name)
                {
                    if (args->s[1] == '=')
                    {
                        return ppp_c_flags_next_flags_set(args, flag, 1);
                    }
                    switch (flag->_type)
                    {
                    case PPP_C_FLAGS_TYPE_BOOL:
                    case PPP_C_FLAGS_TYPE_BOOL_ARRAY:
                        args->s++;
                        args->s_len--;
                        ((ppp_c_flags_flag_handler_t *)flag->_handler)->set_flag(flag, "1", 1, args->argc - args->i);
                        args->short_input = 1;
                        return 0;

                    default:
                        return ppp_c_flags_next_flags_set(args, flag, 2);
                    }
                }
            }
            flag = flag->_next;
        }
    }
    if (!args->short_input && args->allow_unknow)
    {
        // argv
        return ppp_c_flags_push_args(args, args->s - 1);
    }
    printf("Error: unknown short flag: -%c\n", args->s[0]);
    ppp_c_flags_print_usage(args->command);
    args->err = PPP_C_FLAGS_ERROR_UNKNOW_SHORT_FLAG;
    return 1;
}

int ppp_c_flags_execute(
    ppp_c_flags_command_t *root,
    int argc, char **argv,
    uint8_t allow_unknow, int *handler_result)
{
    ppp_c_flags_execute_args_t args = {0};
    args.command = root;
    args.argc = argc;
    args.argv = argv;
    args.handler_result = handler_result;
    args.allow_unknow = allow_unknow;
    if (handler_result)
    {
        *handler_result = 0;
    }
    while (1)
    {
        switch (args.state)
        {
        case PPP_C_FLAGS_STATE_NONE:
            if (ppp_c_flags_next_none(&args))
            {
                return args.err;
            }
            break;
        case PPP_C_FLAGS_STATE_FLAGS:
            if (ppp_c_flags_next_flags(&args))
            {
                return args.err;
            }
            break;
        case PPP_C_FLAGS_STATE_FLAGS_SHORT:
            if (ppp_c_flags_next_short(&args))
            {
                return args.err;
            }
            break;
        }
    }
    return args.err;
}