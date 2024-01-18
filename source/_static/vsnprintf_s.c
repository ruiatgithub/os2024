#include <stdarg.h>

static void ftoa_fixed(char *buffer, double value);
static void ftoa_sci(char *buffer, double value);

int strlen(const char *s)
{
    const char *sc = (void *)0 ;

    for (sc = s; *sc != '\0'; ++sc) /* nothing */
        ;

    return sc - s;
}

/* private function */
#define _ISDIGIT(c)  ((unsigned)((c) - '0') < 10)

/**
 * @brief  This function will duplicate a string.
 *
 * @param  n is the string to be duplicated.
 *
 * @param  base is support divide instructions value.
 *
 * @return the duplicated string pointer.
 */
#ifdef RT_KPRINTF_USING_LONGLONG
inline int divide(unsigned long long *n, int base)
#else
inline int divide(unsigned long *n, int base)
#endif /* RT_KPRINTF_USING_LONGLONG */
{
    int res;

    /* optimized for processor which does not support divide instructions. */
#ifdef RT_KPRINTF_USING_LONGLONG
    res = (int)((*n) % base);
    *n = (long long)((*n) / base);
#else
    res = (int)((*n) % base);
    *n = (long)((*n) / base);
#endif

    return res;
}

int skip_atoi(const char **s)
{
    int i = 0;
    while (_ISDIGIT(**s))
        i = i * 10 + *((*s)++) - '0';

    return i;
}


#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */

#define RT_PRINTF_PRECISION

static char *print_number(char *buf,
                          char *end,
#ifdef RT_KPRINTF_USING_LONGLONG
                          unsigned long long  num,
#else
                          unsigned long  num,
#endif /* RT_KPRINTF_USING_LONGLONG */
                          int   base,
                          int   qualifier,
                          int   s,
#ifdef RT_PRINTF_PRECISION
                          int   precision,
#endif /* RT_PRINTF_PRECISION */
                          int   type)
{
    char c = 0, sign = 0;
#ifdef RT_KPRINTF_USING_LONGLONG
    char tmp[64] = {0};
#else
    char tmp[32] = {0};
#endif /* RT_KPRINTF_USING_LONGLONG */
    int precision_bak = precision;
    const char *digits = (void *)0;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    int i = 0;
    int size = 0;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
    {
        type &= ~ZEROPAD;
    }

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        switch (qualifier)
        {
        case 'h':
            if ((short)num < 0)
            {
                sign = '-';
                num = (unsigned short)-num;
            }
            break;
        case 'L':
        case 'l':
            if ((long)num < 0)
            {
                sign = '-';
                num = (unsigned long)-num;
            }
            break;
        case 0:
        default:
            if ((int)num < 0)
            {
                sign = '-';
                num = (unsigned int)-num;
            }
            break;
        }

        if (sign != '-')
        {
            if (type & PLUS)
            {
                sign = '+';
            }
            else if (type & SPACE)
            {
                sign = ' ';
            }
        }
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 2 || base == 16)
        {
            size -= 2;
        }
        else if (base == 8)
        {
            size--;
        }
    }
#endif /* RT_PRINTF_SPECIAL */

    i = 0;
    if (num == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

#ifdef RT_PRINTF_PRECISION
    if (i > precision)
    {
        precision = i;
    }
    size -= precision;
#else
    size -= i;
#endif /* RT_PRINTF_PRECISION */

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
        {
            size--;
        }

        while (size-- > 0)
        {
            if (buf < end)
            {
                *buf = ' ';
            }

            ++ buf;
        }
    }

    if (sign)
    {
        if (buf < end)
        {
            *buf = sign;
        }
        -- size;
        ++ buf;
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 2)
        {
            if (buf < end)
                *buf = '0';
            ++ buf;
            if (buf < end)
                *buf = 'b';
            ++ buf;
        }
        else if (base == 8)
        {
            if (buf < end)
                *buf = '0';
            ++ buf;
        }
        else if (base == 16)
        {
            if (buf < end)
            {
                *buf = '0';
            }

            ++ buf;
            if (buf < end)
            {
                *buf = type & LARGE ? 'X' : 'x';
            }
            ++ buf;
        }
    }
#endif /* RT_PRINTF_SPECIAL */

    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf < end)
            {
                *buf = c;
            }

            ++ buf;
        }
    }

#ifdef RT_PRINTF_PRECISION
    while (i < precision--)
    {
        if (buf < end)
        {
            *buf = '0';
        }

        ++ buf;
    }
#endif /* RT_PRINTF_PRECISION */

    /* put number in the temporary buffer */
    while (i-- > 0 && (precision_bak != 0))
    {
        if (buf < end)
        {
            *buf = tmp[i];
        }

        ++ buf;
    }

    while (size-- > 0)
    {
        if (buf < end)
        {
            *buf = ' ';
        }

        ++ buf;
    }

    return buf;
}

/**
 * @brief  This function will fill a formatted string to buffer.
 *
 * @param  buf is the buffer to save formatted string.
 *
 * @param  size is the size of buffer.
 *
 * @param  fmt is the format parameters.
 *
 * @param  args is a list of variable parameters.
 *
 * @return The number of characters actually written to buffer.
 */
