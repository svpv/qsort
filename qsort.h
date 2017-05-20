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

/* Sort 3 elements. */
#define Q_SORT3(a1, a2, a3)		\
do {					\
    if (Q_LESS(a2, a1)) {		\
	if (Q_LESS(a3, a2))		\
	    Q_SWAP(a1, a3);		\
	else {				\
	    Q_SWAP(a1, a2);		\
	    if (Q_LESS(a3, a2))		\
		Q_SWAP(a2, a3);		\
	}				\
    }					\
    else if (Q_LESS(a3, a2)) {		\
	Q_SWAP(a2, a3);			\
	if (Q_LESS(a2, a1))		\
	    Q_SWAP(a1, a2);		\
    }					\
} while (0)

/* Partition [q_l,q_r] around a pivot.  After partitioning,
 * [q_l,q_j] are the elements that are less than or equal to the pivot,
 * while [q_i,q_r] are the elements greater than or equal to the pivot. */
#define Q_PARTITION(q_l, q_r, q_i, q_j)					\
do {									\
    /* The middle element, not to be confused with the median. */	\
    Q_UINT q_m = q_l + ((q_r - q_l) >> 1);				\
    /* Reorder the second, the middle, and the last items.		\
     * As [Edelkamp Weiss 2016] explain, using the second element	\
     * instead of the first one helps avoid bad behaviour for		\
     * decreasingly sorted arrays.  This method is used in recent	\
     * versions of gcc's std::sort, see gcc bug 58437#c13, although	\
     * the details are somewhat different (cf. #c14). */		\
    Q_SORT3(q_l + 1, q_m, q_r);						\
    /* Place the median at the beginning. */				\
    Q_SWAP(q_l, q_m);							\
    /* Partition [q_l+2, q_r-1] around the median which is in q_l.	\
     * q_i and q_j are initially off by one, they get decremented	\
     * in the do-while loops. */					\
    q_i = q_l + 1; q_j = q_r;						\
    while (1) {								\
	do q_i++; while (Q_LESS(q_i, q_l));				\
	do q_j--; while (Q_LESS(q_l, q_j));				\
	if (q_i >= q_j) break; /* Sedgewick says "until j < i" */	\
	Q_SWAP(q_i, q_j);						\
    }									\
    /* Compensate for the i==j case. */					\
    q_i = q_j + 1;							\
    /* Put the median to its final place. */				\
    Q_SWAP(q_l, q_j);							\
    /* The median is not part of the left subfile. */			\
    q_j--;								\
} while (0)

/* ex:set ts=8 sts=4 sw=4 noet: */
