#include <platform.h>
#include <definitions.h>
#include <string_lib.h>
#include <alloc.h>
#include <regex.h>
#include <encode.h>

#include <test.h>

static const char *lo_alphabet = "abcdefghijklmnopqrstuvwxyz";
static const char *hi_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void test_get_token(void)
{
    {
        const char *str = "  abc def ,efg ";
        size_t len = strlen(str);

        assert_int_equal(3, StringCountTokens(str, len, ", "));

        {
            StringRef ref = StringGetToken(str, len, 0, ", ");
            assert_int_equal(3, ref.len);
            assert_memory_equal("abc", ref.data, 3);
        }

        {
            StringRef ref = StringGetToken(str, len, 1, ", ");
            assert_int_equal(3, ref.len);
            assert_memory_equal("def", ref.data, 3);
        }

        {
            StringRef ref = StringGetToken(str, len, 2, ", ");
            assert_int_equal(3, ref.len);
            assert_memory_equal("efg", ref.data, 3);
        }
    }

    {
        const char *str = "abc";
        size_t len = strlen(str);

        assert_int_equal(1, StringCountTokens(str, len, ", "));

        {
            StringRef ref = StringGetToken(str, len, 0, ", ");
            assert_int_equal(3, ref.len);
            assert_memory_equal("abc", ref.data, 3);
        }
    }

    {
        const char *str = "abc ";
        size_t len = strlen(str);

        assert_int_equal(1, StringCountTokens(str, len, ", "));

        {
            StringRef ref = StringGetToken(str, len, 0, ", ");
            assert_int_equal(3, ref.len);
            assert_memory_equal("abc", ref.data, 3);
        }
    }
}

static void test_mix_case_tolower(void)
{
    char str[] = "aBcD";
    ToLowerStrInplace(str);

    assert_string_equal(str, "abcd");
}

static void test_empty_tolower(void)
{
    char str[] = "";
    ToLowerStrInplace(str);

    assert_string_equal(str, "");
}

static void test_weird_chars_tolower(void)
{
    static const char *weirdstuff = "1345\0xff%$#@!";

    char weirdstuff_copy_lowercased[CF_MAXVARSIZE];
    strlcpy(weirdstuff_copy_lowercased, weirdstuff, CF_MAXVARSIZE);
    ToLowerStrInplace(weirdstuff_copy_lowercased);

    assert_string_equal(weirdstuff_copy_lowercased, weirdstuff);
}

static void test_alphabet_tolower(void)
{
    char lo_alphabet_lowercased[CF_MAXVARSIZE];
    strlcpy(lo_alphabet_lowercased, lo_alphabet, CF_MAXVARSIZE);
    ToLowerStrInplace(lo_alphabet_lowercased);

    assert_string_equal(lo_alphabet_lowercased, lo_alphabet);
}

static void test_hi_alphabet_tolower(void)
{
    char hi_alphabet_lowercased[CF_MAXVARSIZE];
    strlcpy(hi_alphabet_lowercased, hi_alphabet, CF_MAXVARSIZE);
    ToLowerStrInplace(hi_alphabet_lowercased);

    assert_string_equal(hi_alphabet_lowercased, lo_alphabet);
}

static void test_inplace_tolower(void)
{
    char abc[] = "abc";
    char def[] = "def";

    ToLowerStrInplace(abc);
    ToLowerStrInplace(def);

    assert_string_equal(abc, "abc");
    assert_string_equal(def, "def");
}

static void test_mix_case_toupper(void)
{
    char str[] = "aBcD";
    ToUpperStrInplace(str);
    assert_string_equal(str, "ABCD");
}

static void test_empty_toupper(void)
{
    char str[] = "";
    ToUpperStrInplace(str);
    assert_string_equal(str, "");
}

static void test_weird_chars_toupper(void)
{
    static const char *weirdstuff = "1345\0xff%$#@!";

    char weirdstuff_copy_uppercased[CF_MAXVARSIZE];
    strlcpy(weirdstuff_copy_uppercased, weirdstuff, CF_MAXVARSIZE);
    ToUpperStrInplace(weirdstuff_copy_uppercased);

    assert_string_equal(weirdstuff_copy_uppercased, weirdstuff);
}

static void test_alphabet_toupper(void)
{
    char lo_alphabet_uppercased[CF_MAXVARSIZE];
    strlcpy(lo_alphabet_uppercased, lo_alphabet, CF_MAXVARSIZE);
    ToUpperStrInplace(lo_alphabet_uppercased);

    assert_string_equal(lo_alphabet_uppercased, hi_alphabet);
}

static void test_hi_alphabet_toupper(void)
{
    char hi_alphabet_uppercased[CF_MAXVARSIZE];
    strlcpy(hi_alphabet_uppercased, hi_alphabet, CF_MAXVARSIZE);
    ToUpperStrInplace(hi_alphabet_uppercased);

    assert_string_equal(hi_alphabet_uppercased, hi_alphabet);
}

static void test_inplace_toupper(void)
{
    char abc[] = "abc";
    char def[] = "def";

    ToUpperStrInplace(abc);
    ToUpperStrInplace(def);

    assert_string_equal(abc, "ABC");
    assert_string_equal(def, "DEF");
}

static void test_long_search(void)
{
    char *ns = SearchAndReplace("abc", "abcabc", "test");

    assert_string_equal(ns, "abc");
    free(ns);
}

static void test_replace_empty_pattern(void)
{
    char *ns = SearchAndReplace("foobarbaz", "", "abc");

    assert_string_equal(ns, "foobarbaz");
    free(ns);
}

static void test_replace_empty_replacement(void)
{
    char *ns = SearchAndReplace("foobarbaz", "a", "");

    assert_string_equal(ns, "foobrbz");
    free(ns);
}

static void test_replace_eq_size(void)
{
    char *new_string = SearchAndReplace("sasza szedl sucha szosa", "sz", "xx");

    assert_string_equal(new_string, "saxxa xxedl sucha xxosa");
    free(new_string);
}

static void test_replace_more_size(void)
{
    char *new_string = SearchAndReplace("sasza szedl sucha szosa", "sz", "xxx");

    assert_string_equal(new_string, "saxxxa xxxedl sucha xxxosa");
    free(new_string);
}

static void test_replace_less_size(void)
{
    char *new_string = SearchAndReplace("sasza szedl sucha szosa", "sz", "x");

    assert_string_equal(new_string, "saxa xedl sucha xosa");
    free(new_string);
}

static void test_no_replace(void)
{
    char *new_string = SearchAndReplace("sasza szedl sucha szosa",
                                        "no_such_pattern", "x");

    assert_string_equal(new_string, "sasza szedl sucha szosa");
    free(new_string);
}

/****************************************************************************
 * Tests for StringReplace()                                                *
 *   StringReplace and SearchAndReplace differ in that StringReplace errors *
 *   if the buffer is too small, and SearchAndReplace allocates sufficient  *
 *   space.                                                                 *
 ****************************************************************************/

