/* tests/test_solutions.c
 *
 * Copyright (c) 2010 Michael Forney
 */

int successes = 0;
int failures = 0;

void process_result(bool result, const char * test_name)
{
    if (result)
    {
        ++successes;
        printf("%s succeeded\n", test_name);
    }
    else
    {
        ++failures;
        printf("%s failed\n", test_name);
    }
}

#define EXIT_STATUS (failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE)

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax

