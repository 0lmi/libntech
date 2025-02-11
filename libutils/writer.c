/*
  Copyright 2023 Northern.tech AS

  This file is part of CFEngine 3 - written and maintained by Northern.tech AS.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; version 3.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of CFEngine, the applicable Commercial Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#include <platform.h>
#include <writer.h>

#include <misc_lib.h>
#include <alloc.h>

typedef enum
{
    WT_STRING,
    WT_FILE,
} WriterType;

typedef struct
{
    char *data;
    size_t len;                 /* Does not include trailing zero */
    size_t allocated;           /* Includes trailing zero */
} StringWriterImpl;

struct Writer_
{
    WriterType type;
    union
    {
        StringWriterImpl string;
        FILE *file;
    };
};

/*********************************************************************/

Writer *FileWriter(FILE *file)
{
    Writer *writer = xcalloc(1, sizeof(Writer));

    writer->type = WT_FILE;
    writer->file = file;
    return writer;
}

/*********************************************************************/

Writer *StringWriter(void)
{
    Writer *writer = xcalloc(1, sizeof(Writer));

    writer->type = WT_STRING;
    writer->string.data = xstrdup("");
    writer->string.allocated = 1;
    writer->string.len = 0;
    return writer;
}

/*********************************************************************/

static void StringWriterReallocate(Writer *writer, size_t extra_length)
{
    assert(writer != NULL);
    writer->string.allocated = MAX(writer->string.allocated * 2, writer->string.len + extra_length + 1);
    writer->string.data = xrealloc(writer->string.data, writer->string.allocated);
}

static size_t StringWriterWriteChar(Writer *writer, char c)
{
    assert(writer != NULL);
    if (writer->string.len + 2 > writer->string.allocated)
    {
        StringWriterReallocate(writer, 2);
    }

    writer->string.data[writer->string.len] = c;
    writer->string.data[writer->string.len + 1] = '\0';
    writer->string.len++;

    return 1;
}

static size_t StringWriterWriteLen(Writer *writer, const char *str, size_t len_)
{
    assert(writer != NULL);
    /* NB: str[:len_] may come from read(), which hasn't '\0'-terminated */
    size_t len = strnlen(str, len_);

    if (writer->string.len + len + 1 > writer->string.allocated)
    {
        StringWriterReallocate(writer, len);
    }

    memcpy(writer->string.data + writer->string.len, str, len);
    writer->string.data[writer->string.len + len] = '\0';
    writer->string.len += len;

    return len;
}

/*********************************************************************/

static size_t FileWriterWriteF(Writer *writer, const char *fmt, va_list ap)
{
    assert(writer != NULL);
    return vfprintf(writer->file, fmt, ap);
}

/*********************************************************************/

static size_t FileWriterWriteLen(Writer *writer, const char *str, size_t len_)
{
    assert(writer != NULL);
    size_t len = strnlen(str, len_);

#ifdef CFENGINE_TEST
    return CFENGINE_TEST_fwrite(str, 1, len, writer->file);
#else
    return fwrite(str, 1, len, writer->file);
#endif
}

/*********************************************************************/

size_t WriterWriteF(Writer *writer, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    size_t size = WriterWriteVF(writer, fmt, ap);

    va_end(ap);
    return size;
}

/*********************************************************************/

size_t WriterWriteVF(Writer *writer, const char *fmt, va_list ap)
{
    assert(writer != NULL);
    if (writer->type == WT_STRING)
    {
        char *str = NULL;

        xvasprintf(&str, fmt, ap);
        size_t size = StringWriterWriteLen(writer, str, INT_MAX);

        free(str);
        return size;
    }
    else
    {
        return FileWriterWriteF(writer, fmt, ap);
    }
}

/*********************************************************************/

size_t WriterWriteLen(Writer *writer, const char *str, size_t len)
{
    assert(writer != NULL);
    if (writer->type == WT_STRING)
    {
        return StringWriterWriteLen(writer, str, len);
    }
    else
    {
        return FileWriterWriteLen(writer, str, len);
    }
}