int vsnprintf_s(char *buf, int size, int no_count, const char *fmt, va_list args)
{
#ifdef RT_KPRINTF_USING_LONGLONG
    unsigned long long num = 0;
#else
    unsigned long num = 0;
#endif /* RT_KPRINTF_USING_LONGLONG */
    int i = 0, len = 0;
    char *str = (void *)0, *end = (void *)0, c = 0;
    const char *s = (void *)0;

    unsigned char base = 0;            /* the base of number */
    unsigned char flags = 0;           /* flags to print number */
    unsigned char qualifier = 0;       /* 'h', 'l', or 'L' for integer fields */
    int field_width = 0;     /* width of output field */

#ifdef RT_PRINTF_PRECISION
    int precision = 0;      /* min. # of digits for integers and max for a string */
#endif /* RT_PRINTF_PRECISION */

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *) - 1);
        size = end - buf;
    }

    for (; *fmt ; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str < end)
            {
                *str = *fmt;
            }

            ++ str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1)
        {
            /* skips the first '%' also */
            ++ fmt;
            if (*fmt == '-') flags |= LEFT;
            else if (*fmt == '+') flags |= PLUS;
            else if (*fmt == ' ') flags |= SPACE;
            else if (*fmt == '#') flags |= SPECIAL;
            else if (*fmt == '0') flags |= ZEROPAD;
            else break;
        }

        /* get field width */
        field_width = -1;
        if (_ISDIGIT(*fmt))
        {
            field_width = skip_atoi(&fmt);
        }
        else if (*fmt == '*')
        {
            ++ fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

#ifdef RT_PRINTF_PRECISION
        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++ fmt;
            if (_ISDIGIT(*fmt))
            {
                precision = skip_atoi(&fmt);
            }
            else if (*fmt == '*')
            {
                ++ fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
            {
                precision = 0;
            }
        }
#endif /* RT_PRINTF_PRECISION */
        /* get the conversion qualifier */
        qualifier = 0;
#ifdef RT_KPRINTF_USING_LONGLONG
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
#else
        if (*fmt == 'h' || *fmt == 'l')
#endif /* RT_KPRINTF_USING_LONGLONG */
        {
            qualifier = *fmt;
            ++ fmt;
#ifdef RT_KPRINTF_USING_LONGLONG
            if (qualifier == 'l' && *fmt == 'l')
            {
                qualifier = 'L';
                ++ fmt;
            }
#endif /* RT_KPRINTF_USING_LONGLONG */
        }

        /* the default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str < end) *str = ' ';
                    ++ str;
                }
            }

            /* get character */
            c = (unsigned char)va_arg(args, int);
            if (str < end)
            {
                *str = c;
            }
            ++ str;

            /* put width */
            while (--field_width > 0)
            {
                if (str < end) *str = ' ';
                ++ str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s)
            {
                s = "(NULL)";
            }

            for (len = 0; (len != field_width) && (s[len] != '\0'); len++);
#ifdef RT_PRINTF_PRECISION
            if (precision > 0 && len > precision)
            {
                len = precision;
            }
#endif /* RT_PRINTF_PRECISION */

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str < end) *str = ' ';
                    ++ str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str < end) *str = *s;
                ++ str;
                ++ s;
            }

            while (len < field_width--)
            {
                if (str < end) *str = ' ';
                ++ str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
#ifdef RT_PRINTF_SPECIAL
                field_width += 2; /* `0x` prefix */
                flags |= SPECIAL;
#endif
                flags |= ZEROPAD;
            }
#ifdef RT_PRINTF_PRECISION
            str = print_number(str, end,
                               (unsigned long)va_arg(args, void *),
                               16, qualifier, field_width, precision, flags);
#else
            str = print_number(str, end,
                               (unsigned long)va_arg(args, void *),
                               16, qualifier, field_width, flags);
#endif
            continue;

        case '%':
            if (str < end)
            {
                *str = '%';
            }
            ++ str;
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'b':
            base = 2;
            break;
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str < end)
            {
                *str = '%';
            }
            ++ str;

            if (*fmt)
            {
                if (str < end)
                {
                    *str = *fmt;
                }
                ++ str;
            }
            else
            {
                -- fmt;
            }
            continue;
        }

#ifdef RT_KPRINTF_USING_LONGLONG
        if (qualifier == 'L')
        {
            num = va_arg(args, unsigned long long);
        }
        else if (qualifier == 'l')
#else
        if (qualifier == 'l')
#endif /* RT_KPRINTF_USING_LONGLONG */
        {
            num = va_arg(args, unsigned long);
        }
        else if (qualifier == 'h')
        {
            num = (unsigned short)va_arg(args, int);
            if (flags & SIGN)
            {
                num = (short)num;
            }
        }
        else
        {
            num = (unsigned int)va_arg(args, unsigned long);
        }
#ifdef RT_PRINTF_PRECISION
        str = print_number(str, end, num, base, qualifier, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, qualifier, field_width, flags);
#endif
    }

    if (size > 0)
    {
        if (str < end)
        {
            *str = '\0';
        }
        else
        {
            end[-1] = '\0';
        }
    }

    /* the trailing null byte doesn't count towards the total
    * ++str;
    */
    return str - buf;
}