static void test_string_replace_empty_replacement(void)
{
    char string[] = "foobarbaz";
    size_t actual_size = strlen(string);

    ssize_t ret_size = StringReplace(string, actual_size + 1, "a", "");

    // StringReplace returns the length without counting '\0'
    assert_int_not_equal(ret_size, actual_size);
    assert_string_equal(string, "foobrbz");
}

static void test_string_replace_eq_size(void)
{
    char string[] = "sasza szedl sucha szosa";
    size_t actual_size = strlen(string);

    ssize_t ret_size = StringReplace(string, actual_size + 1, "sz", "xx");

    assert_int_equal(ret_size, actual_size);
    assert_string_equal(string, "saxxa xxedl sucha xxosa");
}

static void test_string_replace_buf_too_small(void)
{
    char string[] = "sasza szedl sucha szosa";
    size_t actual_size = sizeof(string);

    ssize_t ret_size = StringReplace(string, actual_size, "sz", "xxx");

    // Errors, `string` is retained
    assert_int_equal(ret_size, (ssize_t) -1);
    assert_string_equal(string, "sasza szedl sucha szosa");
}

static void test_string_replace_smaller(void)
{
    char string[] = "sasza szedl sucha szosa";
    size_t actual_size = strlen(string);

    ssize_t ret_size = StringReplace(string, actual_size + 1, "sz", "x");
    assert_int_not_equal(ret_size, actual_size);
    assert_string_equal(string, "saxa xedl sucha xosa");
}

static void test_string_replace_none(void)
{
    char string[] = "sasza szedl sucha szosa";
    size_t actual_size = strlen(string);

    ssize_t ret_size = StringReplace(string, actual_size + 1,
                                     "no_such_pattern", "x");

    assert_int_equal(ret_size, 0);
    assert_string_equal(string, "sasza szedl sucha szosa");
}

