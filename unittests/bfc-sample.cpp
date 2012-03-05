/*
 * Copyright (C) 2005-2011 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */


#include "bfc-testsuite.hpp"

#include <stdexcept>

#ifdef _MSC_VER
#include <windows.h>
#include <crtdbg.h>
#endif


using namespace bfc;




#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(WIN64)) \
    && defined(_DEBUG) && !defined(UNDER_CE)

_CrtMemState bfc_memdbg_state_snapshot1;
int trigger_memdump = 0;
int trigger_debugger = 0;

/*
 * Define our own reporting function.
 * We'll hook it into the debug reporting
 * process later using _CrtSetReportHook.
 */
static int
bfc_dbg_report_function(int report_type, char *usermsg, int *retval)
{
    /*
     * By setting retVal to zero, we are instructing _CrtDbgReport
     * to continue with normal execution after generating the report.
     * If we wanted _CrtDbgReport to start the debugger, we would set
     * retVal to one.
     */
    *retval = !!trigger_debugger;

    /*
     * When the report type is for an ASSERT,
     * we'll report some information, but we also
     * want _CrtDbgReport to get called -
     * so we'll return TRUE.
     *
     * When the report type is a WARNing or ERROR,
     * we'll take care of all of the reporting. We don't
     * want _CrtDbgReport to get called -
     * so we'll return FALSE.
     */
    switch (report_type)
    {
    default:
    case _CRT_WARN:
    case _CRT_ERROR:
    case _CRT_ERRCNT:
        fwrite(usermsg, 1, strlen(usermsg), stderr);
        fflush(stderr);
        return FALSE;

    case _CRT_ASSERT:
        fwrite(usermsg, 1, strlen(usermsg), stderr);
        fflush(stderr);
        break;
    }
    return TRUE;
}

static void
bfc_report_mem_analysis(void)
{
    _CrtMemState msNow;

    if (!_CrtCheckMemory())
    {
        fprintf(stderr, ">>>Failed to validate memory heap<<<\n");
    }

    /* only dump leaks when there are in fact leaks */
    _CrtMemCheckpoint(&msNow);

    if (msNow.lCounts[_CLIENT_BLOCK] != 0
        || msNow.lCounts[_NORMAL_BLOCK] != 0
        || (_crtDbgFlag & _CRTDBG_CHECK_CRT_DF
            && msNow.lCounts[_CRT_BLOCK] != 0)
    )
    {
        /* difference detected: dump objects since start. */
        _RPT0(_CRT_WARN, "============== Detected memory leaks! ====================\n");

        _CrtMemState diff;
        if (_CrtMemDifference(&diff, &bfc_memdbg_state_snapshot1, &msNow))
        {
            //_CrtMemDumpAllObjectsSince(&bfc_memdbg_state_snapshot1);

            _CrtMemDumpStatistics(&diff);
        }
    }
}


#endif


static int add_argv_elem(int *argc, char ***argv, const char *elem)
{
    if (!argv[0])
    {
        argv[0] = (char **)malloc(sizeof(argv[0]) * 2);
        if (!argv[0]) return -1;
        argc[0] = 1;
    }
    else
    {
        argv[0] = (char **)realloc((void *)argv[0], sizeof(argv[0]) * (argc[0] + 2));
        if (!argv[0]) return -1;
        argc[0]++;
    }
    argv[0][argc[0] - 1] = strdup(elem);
    if (!argv[0][argc[0] - 1]) return -1;
    argv[0][argc[0]] = NULL;
    return 0;
}


/*
 * process '@filename' commandline arguments (response files)
 */
