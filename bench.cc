/*
 * Copyright (c) 2013, 2017 Alexey Tourbin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* Number of comparisons that a sort function makes. */
size_t ncmp;
#if 1
#define NCMPINC ncmp++
#else
#define NCMPINC (void*)0
#endif

/*
 * Numeric comparison
 */
static int icmp(const void *i1, const void *i2)
{
    NCMPINC;
    return *(int *) i1 - *(int *) i2;
}
void stdlib_isort(int A[], size_t n)
{
    qsort(A, n, sizeof(int), icmp);
}

#ifdef __cplusplus
#include <algorithm>
static inline bool iless(int a, int b)
{
    NCMPINC;
    return a < b;
}
void stl_isort(int A[], size_t n)
{
    std::sort(A, A + n, iless);
}
#endif

#include "mjt.h"
void mjt_isort(int A[], size_t n)
{
#define MJT_ILESS(a, b) (NCMPINC, *(a) < *(b))
    QSORT(int, A, n, MJT_ILESS);
#undef QSORT
}

#include "qsort.h"
void svpv_isort(int A[], size_t n)
{
    int tmp;
#define ILESS(i, j) (NCMPINC, A[i] < A[j])
#define SWAP(i, j) tmp = A[i], A[i] = A[j], A[j] = tmp
    QSORT(n, ILESS, SWAP);
#undef QSORT
}

/*
 * String comparison
 */
#include <string.h>
static int pstrcmp(const void *a, const void *b)
{
    NCMPINC;
    return strcmp(*(const char **)a, *(const char **)b);
}
void stdlib_strsort(const char *A[], size_t n)
{
    qsort(A, n, sizeof *A, pstrcmp);
}

#ifdef __cplusplus
static inline bool strless(const char *a, const char *b)
{
    NCMPINC;
    return strcmp(a, b) < 0;
}
void stl_strsort(const char *A[], size_t n)
{
    std::sort(A, A + n, strless);
}
#endif

#include "mjt.h"
void mjt_strsort(const char *A[], size_t n)
{
#define MJT_STRLESS(a, b) (NCMPINC, strcmp(*(a), *(b)) < 0)
    QSORT(const char *, A, n, MJT_STRLESS);
#undef QSORT
}

#undef QSORT_H
#include "qsort.h"
void svpv_strsort(const char *A[], size_t n)
{
    const char *tmp;
#define STRLESS(i, j) (NCMPINC, strcmp(A[i], A[j]) < 0)
    QSORT(n, STRLESS, SWAP);
}

/*
 * Benchmarking
 */
#include <inttypes.h>
#include <time.h>

#define N (1 << 20)
static int orig[N];
static int copy[N];

#include <string.h>
#include <unistd.h>

double bench_int(size_t n, void (*sort)(int A[], size_t n))
{
    // Make 4 runs, throw away min and max, average the other two.
    clock_t min = UINT64_MAX, max = 0, sum = 0;
    for (int i = 0; i < 4; i++) {
	usleep(1);
	memcpy(copy, orig, sizeof orig);
	sort(copy, n);
	memcpy(copy, orig, sizeof orig);
	ncmp = 0;
	clock_t t = clock();
	sort(copy, n);
	t = clock() - t;
	sum += t;
	if (t < min)
	    min = t;
	else if (t > max)
	    max = t;
    }
    // See if it can actually sort.
    for (size_t i = 1; i < n; i++)
	assert(copy[i-1] <= copy[i]);
    sum -= min + max;
    return (double) (sum) / 2 * CLOCKS_PER_SEC;
}

#define N_STR (1 << 20)
static const char *orig_str[N];
static const char *copy_str[N];

double bench_str(size_t n, void (*strsort)(const char *A[], size_t n))
{
    clock_t min = UINT64_MAX, max = 0, sum = 0;
    for (int i = 0; i < 4; i++) {
	usleep(1);
	memcpy(copy_str, orig_str, sizeof orig_str);
	strsort(copy_str, n);
	memcpy(copy_str, orig_str, sizeof orig_str);
	ncmp = 0;
	clock_t t = clock();
	strsort(copy_str, n);
	t = clock() - t;
	sum += t;
	if (t < min)
	    min = t;
	else if (t > max)
	    max = t;
    }
    for (size_t i = 1; i < n; i++)
	assert(strcmp(copy_str[i-1], copy_str[i]) <= 0);
    sum -= min + max;
    return (double) (sum) / 2 * CLOCKS_PER_SEC;
}

#include <getopt.h>
static int opt_srand;
static int opt_strcmp;
static struct option longopts[] = {
    { "srand", no_argument, &opt_srand, 1 },
    { "strcmp", no_argument, &opt_strcmp, 1 },
    { NULL },
};

int main(int argc, char **argv)
{
    const char *argv0 = argv[0];
    int usage = 0;
    int c;
    while ((c = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
	switch (c) {
	case 0:
	    break;
	default:
	    usage = 1;
	}
    }
    argc -= optind, argv += optind;
    if (argc && !usage) {
	fprintf(stderr, "%s: too many arguments\n", argv0);
	usage = 1;
    }
    if (usage) {
	fprintf(stderr, "Usage: %s [options]\n", argv0);
	return 1;
    }
    if (opt_srand)
	srand(getpid());
    if (opt_strcmp) {
	if (isatty(0))
	    fprintf(stderr, "reading input from stdin\n");
	size_t n = 0;
	while (1) {
	    char *line = NULL;
	    size_t alloc = 0;
	    ssize_t len = getline(&line, &alloc, stdin);
	    if (len < 0)
		break;
	    orig_str[n++] = line;
	    if (n == N_STR)
		break;
	}
	printf("stdlib\t %lf\t", bench_str(n, stdlib_strsort));
	printf("%zu\n", ncmp);
#ifdef __cplusplus
	printf("stl\t% %lf\t", bench_str(n, stl_strsort));
	printf("%zu\n", ncmp);
#endif
	printf("mjt\t %lf\t", bench_str(n, mjt_strsort));
	printf("%zu\n", ncmp);
	printf("svpv\t %lf\t", bench_str(n, svpv_strsort));
	printf("%zu\n", ncmp);
    }
    else {
	size_t n = N;
	for (size_t i = 0; i < N; i++)
	    orig[i] = rand();
	printf("stdlib\t %lf\t", bench_int(n, stdlib_isort));
	printf("%zu\n", ncmp);
#ifdef __cplusplus
	printf("stl\t %lf\t", bench_int(n, stl_isort));
	printf("%zu\n", ncmp);
#endif
	printf("mjt\t %lf\t", bench_int(n, mjt_isort));
	printf("%zu\n", ncmp);
	printf("svpv\t %lf\t", bench_int(n, svpv_isort));
	printf("%zu\n", ncmp);
    }
    return 0;
}

// ex:set ts=8 sts=4 sw=4 noet:
