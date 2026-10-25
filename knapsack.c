/*
 * 0/1 Knapsack (dynamic programming) - C99, VPL friendly.
 *
 * Input  : test cases separated by lines starting with "#Test".
 *          Each case:  "n W" line, then n lines of "weight value".
 *          Read from a file given as argv[1], otherwise from stdin; if stdin
 *          is empty, falls back to Knapsack_data.txt / input.txt.
 * Output : one line per case on stdout AND in result.txt:
 *              #Test <k> (<n> <W>) <max value>
 * Option : -v  prints the DP table row by row (off by default so that the
 *              output stays exactly the required format for VPL grading).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- read every integer of the input, skipping '#' lines ---------- */
static long *read_numbers(FILE *fp, size_t *count)
{
    size_t cap = 256, n = 0;
    long *a = malloc(cap * sizeof *a);
    char *line = NULL;
    size_t lcap = 0, len = 0;
    int ch;

    if (!a) return NULL;
    line = malloc(lcap = 256);
    if (!line) { free(a); return NULL; }

    for (;;) {
        len = 0;
        while ((ch = fgetc(fp)) != EOF && ch != '\n') {   /* '\r' removed below */
            if (len + 1 >= lcap) {
                char *t = realloc(line, lcap *= 2);
                if (!t) { free(line); free(a); return NULL; }
                line = t;
            }
            line[len++] = (char)ch;
        }
        line[len] = '\0';
        if (len == 0 && ch == EOF) break;

        char *p = line;
        while (isspace((unsigned char)*p)) p++;          /* skip blanks, \r */
        if (*p == '#') { if (ch == EOF) break; continue; } /* "#Test" header */

        for (;;) {
            char *end;
            long v = strtol(p, &end, 10);
            if (end == p) break;                          /* no more numbers */
            if (n == cap) {
                long *t = realloc(a, (cap *= 2) * sizeof *a);
                if (!t) { free(line); free(a); return NULL; }
                a = t;
            }
            a[n++] = v;
            p = end;
        }
        if (ch == EOF) break;
    }
    free(line);
    *count = n;
    return a;
}

/* ---------- DP: returns best value, optionally prints the table --------- */
static long knapsack(int n, int W, const long *wt, const long *val, int verbose)
{
    size_t cols = (size_t)W + 1;
    long *dp = calloc((size_t)(n + 1) * cols, sizeof *dp);
    long best;
    if (!dp) return -1;

    for (int i = 1; i <= n; i++)
        for (int w = 0; w <= W; w++) {
            long skip = dp[(size_t)(i - 1) * cols + w];
            long take = skip;
            if (wt[i - 1] <= w) {
                long t = dp[(size_t)(i - 1) * cols + (w - wt[i - 1])] + val[i - 1];
                if (t > take) take = t;
            }
            dp[(size_t)i * cols + w] = take;
        }
    best = dp[(size_t)n * cols + W];

    if (verbose) {
        printf("    w:");
        for (int w = 0; w <= W; w++) printf("%5d", w);
        printf("\n");
        for (int i = 0; i <= n; i++) {
            printf("i=%2d  :", i);
            for (int w = 0; w <= W; w++) printf("%5ld", dp[(size_t)i * cols + w]);
            printf("\n");
        }
    }
    free(dp);
    return best;
}

int main(int argc, char **argv)
{
    int verbose = 0;
    const char *path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) verbose = 1;
        else path = argv[i];
    }

    size_t cnt = 0;
    long *nums = NULL;
    if (path) {
        FILE *f = fopen(path, "r");
        if (!f) { fprintf(stderr, "Cannot open %s\n", path); return 1; }
        nums = read_numbers(f, &cnt);
        fclose(f);
    } else {
        nums = read_numbers(stdin, &cnt);
        if (nums && cnt == 0) {                         /* nothing on stdin */
            const char *fb[] = {"Knapsack_data.txt", "input.txt"};
            for (int k = 0; k < 2 && cnt == 0; k++) {
                FILE *f = fopen(fb[k], "r");
                if (f) { free(nums); nums = read_numbers(f, &cnt); fclose(f); }
            }
        }
    }
    if (!nums) { fprintf(stderr, "Out of memory\n"); return 1; }

    FILE *out = fopen("result.txt", "w");
    size_t pos = 0;
    int test = 0;

    while (pos + 1 < cnt) {
        long n = nums[pos], W = nums[pos + 1];
        pos += 2;
        if (n < 0 || W < 0 || pos + 2 * (size_t)n > cnt) {
            fprintf(stderr, "Malformed test case %d\n", test + 1);
            break;
        }
        long *wt = malloc((n ? n : 1) * sizeof *wt);
        long *val = malloc((n ? n : 1) * sizeof *val);
        if (!wt || !val) { free(wt); free(val); break; }
        for (long i = 0; i < n; i++) {
            wt[i]  = nums[pos++];
            val[i] = nums[pos++];
        }
        test++;
        long best = knapsack((int)n, (int)W, wt, val, verbose);
        printf("#Test %d (%ld %ld) %ld\n", test, n, W, best);
        if (out) fprintf(out, "#Test %d (%ld %ld) %ld\n", test, n, W, best);
        free(wt);
        free(val);
    }
    if (out) fclose(out);
    free(nums);
    return 0;
}