static int handle_response_files(int *argc, char ***argv)
{
    int new_argc = 0;
    char **new_argv = NULL;
    int i;
    FILE *f;
    char str[1024];
    char *elem;
    char *endp;

    for (i = 0; i < argc[0]; i++)
    {
        if (i == 0)
        {
            if (add_argv_elem(&new_argc, &new_argv, argv[0][0]))
                return -1;
            continue;
        }
        if (argv[0][i][0] == '@')
        {
            f = fopen(argv[0][i]+1, "r");
            if (!f)
            {
                fprintf(stderr, "Cannot open responsefile %s\n", argv[0][i]+1);
                return -1;
            }
            while (!feof(f))
            {
                str[0] = 0;
                fgets(str, sizeof(str), f);
                for (elem = str; *elem; )
                {
                    /* skip leading whitespace */
                    while (*elem && (isspace(*elem) || strchr(" \t\r\n", *elem)))
                        elem++;
                    switch (*elem)
                    {
                    case 0:
                        break;

                    case '\'':
                    case '\"':
                        /* quoted argument... */
                        {
                            char c = *elem++;

                            endp = elem;
                            while (*endp && *endp != c)
                                endp++;
                            if (*endp != c)
                            {
                                fprintf(stderr, "unterminated string in response file\n");
                                return -1;
                            }
                            *endp++ = 0;
                            if (add_argv_elem(&new_argc, &new_argv, elem))
                                return -1;
                            elem = endp;
                        }
                        continue;

                    default:
                        endp = elem;
                        while (*endp && !isspace(*endp) && !strchr(" \t\r\n", *endp))
                            endp++;
                        if (*endp)
                            *endp++ = 0;
                        if (add_argv_elem(&new_argc, &new_argv, elem))
                            return -1;
                        elem = endp;
                        continue;
                    }
                }
            }
            fclose(f);
        }
        else
        {
            /* regular argument... */
            if (add_argv_elem(&new_argc, &new_argv, argv[0][i]))
                return -1;
        }
    }

    *argc = new_argc;
    *argv = new_argv;

    for (i = 0; i < argc[0]; i++)
    {
        printf("argv[%d] = \"%s\"\n", i, argv[0][i]);
    }
    return 0;
}

#if 0
static void shift_argv(int *new_argc, char ***new_argv, int shift)
{
    int c = *new_argc;
    int i;

    for (i = 1; i <= shift; i++)
    {
        free(new_argv[0][i]);
        new_argv[0][i] = NULL;
    }

    if (c <= shift + 1)
    {
        /* may be too large shift? or at least it's maximum shift: clip all args */
        *new_argc = (c ? 1 : 0);
    }
    else
    {
        memmove(&new_argv[0][1], &new_argv[0][1 + shift], (c - shift - 1) * sizeof(new_argv[0][0]));
        *new_argc -= shift;
    }
}
#endif

static void free_argv(int new_argc, char **new_argv)
{
    int i;

    for (i = 0; i < new_argc; i++)
    {
        if (new_argv[i])
        {
            free(new_argv[i]);
        }
    }
    free(new_argv);
}






#if 0
template <> class bfc_visitor<fixture> /* bfc_fixture_visitor */
{
};

template <> class bfc_visitor<test> /* bfc_test_visitor */
{
};
#endif

class hamster_fixture_visitor : public bfc_visitor<fixture>
{
protected:
    bfc_run_state_t *m_run_or_list;

public:
    virtual bfc_run_state_t operator()(fixture &t_ref, const std::string &name)
    {
        fprintf(stderr, "fixture name: %s\n", name.c_str());

        return (m_run_or_list ? *m_run_or_list : CONTINUE_RUN);
    }

    virtual operator bool()
    {
        return true;
    }

public:
    hamster_fixture_visitor(bfc_run_state_t *run_or_list = NULL) : m_run_or_list(run_or_list)
    {
    }
};
class hamster_test_visitor : public bfc_visitor<test>
{
protected:
    bfc_run_state_t *m_run_or_list;

public:
    virtual bfc_run_state_t operator()(test &t_ref, const std::string &name)
    {
        fprintf(stderr, "fixture name: %s\n", name.c_str());

        return (m_run_or_list ? *m_run_or_list : CONTINUE_RUN);
    }

    virtual operator bool()
    {
        return true;
    }

public:
    hamster_test_visitor(bfc_run_state_t *run_or_list = NULL) : m_run_or_list(run_or_list)
    {
    }
};





