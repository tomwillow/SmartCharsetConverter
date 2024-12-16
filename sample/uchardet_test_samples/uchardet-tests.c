/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Universal charset detector code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Jehan <jehan@girinstud.io>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/uchardet.h"

#define BUFFER_SIZE 65536

#if defined(_WIN32) || defined(__CYGWIN__)
#define realpath(filename,unused) _fullpath(NULL, filename, 0)
#define SEP '\\'
#else
#define SEP '/'
#endif

void
detect(FILE *fp,
       char *expected_charset, char *expected_lang,
       float *expected_confidence, size_t *expected_candidate,
       char **charset, char **lang, float *confidence)
{
    uchardet_t  handle = uchardet_new();
    char        buffer[BUFFER_SIZE];
    int         i;

    while (1)
    {
        size_t len = fread(buffer, 1, BUFFER_SIZE, fp);
        if (len == 0)
            break;
        int retval = uchardet_handle_data(handle, buffer, len);
        if (retval != 0)
        {
            fprintf(stderr,
                    "uchardet-tests: handle data error.\n");
            exit(1);
        }
    }
    uchardet_data_end(handle);

    *charset = strdup(uchardet_get_encoding(handle, 0));
    if (uchardet_get_language(handle, 0))
      *lang = strdup(uchardet_get_language(handle, 0));
    else
      *lang = NULL;
    *confidence = uchardet_get_confidence(handle, 0);
    for (i = 0; (*charset)[i]; i++)
    {
        /* Our test files are lowercase. */
        (*charset)[i] = tolower((*charset)[i]);
    }

    if (strcmp(expected_charset, *charset) != 0 ||
        *lang == NULL                           ||
        strcmp(expected_lang, *lang) != 0)
    {
        size_t n_candidates = uchardet_get_n_candidates(handle);

        *expected_confidence = 0.0f;
        *expected_candidate = SIZE_MAX;
        for (i = 0; i < n_candidates; i++)
        {
            if (uchardet_get_language(handle, i) == NULL)
                /* Skip this case for now. */
                continue;

            if (strcasecmp(expected_charset, uchardet_get_encoding(handle, i)) == 0 &&
                strcasecmp(expected_lang, uchardet_get_language(handle, i)) == 0)
            {
                *expected_confidence = uchardet_get_confidence(handle, i);
                *expected_candidate = i;
                break;
            }
        }
    }
    else
    {
        *expected_confidence = *confidence;
        *expected_candidate = 0;
    }

    uchardet_delete(handle);
}

int
main(int argc, char ** argv)
{
    FILE *f;
    char *filename;
    char *path;
    char *expected_charset;
    char *expected_lang = NULL;
    char *charset;
    char *lang;
    float expected_confidence = 0.0f;
    float confidence = 0.0f;
    size_t expected_candidate = 0;
    /* In a unit test, 0 means success, other returned values mean failure. */
    int   success = 1;

    if (argc != 2)
    {
        /* The test program expects exactly 1 argument. */
        fprintf(stderr,
                "uchardet-tests expects exactly 1 argument\n");
        return 1;
    }

    filename = strdup(argv[1]);
    f = fopen(filename, "r");
    if (f == NULL)
    {
        /* Error opening the test file. */
        fprintf(stderr,
                "uchardet-tests: error opening the test file \"%s\"\n",
                filename);
        free(filename);
        return 1;
    }

    path = realpath(filename, NULL);
    assert(path);
    expected_charset = strrchr(path, SEP);
    assert(expected_charset);
    *expected_charset = '\0';
    expected_charset++;
    expected_charset = strtok(expected_charset, ".");

    expected_lang = strrchr(path, SEP);
    assert(expected_lang);
    expected_lang++;

    detect(f, expected_charset, expected_lang, &expected_confidence, &expected_candidate, &charset, &lang, &confidence);
    fclose (f);

    /* No lang detection is a failure, except for a few charset for
     * which we still don't detect languages.
     * TODO.
     * */
    if (strcmp(expected_charset, "ascii") == 0 ||
        strcmp(expected_charset, "utf-16") == 0 ||
        strcmp(expected_charset, "utf-16") == 0 ||
        strcmp(expected_charset, "utf-32") == 0)
    {
        success = (strcmp(charset, expected_charset) != 0);
    }
    else if (lang)
    {
        success = (strcmp(charset, expected_charset) != 0) +
                  (strcmp(lang, expected_lang) != 0);
    }

    if (success != 0)
    {
        char *candidate_str = NULL;

        if (expected_confidence != 0.0f)
        {
            size_t candidate_len;

            candidate_len = snprintf(candidate_str, 0, "%d", expected_candidate + 1);
            candidate_str = malloc(candidate_len + 1);
            snprintf(candidate_str, candidate_len + 1, "%d", expected_candidate + 1);
        }

        fprintf(stderr,
                "uchardet-tests FAILED: found %s/%s (confidence: %f) - expecting %s/%s (confidence: %f - candidate: %s)\n",
                lang ? lang : "n/a", charset, confidence,
                expected_lang, expected_charset, expected_confidence,
                candidate_str == NULL ? "n/a" : candidate_str);

        if (candidate_str != NULL)
            free(candidate_str);
    }

    free(path);
    free(charset);
    free(lang);
    free(filename);

    return success;
}