/*********************************************************************/

size_t WriterWrite(Writer *writer, const char *str)
{
    return WriterWriteLen(writer, str, INT_MAX);
}

/*********************************************************************/

size_t WriterWriteChar(Writer *writer, char c)
{
    assert(writer != NULL);
    if (writer->type == WT_STRING)
    {
        return StringWriterWriteChar(writer, c);
    }
    else
    {
        char s[2] = { c, '\0' };
        return FileWriterWriteLen(writer, s, 1);
    }
}

/*********************************************************************/

size_t StringWriterLength(const Writer *writer)
{
    assert(writer != NULL);
    if (writer->type != WT_STRING)
    {
        ProgrammingError("Wrong writer type");
    }

    return writer->string.len;
}

/*********************************************************************/

const char *StringWriterData(const Writer *writer)
{
    assert(writer != NULL);
    if (writer->type != WT_STRING)
    {
        ProgrammingError("Wrong writer type");
    }

    return writer->string.data;
}

/*********************************************************************/

void WriterClose(Writer *writer)
{
    assert(writer != NULL);
    if (writer->type == WT_STRING)
    {
        free(writer->string.data);
    }
    else
    {
#ifdef CFENGINE_TEST
        CFENGINE_TEST_fclose(writer->file);
#else
        fclose(writer->file);
#endif
    }
    free(writer);
}

/*********************************************************************/

char *StringWriterClose(Writer *writer)
// NOTE: transfer of ownership for allocated return value
{
    assert(writer != NULL);

    if (writer->type != WT_STRING)
    {
        ProgrammingError("Wrong writer type");
    }
    char *data = writer->string.data;

    free(writer);
    return data;
}

FILE *FileWriterDetach(Writer *writer)
{
    assert(writer != NULL);
    if (writer->type != WT_FILE)
    {
        ProgrammingError("Wrong writer type");
    }
    FILE *file = writer->file;
    free(writer);
    return file;
}

static void WriterWriteOptions(Writer *w, const struct option options[],
                               const char *const hints[])
{
    WriterWriteF(w, "\nOptions:\n");
    for (int i = 0; options[i].name != NULL; i++)
    {
        char short_option[] = ", -*";
        if (options[i].val < 128)
        {
            // Within ASCII range, means there is a short option.
            short_option[3] = options[i].val;
        }
        else
        {
            // No short option.
            short_option[0] = '\0';
        }
        if (options[i].has_arg)
        {
            WriterWriteF(w, "  --%-12s%s value - %s\n", options[i].name,
                         short_option, hints[i]);
        }
        else
        {
            WriterWriteF(w, "  --%-12s%-10s - %s\n", options[i].name,
                         short_option, hints[i]);
        }
    }
}

static void WriterWriteCommands(Writer *w, const Description *commands)
{
    assert(commands != NULL);
    WriterWriteF(w, "\nCommands:\n");
    for (int i = 0; commands[i].name != NULL; i++)
    {
        WriterWriteF(w, "  %-12s - %s.\n", commands[i].name,
                                           commands[i].description);
        WriterWriteF(w, "  %-12s   Usage: %s\n", "", commands[i].usage);
    }
}

void WriterWriteHelp(Writer *w, const char *component,
                     const struct option options[], const char *const hints[],
                     const Description *commands, bool command_first,
                     bool accepts_file_argument)
{
    WriterWriteF(w, "Usage: %s%s [OPTIONS]%s%s\n", component,
                 (commands && command_first) ? " COMMAND" : "",
                 (commands && !command_first) ? " COMMAND" : "",
                 accepts_file_argument ? " [FILE]" : "");
    if ((commands != NULL) && command_first)
    {
        WriterWriteCommands(w, commands);
    }
    WriterWriteOptions(w, options, hints);
    if ((commands != NULL) && !command_first)
    {
        WriterWriteCommands(w, commands);
    }
    WriterWriteF(w, "\nWebsite: https://cfengine.com\n");
    WriterWriteF(w, "This software is Copyright 2023 Northern.tech AS.\n ");
}