int
main(int argc, char **argv)
{
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(WIN64)) \
        && defined(_DEBUG) && (HAM_LEAN_AND_MEAN_FOR_PROFILING_LEVEL < 2)
    /*
     * Hook in our client-defined reporting function.
     * Every time a _CrtDbgReport is called to generate
     * a debug report, our function will get called first.
     */
    _CrtSetReportHook(bfc_dbg_report_function);

    /*
     * Define the report destination(s) for each type of report
     * we are going to generate.  In this case, we are going to
     * generate a report for every report type: _CRT_WARN,
     * _CRT_ERROR, and _CRT_ASSERT.
     * The destination(s) is defined by specifying the report mode(s)
     * and report file for each report type.
     */
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    // Store a memory checkpoint in the s1 memory-state structure
    _CrtMemCheckpoint(&bfc_memdbg_state_snapshot1);

    atexit(bfc_report_mem_analysis);

    // Get the current bits
    int i = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    i &= 0x0000FFFF
        & ~(_CRTDBG_ALLOC_MEM_DF
        | _CRTDBG_DELAY_FREE_MEM_DF
        | _CRTDBG_LEAK_CHECK_DF
        | _CRTDBG_CHECK_ALWAYS_DF);

    i |= _CRTDBG_ALLOC_MEM_DF;

    // Set the debug-heap flag so that freed blocks are kept on the
    // linked list, to catch any inadvertent use of freed memory
#if 0
    i |= _CRTDBG_DELAY_FREE_MEM_DF;
#endif

    // Set the debug-heap flag so that memory leaks are reported when
    // the process terminates. Then, exit.
    i |= _CRTDBG_LEAK_CHECK_DF;

    // Clear the upper 16 bits and OR in the desired freqency
#if 0
    i = (i & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_1024_DF;
#else
    i |= _CRTDBG_CHECK_ALWAYS_DF;
#endif

    // Set the new bits
    _CrtSetDbgFlag(i);

    // set a malloc marker we can use it in the leak dump at the end of
    // the program:
//    (void)_calloc_dbg(1, 1, _CLIENT_BLOCK, __FILE__, __LINE__);
#endif

    int new_argc = argc;
    char **new_argv = argv;

    if (handle_response_files(&new_argc, &new_argv))
    {
        fprintf(stderr, "error preparing arguments.\n");
        return EXIT_FAILURE;
    }



    /*
     * when running in visual studio, the working directory is different
     * from the unix/cygwin environment. this can be changed, but the
     * working directory setting is not stored in the unittests.vcproj file,
     * but in unittests.vcproj.<hostname><username>; and this file is not
     * distributed.
     *
     * therefore, at runtime, if we're compiling under visual studio, set
     * the working directory manually.
     */
#ifdef _MSC_VER
#   ifndef UNDER_CE
#if 0 // [i_a] given my own build env, this directory changes as well
    SetCurrentDirectoryA("../unittests"); /* [i_a] */
#elif defined(UNITTEST_PATH)
    SetCurrentDirectoryA(UNITTEST_PATH);
#elif !defined(UNDER_CE)
    // .\win32\msvc2008\bin\Win32_MSVC2008.Debug -> .\unittests
    SetCurrentDirectoryA("../../../../unittests"); /* [i_a] */
#endif
#   endif
#endif

    // set up the testrunner rig:
#if 0 // turn this on (--> #if 01) to assist with debugging testcases:
    // exceptions, etc. will pass through to your debugger
    testrunner::get_instance()->catch_coredumps(0);
    testrunner::get_instance()->catch_exceptions(0);
#else
    testrunner::get_instance()->catch_coredumps(0);
    testrunner::get_instance()->catch_exceptions(1);
#endif
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(WIN64))
#   if defined(UNDER_CE)
    testrunner::get_instance()->outputdir("M:/");
    testrunner::get_instance()->inputdir("./");
#   else
    char *temppath = getenv("TEMP");
    if (!temppath)
        temppath = "C:/";
    testrunner::get_instance()->outputdir(temppath);
    testrunner::get_instance()->inputdir("./");