static void test_string_replace_many_percentages(void)
{
    char string[29] = "%%%%%%%%%%%%%%";
    size_t actual_size = sizeof(string);

    ssize_t ret_size = StringReplace(string, actual_size,
                                     "%", "%%");

    assert_int_equal(ret_size, actual_size - 1);
    assert_string_equal(string, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
}

static void test_string_replace_n(void)
{
    char buf[11] = "1,2,3,4\0\0\0"; /* strlen(buf) == 7 */

    ssize_t ret = StringReplaceN(buf, sizeof(buf), ",", "__", 2);
    assert_int_equal(ret, 7 + 2);
    assert_string_equal(buf, "1__2__3,4");

    /* nothing should happen (n == 0) */
    ret = StringReplaceN(buf, sizeof(buf), ",", "__", 0);
    assert_int_equal(ret, 0);
    assert_string_equal(buf, "1__2__3,4");

    /* one replacement possible */
    ret = StringReplaceN(buf, sizeof(buf), ",", "__", 2);
    assert_int_equal(ret, 7 + 3);
    assert_string_equal(buf, "1__2__3__4");

    /* nothing should happen (n == 0) */
    ret = StringReplaceN(buf, sizeof(buf), ",", "__", 0);
    assert_int_equal(ret, 0);
    assert_string_equal(buf, "1__2__3__4");

    /* nothing should happen (nothing to replace) */
    ret = StringReplaceN(buf, sizeof(buf), ",", "__", 5);
    assert_int_equal(ret, 0);
    assert_string_equal(buf, "1__2__3__4");
}

/****************************************************************************/

static void test_concatenate(void)
{
    char *new_string = StringConcatenate(2, "snookie", "sitch");
    assert_string_equal(new_string, "snookiesitch");
    free(new_string);

    new_string = StringConcatenate(4, "a", NULL, "c", "d");
    assert_string_equal(new_string, "acd");
    free(new_string);

    new_string = StringConcatenate(3, "a", "b", "c", "d");
    assert_string_equal(new_string, "abc");
    free(new_string);

    new_string = StringConcatenate(1, "stuff");
    assert_string_equal(new_string, "stuff");
    free(new_string);

    new_string = StringConcatenate(0, NULL);
    assert_false(new_string);
}

static void test_substring_overshoot(void)
{
    char *new_string = StringSubstring("abcdef", 6, 0, 10);

    assert_string_equal(new_string, "abcdef");
    free(new_string);
}

static void test_substring_positive(void)
{
    char *new_string = StringSubstring("abcdef", 6, 2, 3);

    assert_string_equal(new_string, "cde");
    free(new_string);
}

static void test_substring_negative_length(void)
{
    char *new_string = StringSubstring("abcdef", 6, 2, -1);

    assert_string_equal(new_string, "cde");
    free(new_string);
}

static void test_substring_negative(void)
{
    char *new_string = StringSubstring("abcdef", 6, -3, -1);

    assert_string_equal(new_string, "de");
    free(new_string);
}

static void test_substring_evil(void)
{
    char *new_string = StringSubstring("abcdef", 6, 4, -4);

    assert_int_equal(new_string, NULL);
}

static void test_string_to_long(void)
{
    // Basic usage:
    assert_int_equal(StringToLongExitOnError("0"),    0);
    assert_int_equal(StringToLongExitOnError("-0"),   0);
    assert_int_equal(StringToLongExitOnError("+0"),   0);
    assert_int_equal(StringToLongExitOnError("123"),  123);
    assert_int_equal(StringToLongExitOnError("+123"), 123);

    // WARNING: Some platforms have 32-bit long, 2,147,483,647 is LONG_MAX
    assert_int_equal(StringToLongExitOnError("2147483647"), 2147483647);
    assert_int_equal(StringToLongExitOnError("1987654320"), 1987654320);

    // Negative numbers:
    assert_int_equal((int)StringToLongExitOnError("-1"),       -1);
    assert_int_equal(     StringToLongExitOnError("-1"), (long)-1);
    assert_int_equal((int)StringToLongExitOnError("-1"),       -1);
    assert_int_equal(     StringToLongExitOnError("-1"), (long)-1);

    // Leading spaces:
    assert_int_equal(StringToLongExitOnError(" 0") ,   0);
    assert_int_equal(StringToLongExitOnError(" 123"),  123);
    assert_int_equal(StringToLongExitOnError(" -123"), (long)-123);
    assert_int_equal(StringToLongExitOnError("             0"),   0);
    assert_int_equal(StringToLongExitOnError("             123"), 123);
    assert_int_equal(StringToLongExitOnError("            -123"), (long)-123);

    // Trailing spaces:
    assert_int_equal(StringToLongExitOnError("0 "),    0);
    assert_int_equal(StringToLongExitOnError("789 "),  789);
    assert_int_equal(StringToLongExitOnError("-789 "), (long)-789);
    assert_int_equal(StringToLongExitOnError("0               "), 0);
    assert_int_equal(StringToLongExitOnError("789             "), 789);
    assert_int_equal(StringToLongExitOnError("-789            "), (long)-789);

    // More spaces:
    assert_int_equal(StringToLongExitOnError("   0    "), 0);
    assert_int_equal(StringToLongExitOnError("   -0   "), 0);
    assert_int_equal(StringToLongExitOnError("   456  "), 456);

    // Space separated numbers:
    assert_int_equal(StringToLongExitOnError("   456  9  "), 456);
    assert_int_equal(StringToLongExitOnError("1 0"),         1);
}

static void test_string_to_long_default(void)
{
    assert_int_equal(StringToLongDefaultOnError("0",10), 0);
    assert_int_equal(StringToLongDefaultOnError(" ",10), 10);
    assert_int_equal(StringToLongDefaultOnError("error",123), 123);
    assert_int_equal(StringToLongDefaultOnError("-error",-123), (long)-123);
}

static void test_string_to_long_errors(void)
{
    // A succesful call to StringToLong should return 0:
    long target = 0;
    assert_int_equal(StringToLong("1234", &target), 0);
    assert_int_equal(target, 1234);

    // Test that invalid inputs give error return code:
    assert_int_not_equal(StringToLong("",       &target), 0);
    assert_int_not_equal(StringToLong(" ",      &target), 0);
    assert_int_not_equal(StringToLong("error",  &target), 0);
    assert_int_not_equal(StringToLong("-error", &target), 0);
    assert_int_not_equal(StringToLong("ffff",   &target), 0);
    assert_int_not_equal(StringToLong("1d",     &target), 0);
    assert_int_not_equal(StringToLong("56789d", &target), 0);
    assert_int_equal(StringToLong("9999999999999999999999999999999",
                                  &target),
                     ERANGE);
    assert_int_equal(StringToLong(" 999999999999999999999999999999",
                                  &target),
                     ERANGE);
    assert_int_equal(StringToLong("-999999999999999999999999999999",
                                  &target),
                     ERANGE);

    // Test that error logging function can be called:
    LogStringToLongError("-999999999999999999999999999999", "string_lib_test",
                         ERANGE);

    // Check that target is unmodified after errors:
    assert_int_equal(target, 1234);
}

static void test_string_to_long_unsafe(void)
{
    assert_int_equal(StringToLongUnsafe("0"),  0);
    assert_int_equal(StringToLongUnsafe("1"),  1);
    assert_int_equal(StringToLongUnsafe(" 0"), 0);
    assert_int_equal(StringToLongUnsafe(" 1"), 1);
    assert_int_equal(StringToLongUnsafe("-1"),    (long)-1);
    assert_int_equal(StringToLongUnsafe(" -1"),   (long)-1);
    assert_int_equal(StringToLongUnsafe(" -987"), (long)-987);
    assert_int_equal(StringToLongUnsafe("1987654320"),   1987654320);
    assert_int_equal(StringToLongUnsafe(" 1987654320"),  1987654320);
    assert_int_equal(StringToLongUnsafe(" -1987654320"), (long)-1987654320);

    // Weird edge case:
    assert_int_equal(StringToLongUnsafe(""), 0);
}

// StringToLongExitOnError should replace StringToLongUnsafe:
#define assert_string_to_long_unsafe(x)\
{\
    assert_int_equal(StringToLongExitOnError(x), StringToLongUnsafe(x));\
}

static void test_string_to_long_compatibility(void)
{
    // All these inputs should give same result for new and old function:
    assert_string_to_long_unsafe("0");
    assert_string_to_long_unsafe("-1");
    assert_string_to_long_unsafe("-0");
    assert_string_to_long_unsafe("  -0");
    assert_string_to_long_unsafe("1");
    assert_string_to_long_unsafe("123");
    assert_string_to_long_unsafe("1987654320");
    assert_string_to_long_unsafe("          1987654320");

    // Old function (StringToLongUnsafe) does not allow trailing whitespace
}

#define ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG(str, value_out, expected) \
    assert_int_equal(StringDecimalToLong(str, &value_out), 0); \
    assert_int_equal(value_out, expected)

static void test_string_decimal_to_long(void)
{
    long value_out;

    // Basic usage:
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("0.0", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("-0.0", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("+0.0", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("123.0", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("+123.0", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("-123.0", value_out, (long) -123);

    // Leading spaces:
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG(" 0.0", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG(" 123.0", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG(" -123.0", value_out, (long) -123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("           0.0", value_out, 0);  // 12 is max accepted int length
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("      123.0", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("      -123.0", value_out, (long) -123);

    // Trailing spaces:
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("9.0 ", value_out, 9);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("789.0 ", value_out, 789);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("-789.0 ", value_out, (long) -789);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("9.0           ", value_out, 9);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("789.0         ", value_out, 789);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("-789.0        ", value_out, (long) -789);

    // More spaces:
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("   0.0     ", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("   -0.0    ", value_out, 0);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("   123.0  ", value_out, 123);

    // Space separated numbers:
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("   123.456 78   9  ", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("123.0 45", value_out, 123);

    // Edge case: it doesnt matter what is after the decimal point
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("789.0b", value_out, 789);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("123.blah", value_out, 123);
    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("456.", value_out, 456);
}

static void test_string_decimal_to_long_errors(void)
{
    // Successful call returns 0
    long value_out;

    ASSERT_INT_EQUAL_STRING_DECIMAL_TO_LONG("1234.5678", value_out, 1234);

    assert_int_not_equal(StringDecimalToLong("", &value_out), 0);
    assert_int_not_equal(StringDecimalToLong(" ", &value_out), 0);
    assert_int_not_equal(StringDecimalToLong("error", &value_out), 0);
    assert_int_not_equal(StringDecimalToLong("-error", &value_out), 0);
    assert_int_not_equal(StringDecimalToLong("ffff", &value_out), 0);
    assert_int_equal(StringDecimalToLong("999999999999999999999999999999.9", &value_out),
                     -84);
    assert_int_equal(StringDecimalToLong(" 999999999999999999999999999999.9", &value_out),
                     -84);
    assert_int_equal(StringDecimalToLong("-999999999999999999999999999999.9", &value_out),
                     -84);

    // value_out should be unmodified after errors
    assert_int_equal(value_out, 1234);
}

static void test_string_to_ulong(void)
{
    // Basic usage:
    assert_int_equal(StringToUlongExitOnError("0"),    0);
    assert_int_equal(StringToUlongExitOnError("-0"),   0);
    assert_int_equal(StringToUlongExitOnError("+0"),   0);
    assert_int_equal(StringToUlongExitOnError("123"),  123);
    assert_int_equal(StringToUlongExitOnError("+123"), 123);

    // WARNING: Some platforms have 32-bit unsigned long, 4 294 967 295 is ULONG_MAX
    assert_int_equal(StringToUlongExitOnError("2147483647"), 2147483647);
    assert_int_equal(StringToUlongExitOnError("1987654320"), 1987654320);

    // Leading spaces:
    assert_int_equal(StringToUlongExitOnError(" 0") ,   0);
    assert_int_equal(StringToUlongExitOnError(" 123"),  123);
    assert_int_equal(StringToUlongExitOnError(" -0"), 0);
    assert_int_equal(StringToUlongExitOnError("             0"),   0);
    assert_int_equal(StringToUlongExitOnError("             123"), 123);
    assert_int_equal(StringToUlongExitOnError("            -0"), 0);

    // Trailing spaces:
    assert_int_equal(StringToUlongExitOnError("0 "),    0);
    assert_int_equal(StringToUlongExitOnError("789 "),  789);
    assert_int_equal(StringToUlongExitOnError("-0 "), 0);
    assert_int_equal(StringToUlongExitOnError("0               "), 0);
    assert_int_equal(StringToUlongExitOnError("789             "), 789);
    assert_int_equal(StringToUlongExitOnError("+789            "), 789);

    // More spaces:
    assert_int_equal(StringToUlongExitOnError("   0    "), 0);
    assert_int_equal(StringToUlongExitOnError("   -0   "), 0);
    assert_int_equal(StringToUlongExitOnError("   456  "), 456);

    // Space separated numbers:
    assert_int_equal(StringToUlongExitOnError("   456  9  "), 456);
    assert_int_equal(StringToUlongExitOnError("1 0"),         1);
}

static void test_string_to_ulong_default(void)
{
    assert_int_equal(StringToUlongDefaultOnError("0",10), 0);
    assert_int_equal(StringToUlongDefaultOnError(" ",10), 10);
    assert_int_equal(StringToUlongDefaultOnError("error",123), 123);
    assert_int_equal(StringToUlongDefaultOnError("-123",0), 0);
}

static void test_string_to_ulong_errors(void)
{
    // A succesful call to StringToLong should return 0:
    long target = 0;
    assert_int_equal(StringToUlong("1234", &target), 0);
    assert_int_equal(target, 1234);

    // Test that invalid inputs give error return code:
    assert_int_not_equal(StringToUlong("",       &target), 0);
    assert_int_not_equal(StringToUlong(" ",      &target), 0);
    assert_int_not_equal(StringToUlong("error",  &target), 0);
    assert_int_not_equal(StringToUlong("-error", &target), 0);
    assert_int_not_equal(StringToUlong("ffff",   &target), 0);
    assert_int_not_equal(StringToUlong("1d",     &target), 0);
    assert_int_not_equal(StringToUlong("56789d", &target), 0);
    assert_int_equal(StringToUlong("9999999999999999999999999999999",
                                  &target),
                     ERANGE);
    assert_int_equal(StringToUlong(" 999999999999999999999999999999",
                                  &target),
                     ERANGE);
    assert_int_equal(StringToUlong("-1",
                                  &target),
                     ERANGE);

    // Test that error logging function can be called:
    LogStringToLongError("-999999999999999999999999999999", "string_lib_test",
                         ERANGE);

    // Check that target is unmodified after errors:
    assert_int_equal(target, 1234);
}

static void test_string_to_int64(void)
{
    assert_int_equal(StringToInt64ExitOnError("0"),    0);
    assert_int_equal(StringToInt64ExitOnError("-0"),   0);
    assert_int_equal(StringToInt64ExitOnError("+0"),   0);
    assert_int_equal(StringToInt64ExitOnError("123"),  123);
    assert_int_equal(StringToInt64ExitOnError("+123"), 123);
    assert_int_equal(StringToInt64ExitOnError("9999999999"), 9999999999);
    assert_int_equal(StringToInt64ExitOnError("-9999999999"), -9999999999);
    assert_int_equal(StringToInt64ExitOnError("1234 "), 1234);
    assert_int_equal(StringToInt64ExitOnError("1234\n"), 1234);

    assert_int_equal(StringToInt64DefaultOnError("123", -1), 123);
    assert_int_equal(StringToInt64DefaultOnError("9999999999", 0), 9999999999);
    assert_int_equal(StringToInt64DefaultOnError("-9999999999", 0), -9999999999);
    assert_int_equal(StringToInt64DefaultOnError("", 456), 456);
    assert_int_equal(StringToInt64DefaultOnError(" ", 456), 456);
    assert_int_equal(StringToInt64DefaultOnError("\n", 456), 456);
    assert_int_equal(StringToInt64DefaultOnError("a", 456), 456);
    assert_int_equal(StringToInt64DefaultOnError("abc", 456), 456);
    assert_int_equal(StringToInt64DefaultOnError("k", 456), 456);

    {
        // Paranoid test, ensure value is not cast to smaller type:
        int error_code;
        int64_t out;

        error_code = StringToInt64("9223372036854775807", &out);
        assert_int_equal(error_code, 0);
        assert_true(out == 9223372036854775807);

        error_code = StringToInt64("-9223372036854775808", &out);
        assert_int_equal(error_code, 0);
        // https://stackoverflow.com/a/60323339 :
        assert_true(out == -9223372036854775807 - 1);
    }

    {
        // Error cases:
        // Give bad address - function should not read/write to it

        int error_code;

        error_code = StringToInt64("", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64(" ", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("-", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("\n", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("[]", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("{}", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("\"\"", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("e", NULL + 1);
        assert_true(error_code != 0);

        error_code = StringToInt64("1234abc", NULL + 1);
        assert_true(error_code != 0);
    }
}

static void test_string_from_long(void)
{
    char *result = StringFromLong(123456789);
    assert_string_equal("123456789", result);
    free(result);
    result = StringFromLong(-123456789);
    assert_string_equal("-123456789", result);
    free(result);
}

static void test_string_to_double(void)
{
    assert_true(1234.1234 == StringToDouble("1234.1234"));
}

static void test_string_from_double(void)
{
    char *result = StringFromDouble(1234.1234);
    assert_string_equal("1234.12", result);
    free(result);
}

static void test_safe_compare(void)
{
    // Strings which are equal:
    assert_true(StringSafeCompare(NULL, NULL) == 0);
    assert_true(StringSafeCompare("", "") == 0);
    assert_true(StringSafeCompare("a", "a") == 0);
    assert_true(StringSafeCompare("abc", "abc") == 0);
    assert_true(StringSafeCompare("Hello, world!", "Hello, world!") == 0);

    // Strings which are not equal:
    assert_true(StringSafeCompare("abc", "abC") != 0);
    assert_true(StringSafeCompare("a", "b") != 0);

    // Test ordering of strings (correct sign):
    assert_true(StringSafeCompare(NULL, "a") <= -1);
    assert_true(StringSafeCompare("", "a") <= -1);
    assert_true(StringSafeCompare("a", NULL) >= 1);
    assert_true(StringSafeCompare("a", "") >= 1);
    assert_true(StringSafeCompare("albatross", "bear") <= -1);
    assert_true(StringSafeCompare("lynx", "chicken") >= 1);
}

static void test_safe_equal(void)
{
    assert_true(StringEqual(NULL, NULL));
    assert_true(StringEqual("a", "a"));
    assert_true(StringEqual("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"));
    assert_true(StringEqual("0123456789", "0123456789"));
    assert_true(StringEqual("CamelCase", "CamelCase"));
    assert_true(StringEqual("(){}[]<>", "(){}[]<>"));
    assert_true(StringEqual("+-*/%%^", "+-*/%%^"));

    assert_false(StringEqual("", NULL));
    assert_false(StringEqual(NULL, ""));
    assert_false(StringEqual("a", "b"));
    assert_false(StringEqual("a", "A"));
    assert_false(StringEqual("abc def", "abc deF"));
}

static void test_safe_compare_ignore_case(void)
{
    // Strings which are equal:
    assert_true(StringSafeCompare_IgnoreCase(NULL, NULL) == 0);
    assert_true(StringSafeCompare_IgnoreCase("", "") == 0);
    assert_true(StringSafeCompare_IgnoreCase("a", "a") == 0);
    assert_true(StringSafeCompare_IgnoreCase("abc", "abc") == 0);
    assert_true(StringSafeCompare_IgnoreCase("Hello, world!", "Hello, world!") == 0);

    // Strings with only case differences:
    assert_true(StringSafeCompare_IgnoreCase("abc", "abC") == 0);
    assert_true(StringSafeCompare_IgnoreCase("a", "A") == 0);
    assert_true(StringSafeCompare_IgnoreCase("HELLO, WORLD!", "Hello, world!") == 0);
    assert_true(StringSafeCompare_IgnoreCase("HELLO, WORLD!", "hello, world!") == 0);

    // Test ordering of strings (correct sign):
    assert_true(StringSafeCompare_IgnoreCase(NULL, "a") <= -1);
    assert_true(StringSafeCompare_IgnoreCase("", "a") <= -1);
    assert_true(StringSafeCompare_IgnoreCase("a", NULL) >= 1);
    assert_true(StringSafeCompare_IgnoreCase("a", "") >= 1);
    assert_true(StringSafeCompare_IgnoreCase("albatross", "bear") <= -1);
    assert_true(StringSafeCompare_IgnoreCase("lynx", "chicken") >= 1);

    // Cases where StringSafeCompare and StringSafeCompare_IgnoreCase should be the same:
    assert_int_equal(StringSafeCompare("a", "b"), StringSafeCompare_IgnoreCase("a", "b"));
    assert_int_equal(StringSafeCompare("a", "b"), StringSafeCompare_IgnoreCase("A", "B"));
    assert_int_equal(StringSafeCompare("A", "B"), StringSafeCompare_IgnoreCase("a", "b"));
    assert_int_equal(StringSafeCompare("bbc", "bbd"), StringSafeCompare_IgnoreCase("BBC", "bbd"));
    assert_int_equal(StringSafeCompare("bbc", "bbd"), StringSafeCompare_IgnoreCase("BBC", "BBd"));
}

static void test_safe_equal_ignore_case(void)
{
    assert_true(StringEqual_IgnoreCase(NULL, NULL));
    assert_true(StringEqual_IgnoreCase("a", "a"));
    assert_true(StringEqual_IgnoreCase("a", "A"));
    assert_true(StringEqual_IgnoreCase(hi_alphabet, lo_alphabet));
    assert_true(StringEqual_IgnoreCase("0123456789", "0123456789"));
    assert_true(StringEqual_IgnoreCase("CamelCase", "camelcase"));
    assert_true(StringEqual_IgnoreCase("(){}[]<>", "(){}[]<>"));
    assert_true(StringEqual_IgnoreCase("+-*/%%^", "+-*/%%^"));
    assert_true(StringEqual_IgnoreCase("abc def", "abc deF"));

    assert_false(StringEqual_IgnoreCase("", NULL));
    assert_false(StringEqual_IgnoreCase(NULL, ""));
    assert_false(StringEqual_IgnoreCase("a", "b"));
}

static void test_safe_equal_n(void)
{
    assert_true(StringEqualN("abcd", "abcX", 3));
    assert_true(StringEqualN_IgnoreCase("abcd", "ABCX", 3));

    assert_false(StringEqualN("abcd", "abXX", 3));
    assert_false(StringEqualN_IgnoreCase("abcd", "ABXX", 3));

    assert_true(StringEqualN("123abc", "123abc", 1000));
    assert_true(StringEqualN_IgnoreCase("123abc", "123ABC", 1000));
}

static void test_match(void)
{
#ifdef WITH_PCRE
    assert_true(StringMatch("^a.*$", "abc", NULL, NULL));
    assert_true(StringMatch("a", "a", NULL, NULL));
    assert_true(StringMatch("a", "ab", NULL, NULL));
    assert_false(StringMatch("^a.*$", "bac", NULL, NULL));
#endif // WITH_PCRE
}


static void test_match_full(void)
{
#ifdef WITH_PCRE
    assert_true(StringMatchFull("^a.*$", "abc"));
    assert_true(StringMatchFull("a", "a"));
    assert_false(StringMatchFull("a", "ab"));
    assert_false(StringMatchFull("^a.*$", "bac"));
#endif // WITH_PCRE
}

static void test_encode_base64(void)
{
#ifdef WITH_OPENSSL
    {
        char *res = StringEncodeBase64("", 0);
        assert_string_equal("", res);
        free(res);
    }

    {
        char *res = StringEncodeBase64("a", 1);
        assert_string_equal("YQ==", res);
        free(res);
    }

    {
        char *res = StringEncodeBase64("aa", 2);
        assert_string_equal("YWE=", res);
        free(res);
    }

    {
        char *res = StringEncodeBase64("aaa", 3);
        assert_string_equal("YWFh", res);
        free(res);
    }

    {
        char *res =  StringEncodeBase64("aaaa", 4);
        assert_string_equal("YWFhYQ==", res);
        free(res);
    }

    {
        char *res = StringEncodeBase64("snookie", 7);
        assert_string_equal("c25vb2tpZQ==", res);
        free(res);
    }

    {
        char *res = StringEncodeBase64("test", 4);
        assert_string_equal("dGVzdA==", res);
        free(res);
    }

    // valgrind leaks should be due to crypto one-time allocations
#endif // WITH_OPENSSL
}


static void test_escape_char_copy(void)
{
    char *in1 = "my test with no escape";
    char *out1 = EscapeCharCopy(in1, '7', '\\');
    assert_string_equal(out1, in1);
    free(out1);

    char *in2 = "my test with 'some' escape";
    char *out2 = EscapeCharCopy(in2, '\'', '\\');
    assert_string_equal(out2, "my test with \\'some\\' escape");
    free(out2);

    char *in3 = "my test with 7some7";
    char *out3 = EscapeCharCopy(in3, '7', '\\');
    assert_string_equal(out3, "my test with \\7some\\7");
    free(out3);

    char *in4 = "\"my\" test with 7some7";
    char *out4 = EscapeCharCopy(in4, '\"', '\\');
    assert_string_equal(out4, "\\\"my\\\" test with 7some7");
    free(out4);

    char *in5 = "\"my test with 7some7\"";
    char *out5 = EscapeCharCopy(in5, '\"', '\\');
    assert_string_equal(out5, "\\\"my test with 7some7\\\"");
    free(out5);
}

static void test_chop_no_spaces(void)
{
    char s[] = "abc";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("abc", s);
}

static void test_chop_single_space(void)
{
    char s[] = "abc ";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("abc", s);
}

static void test_chop_two_spaces(void)
{
    char s[] = "abc  ";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("abc", s);
}

static void test_chop_empty(void)
{
    char s[] = "";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("", s);
}

static void test_chop_empty_single_space(void)
{
    char s[] = " ";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("", s);
}

static void test_chop_empty_two_spaces(void)
{
    char s[] = "  ";
    Chop(s, CF_EXPANDSIZE);
    assert_string_equal("", s);
}

static void test_trim_crlf(void)
{
    {
        char *const data = xstrdup("\r\n");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, "");
        assert_int_equal(new_length, 0);
        free(data);
    }
    {
        char *const data = xstrdup("\r\n\r\n");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, "\r\n");
        assert_int_equal(new_length, 2);
        free(data);
    }
    {
        char *const data = xstrdup(",");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, ",");
        assert_int_equal(new_length, 1);
        free(data);
    }
    {
        char *const data = xstrdup(",\r\n");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, ",");
        assert_int_equal(new_length, 1);
        free(data);
    }
    {
        char *const data = xstrdup("a,b,c,d,\" \"\r\n");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, "a,b,c,d,\" \"");
        assert_int_equal(new_length, strlen(data));
        free(data);
    }
    {
        char *const data = xstrdup(" ,\r\n");
        const size_t new_length = TrimCSVLineCRLF(data);
        assert_string_equal(data, " ,");
        assert_int_equal(new_length, strlen(data));
        free(data);
    }
    {
        const char *const original = ",\r\n";
        char *const a = xstrdup(original);
        char *const b = xstrdup(original);
        const size_t a_len = TrimCSVLineCRLF(a);
        const size_t b_len = TrimCSVLineCRLF(b);
        assert_string_equal(a, ",");
        assert_string_equal(b, ",");
        assert_int_equal(a_len, b_len);
        const size_t bb_len = TrimCSVLineCRLF(b);
        assert_int_equal(b_len, bb_len);
        assert_string_equal(b, ",");
        free(a);
        free(b);
    }

    // Test strict version, which has assertions for empty strings and spaces:
    {
        char *const data = xstrdup(",\r\n");
        const size_t new_length = TrimCSVLineCRLFStrict(data);
        assert_string_equal(data, ",");
        assert_int_equal(new_length, 1);
        free(data);
    }
    {
        char *const data = xstrdup(",");
        const size_t new_length = TrimCSVLineCRLFStrict(data);
        assert_string_equal(data, ",");
        assert_int_equal(new_length, 1);
        free(data);
    }
    {
        char *const data = xstrdup("'\r\n',\r\n");
        const size_t new_length = TrimCSVLineCRLFStrict(data);
        assert_string_equal(data, "'\r\n',");
        assert_int_equal(new_length, strlen(data));
        free(data);
    }
    {
        char *const data = xstrdup("a,b,c,d,\" \"\r\n");
        const size_t new_length = TrimCSVLineCRLFStrict(data);
        assert_string_equal(data, "a,b,c,d,\" \"");
        assert_int_equal(new_length, strlen(data));
        free(data);
    }
    {
        char *const data = xstrdup("'\r\n',\r\n");
        const size_t new_length = TrimCSVLineCRLFStrict(data);
        assert_string_equal(data, "'\r\n',");
        assert_int_equal(new_length, strlen(data));
        free(data);
    }
}

static void test_close_hole(void)
{
    char *test_string = xstrdup("test");
    StringCloseHole(test_string, 0, 0);
    assert_string_equal(test_string, "test");
    StringCloseHole(test_string, 4, 4);
    assert_string_equal(test_string, "test");

    size_t length = strlen(test_string);
    StringCloseHole(test_string, length - 1, length);
    assert_string_equal(test_string, "tes");
    StringCloseHole(test_string, 0, 1);
    assert_string_equal(test_string, "es");
    StringCloseHole(test_string, 1, 2);
    assert_string_equal(test_string, "e");
    StringCloseHole(test_string, 0, 1);
    assert_string_equal(test_string, "");
    free(test_string);
}

static void test_ends_with(void)
{
    assert_true(StringEndsWith("file.json", ".json"));
    assert_true(StringEndsWith("file.json", "file.json"));
    assert_false(StringEndsWith(".json", "file"));
    assert_false(StringEndsWith("a", "aa"));
}

char *test_stringvformat_sup(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    char *fmted = StringVFormat(s, ap);
    va_end(ap);
    return fmted;
}

static void test_stringvformat(void)
{
    char *s = test_stringvformat_sup("%s%d", "abc", 42);
    assert_string_equal(s, "abc42");
    free(s);
}

static void test_stringformat(void)
{
    char *s = StringFormat("%d%s%d", 1, "a", 2);
    assert_string_equal(s, "1a2");
    free(s);
}

static void test_string_copy(void)
{
    char a[0 + 1];
    char b[1 + 1];
    char c[2 + 1];
    char non_terminated[3];
    const size_t str_len = 3;
    const size_t buf_size = str_len + 1;
    char d[buf_size];
    char e[buf_size];
    char f[buf_size];
    char g[buf_size];
    char h[buf_size];

    // Non terminated string source:
    non_terminated[0] = '3';
    non_terminated[1] = '4';
    non_terminated[2] = '5';

    // Returns 3 since it was truncated to a string of length 2:
    // (The original was at least 3 bytes long and did not fit).
    assert_int_equal(3, StringCopy(non_terminated, d, 3));
    assert_string_equal(d, "34");

    // Too long string source (returns buf_size, NUL terminates and truncates):
    assert_int_equal(buf_size, StringCopy("longstring", d, buf_size));
    assert_string_equal(d, "lon");

    assert_int_equal(0, StringCopy("", a, 1));
    assert_int_equal(1, StringCopy("A", b, 2));
    assert_int_equal(2, StringCopy("BC", c, 3));
    assert_int_equal(3, StringCopy("DEF", d, buf_size));
    assert_int_equal(4, StringCopy("GHIJ", e, buf_size));
    assert_int_equal(4, StringCopy("KLMNO", f, buf_size));
    assert_int_equal(4, StringCopy("PQRSTU", g, buf_size));
    assert_int_equal(4, StringCopy("VWXYZ 1", h, buf_size));
    assert_string_equal(a, "");
    assert_string_equal(b, "A");
    assert_string_equal(c, "BC");
    assert_string_equal(d, "DEF");
    assert_string_equal(e, "GHI");
    assert_string_equal(f, "KLM");
    assert_string_equal(g, "PQR");
    assert_string_equal(h, "VWX");
    assert_int_equal(1, StringCopy(d, a, 1)); // Truncated, 1 NUL byte written
    assert_int_equal(2, StringCopy(d, b, 2)); // Truncated, 2 bytes written
    assert_int_equal(3, StringCopy(d, c, 3)); // Truncated, 3 bytes written
    // src == dst isn't allowed, have to skip d
    assert_int_equal(3, StringCopy(d, e, buf_size));
    assert_int_equal(3, StringCopy(d, f, buf_size));
    assert_int_equal(3, StringCopy(d, g, buf_size));
    assert_int_equal(3, StringCopy(d, h, buf_size));
    assert_string_equal(a, "");
    assert_string_equal(b, "D");
    assert_string_equal(c, "DE");
    assert_string_equal(d, "DEF");
    assert_string_equal(e, "DEF");
    assert_string_equal(f, "DEF");
    assert_string_equal(g, "DEF");
    assert_string_equal(h, "DEF");

    // Let's also try a longer string:
    int length = strlen(lo_alphabet);
    char buf[1024];
    assert_int_equal(length, StringCopy(lo_alphabet, buf, 1024));
    assert_string_equal(buf, lo_alphabet);

    // Let's check that we haven't corrupted the stack somehow:
    assert_true(non_terminated[0] == '3');
    assert_true(non_terminated[1] == '4');
    assert_true(non_terminated[2] == '5');
    assert_string_equal(d, "DEF");
    assert_int_equal(3, StringCopy(non_terminated, d, 3));
    assert_string_equal(d, "34");

    // Let's check that we don't write out of specified maximum
    char ones[2] = {'1', '1'};
    assert_int_equal(0, StringCopy("", ones, 1));
    assert_true(ones[0] == '\0');
    assert_true(ones[1] == '1');
}

static void test_stringscanfcapped(void)
{
    char buf[20];
    char sp[30];

    strcpy(sp,"");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "");

    strcpy(sp,"\n");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "");

    strcpy(sp,"\n2345678901234567890abcdefghi");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "");

    strcpy(sp,"12345678901234567890abcdefghi");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "1234567890123456789");

    strcpy(sp,"12345678901234567890abcde\nghi");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "1234567890123456789");

    strcpy(sp,"123456789012345\n7890abcdefghi");
    StringNotMatchingSetCapped(sp,20,"\n",buf);
    assert_string_equal(buf, "123456789012345");
}

static void test_PathAppend(void)
{
    char dst[10];
    bool ret;

    {                                                           /* fits */
        dst[0] = '\0';
        ret = PathAppend(dst, sizeof(dst), "blah", '/');
        assert_string_equal(dst, "/blah");
        assert_true(ret);
    }
    {                             /* SAME, but string already has separator */
        strcpy(dst, "/");
        ret = PathAppend(dst, sizeof(dst), "blah", '/');
        assert_string_equal(dst, "/blah");
        assert_true(ret);
    }
    {                                                 /* trailing separator */
        dst[0] = '\0';
        ret = PathAppend(dst, sizeof(dst), "blah/", '/');
        assert_string_equal(dst, "/blah/");
        assert_true(ret);
    }
    {                       /* SAME, but string already has separator ahead */
        strcpy(dst, "/");
        ret = PathAppend(dst, sizeof(dst), "blah/", '/');
        assert_string_equal(dst, "/blah/");
        assert_true(ret);
    }
    {                                                        /* barely fits */
        dst[0] = '\0';
        ret = PathAppend(dst, 6, "blah", '/');
        assert_string_equal(dst, "/blah");
        assert_true(ret);
    }
    {                             /* SAME, but string already has separator */
        strcpy(dst, "/");
        ret = PathAppend(dst, 6, "blah", '/');
        assert_string_equal(dst, "/blah");
        assert_true(ret);
    }
    {                           /* barely not fits (off by one), do nothing */
        dst[0] = '\0';
        ret = PathAppend(dst, 5, "blah", '/');
        assert_string_equal(dst, "");
        assert_false(ret);
    }
    {                             /* SAME, but string already has separator */
        strcpy(dst, "/");
        ret = PathAppend(dst, 5, "blah", '/');
        assert_string_equal(dst, "/");
        assert_false(ret);
    }
    {                                               /* overflow, do nothing */
        dst[0] = '\0';
        ret = PathAppend(dst, 2, "blah", '/');
        assert_string_equal(dst, "");
        assert_false(ret);
    }
    {                             /* SAME, but string already has separator */
        strcpy(dst, "/");
        ret = PathAppend(dst, 2, "blah", '/');
        assert_string_equal(dst, "/");
        assert_false(ret);
    }
}

static void test_StrCat(void)
{
    char dst[10];
    size_t dst_len;

    {
        dst[0] = '\0';
        dst_len = 0;
        StrCat(dst, sizeof(dst), &dst_len, "blah", 0);
        assert_string_equal(dst, "blah");
        assert_int_equal(dst_len, 4);
        StrCat(dst, sizeof(dst), &dst_len, "", 0);
        assert_string_equal(dst, "blah");
        assert_int_equal(dst_len, 4);
        StrCat(dst, sizeof(dst), &dst_len, " ", 0);
        assert_string_equal(dst, "blah ");
        assert_int_equal(dst_len, 5);
        StrCat(dst, sizeof(dst), &dst_len, "blue", 0);
        assert_string_equal(dst, "blah blue");
        assert_int_equal(dst_len, 9);
        /* Append one OVERFLOWing character. */
        StrCat(dst, sizeof(dst), &dst_len, "1", 0);
        /* It should protect against overflow. */
        assert_string_equal(dst, "blah blue");
        /* But the length indicates the needed length. */
        assert_int_equal(dst_len, 10);
    }
    {                        /* The string to append is not '\0'-terminated */
        const char *src = "blah blue";
        dst[0] = '\0';
        dst_len = 0;
        StrCat(dst, sizeof(dst), &dst_len, src, 4);
        assert_string_equal(dst, "blah");
        assert_int_equal(dst_len, 4);
        StrCat(dst, sizeof(dst), &dst_len, src, 4);
        assert_string_equal(dst, "blahblah");
        assert_int_equal(dst_len, 8);
        StrCat(dst, sizeof(dst), &dst_len, src, 2);
        assert_string_equal(dst, "blahblahb");                  /* overflow */
        assert_int_equal(dst_len, 10);
    }
    {
        dst[0] = '\0';
        dst_len = 0;
        StrCat(dst, 4, &dst_len, "blah", 0);
        assert_string_equal(dst, "bla");
        /* Overflow so dst_len indicates the needed length. */
        assert_int_equal(dst_len, 4);
        StrCat(dst, 4, &dst_len, "", 0);
        assert_string_equal(dst, "bla");
        assert_int_equal(dst_len, 4);
        StrCat(dst, 4, &dst_len, "blue", 0);
        assert_string_equal(dst, "bla");
        assert_int_equal(dst_len, 8);
    }
    {                                      /* SAME but pass NULL as dst_len */
        dst[0] = '\0';
        StrCat(dst, 4, NULL, "blah", 0);
        assert_string_equal(dst, "bla");
        StrCat(dst, 4, NULL, "", 0);
        assert_string_equal(dst, "bla");
        StrCat(dst, 4, NULL, "blue", 0);
        assert_string_equal(dst, "bla");
    }

    {                           /* Do not reset dst but reset only dst_len. */
        dst_len = 0;
        StrCat(dst, sizeof(dst), &dst_len, "1", 0);
        assert_string_equal(dst, "1");
        assert_int_equal(dst_len, 1);
    }
}

static void test_StrCatDelim(void)
{
    char dst[10];
    size_t dst_len;

    {                    /* Simple appends, we don't care about truncation. */
        dst[0] = '\0';
        StrCatDelim(dst, sizeof(dst), NULL, "blah", ',');
        StrCatDelim(dst, sizeof(dst), NULL, "blah", ',');
        assert_string_equal(dst, "blah,blah");
        StrCatDelim(dst, sizeof(dst), NULL, "blah", ',');
        assert_string_equal(dst, "blah,blah");
        StrCatDelim(dst, sizeof(dst), NULL, "1", ',');
        assert_string_equal(dst, "blah,blah");
    }
    {                                        /* SAME, but check truncation. */
        dst[0] = '\0';
        dst_len = 0;
        StrCatDelim(dst, sizeof(dst), &dst_len, "blah", ',');
        assert_int_equal(dst_len, 4);
        StrCatDelim(dst, sizeof(dst), &dst_len, "blah", ',');
        assert_string_equal(dst, "blah,blah");
        assert_int_equal(dst_len, 9);
        StrCatDelim(dst, sizeof(dst), &dst_len, "blah", ',');
        assert_string_equal(dst, "blah,blah");
        assert_int_equal(dst_len, 14);                     /* needed length */
        StrCatDelim(dst, sizeof(dst), &dst_len, "1", ',');
        assert_string_equal(dst, "blah,blah");
        assert_int_equal(dst_len, 16);
    }
    {                                               /* Only the comma fits. */
        strcpy(dst, "12345678");
        StrCatDelim(dst, sizeof(dst), NULL, "1", ',');
        assert_string_equal(dst, "12345678");
    }
    {                                        /* SAME, but check truncation. */
        strcpy(dst, "12345678");
        dst_len = 8;
        StrCatDelim(dst, sizeof(dst), &dst_len, "1", ',');
        assert_string_equal(dst, "12345678");
        assert_int_equal(dst_len, 10);           /* 10 is the needed length */
    }
}

static void test_StringMatchesOption(void)
{
    assert_true(StringMatchesOption("-i", "--info", "-i"));
    assert_false(StringMatchesOption("-i", "--info", "-I"));

    assert_true(StringMatchesOption("--info", "--inform", "-I"));
    assert_false(StringMatchesOption("--inform-me", "--info", "-I"));

    assert_true(StringMatchesOption("--client", "--clients", "-C"));
    assert_true(StringMatchesOption("--hub", "--hubs", "-H"));
    assert_true(StringMatchesOption("--host", "--hosts", "-H"));
}

int main()
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
        unit_test(test_get_token),

        unit_test(test_mix_case_tolower),
        unit_test(test_empty_tolower),
        unit_test(test_weird_chars_tolower),
        unit_test(test_alphabet_tolower),
        unit_test(test_hi_alphabet_tolower),
        unit_test(test_inplace_tolower),

        unit_test(test_mix_case_toupper),
        unit_test(test_empty_toupper),
        unit_test(test_weird_chars_toupper),
        unit_test(test_alphabet_toupper),
        unit_test(test_hi_alphabet_toupper),
        unit_test(test_inplace_toupper),

        unit_test(test_replace_empty_pattern),
        unit_test(test_replace_empty_replacement),
        unit_test(test_long_search),
        unit_test(test_replace_eq_size),
        unit_test(test_replace_more_size),
        unit_test(test_replace_less_size),
        unit_test(test_no_replace),

        unit_test(test_string_replace_empty_replacement),
        unit_test(test_string_replace_eq_size),
        unit_test(test_string_replace_buf_too_small),
        unit_test(test_string_replace_smaller),
        unit_test(test_string_replace_none),
        unit_test(test_string_replace_many_percentages),
        unit_test(test_string_replace_n),

        unit_test(test_concatenate),

        unit_test(test_substring_overshoot),
        unit_test(test_substring_positive),
        unit_test(test_substring_negative_length),
        unit_test(test_substring_negative),
        unit_test(test_substring_evil),

        unit_test(test_string_to_long),
        unit_test(test_string_to_long_default),
        unit_test(test_string_to_long_errors),
        unit_test(test_string_to_long_unsafe),
        unit_test(test_string_to_long_compatibility),
        unit_test(test_string_to_ulong),
        unit_test(test_string_to_ulong_default),
        unit_test(test_string_to_ulong_errors),
        unit_test(test_string_to_int64),
        unit_test(test_string_from_long),
        unit_test(test_string_to_double),
        unit_test(test_string_from_double),
        unit_test(test_string_decimal_to_long),
        unit_test(test_string_decimal_to_long_errors),

        unit_test(test_safe_compare),
        unit_test(test_safe_equal),
        unit_test(test_safe_compare_ignore_case),
        unit_test(test_safe_equal_ignore_case),
        unit_test(test_safe_equal_n),

        unit_test(test_match),
        unit_test(test_match_full),

        unit_test(test_encode_base64),

        unit_test(test_escape_char_copy),

        unit_test(test_chop_no_spaces),
        unit_test(test_chop_single_space),
        unit_test(test_chop_two_spaces),
        unit_test(test_chop_empty),
        unit_test(test_chop_empty_single_space),
        unit_test(test_chop_empty_two_spaces),

        unit_test(test_trim_crlf),
        unit_test(test_close_hole),

        unit_test(test_ends_with),

        unit_test(test_stringformat),
        unit_test(test_stringvformat),
        unit_test(test_string_copy),

        unit_test(test_stringscanfcapped),

        unit_test(test_PathAppend),
        unit_test(test_StrCat),
        unit_test(test_StrCatDelim),

        unit_test(test_StringMatchesOption),
    };

    return run_tests(tests);
}