#   endif
#endif

    // as we wish to print all collected errors at the very end, we act
    // as if we don't want the default built-in reporting, hence we MUST
    // call init_run() here.
    testrunner::get_instance()->init_run();
    unsigned int r;
    bfc_run_state_t run_mode = CONTINUE_RUN;
    hamster_fixture_visitor fv(&run_mode);
    hamster_test_visitor tv(&run_mode);

    if (new_argc > 1)
    {
        std::string lead_fixture;
        std::string lead_test;
        bool lead = false;
        bool inclusive_begin = true;

        r = 0;
        for (int i = 1; i <= new_argc; i++)
        {
            std::string fixture_name;
            std::string option_name;
            if (i < new_argc)
            {
                option_name = new_argv[i];
                if (option_name[0] == '-')
                {
                    if (option_name == "-h"
                        || option_name == "-?"
                        || option_name == "--help")
                    {
                        fprintf(stderr,
"unittests [options]\n"
"\n"
"Executes the suite of unittests. The default is to execute all\n"
"tests, but you may specify the names of fixtures (test groups) and test\n"
"names to run a subset. Subsets can be specified in a format which allows\n"
"picking and executing both individual tests and fixtures and from-to series\n"
"of these.\n"
"\n"
"The order in which you specify the fixtures/tests to pick is also the order\n"
"in which the unittests application will execute these tests.\n"
"\n"
"You may also specify the '--list' commandline option to view the list of\n"
"available fixtures and the tests contained therein. When you specify '--list'\n"
"in combination with fixture and/or test arguments, then '--list' will produce\n"
"the list of tests that you've selected thusly, i.e. '--list' acts like a\n"
"'--no-run' option.\n"
"\n"
"You specify a fixture by adding one or two optional colons as a postfix, e.g.\n"
"any one of\n"
"\n"
"  LogTest::\n"
"  LogTest:\n"
"  LogTest\n"
"\n"
"will suffice to execute all tests in the 'LogTest' fixture.\n"
"\n"
"You specify a test by prefixing it with one or two mandatory colons. When you\n"
"do not specify the fixture name as well, than any test with that name in any\n"
"fixture will be executed.\n"
"Instead of no fixture name, you may also specify the '*' wildcard. Similarly,\n"
"you may specify the '*' wildcard for the test name when specifying a fixture.\n"
"\n"
"To specify a series of tests, you may add a lone '*' wildcard between the\n"
"'from' and 'to' (both are included in the test run) specification.\n"
"Having a '*' as the very first argument implies the series is to start with\n"
"the first available test (see the --list output) and end at the test which is\n"
"specified next. Similarly, a '*' as the very last argument is interpreted as\n"
"a series which starts at the previously specified test and will continue\n"
"until the very last test in the --list set.\n"
"\n"
"Examples:\n"
"\n"
"    :XyzTest\n"
"~ execute all the tests named 'XyzTest'.\n"
" \n"
"    FixtureA::XyzTest\n"
"~ execute the 'XyzTest' unittest which is part of the 'FixtureA' fixture.\n"
);
                        return r;
                    }
                    if (option_name == "--list")
                    {
                        run_mode = SKIP_THIS_TEST;
                        continue;
                    }
                }
                fixture_name = new_argv[i];
            }

            if (fixture_name == "*")
            {
                // lead or tail or chain?
                lead = true;
                fixture_name.clear();
            }
            else
            {
                size_t pos = fixture_name.find(':');
                std::string test_name;
                if (pos != std::string::npos)
                {
                    test_name = fixture_name.substr(pos + 1);
                    fixture_name = fixture_name.substr(0, pos);
                    while ((pos = test_name.find(':')) != std::string::npos)
                    {
                        test_name.erase(pos, 1);
                    }
                    if (test_name == "*")
                    {
                        test_name.clear();
                    }
                    if (fixture_name == "*")
                    {
                        fixture_name.clear();
                    }
                }

                if (!lead && (i < new_argc)
                    && (i+1 >= new_argc || std::string(new_argv[i+1]) != "*"))
                {
                    // single case:
                    r = testrunner::get_instance()->run(
                            fv, tv,
                            fixture_name.c_str(), test_name.c_str(),
                            false);
                    inclusive_begin = true;
                }
                else if (lead)
                {
                    r = testrunner::get_instance()->run(
                            fv, tv,
                            lead_fixture, lead_test,
                            fixture_name, test_name,
                            inclusive_begin,
                            false,
                            false);
                    inclusive_begin = false;
                }
                lead_fixture = fixture_name;
                lead_test = test_name;
                lead = false;
            }
        }
    }
    else
    {
        r = testrunner::get_instance()->run(fv, tv, false);
    }
    testrunner::get_instance()->print_errors();
    testrunner::delete_instance();

    free_argv(new_argc, new_argv);

    return (r);
}

#if UNDER_CE
int
_tmain(int argc, _TCHAR* argv[])
{
    return (main(0, 0));
}
#endif
