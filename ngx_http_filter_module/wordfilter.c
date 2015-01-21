/**
 * Program by Yanbin Ma
 *
 * Tue Jan 20 08:35:03 CST 2015
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* A implement of hash using C */
#include "uthash.h"

/* The max length of word */
#define MAXLEN 62

const char *host = "127.0.0.1";
const int   port = 6379;

/* Hash Table */
struct hashtable {
	char k[64];
	float v;
	UT_hash_handle hh;
};

struct hashtable *wc = NULL;		/* word counter */
struct hashtable *ww = NULL;		/* word weights */
struct hashtable *tf = NULL;		/* term frequency */

inline int sort_by_value(struct hashtable *a, struct hashtable *b) {
	return (b->v - a->v);
}

/* The counter of words */
void counter(char *name)
{
	struct hashtable *s;

	HASH_FIND_STR(wc, name, s);
	if (s == NULL) {
		s = (struct hashtable *)malloc(sizeof(struct hashtable));
		strncpy(s->k, name, MAXLEN);
		s->v = 1;
		HASH_ADD_STR(wc, k, s);
	}
	else {
		s->v += 1;
	}
}

void tfcomputer(int sum)
{
	if (!sum) return;

	struct hashtable *s, *f, *w;

	for (s = wc; s != NULL; s = (struct hashtable *)s->hh.next) {
		f = (struct hashtable *)malloc(sizeof(struct hashtable));
		strncpy(f->k, s->k, MAXLEN);
		/* search word weights, if not setting default value 1.0 */
		HASH_FIND_STR(ww, f->k, w);
		f->v = (w != NULL)? (s->v / sum) * w->v : (s->v / sum) * 1.0;
		HASH_ADD_STR(tf, k, f);
	}
	HASH_SORT(tf, sort_by_value);
}

/* Print the hash sorting by value */
void hashprinter(struct hashtable *hash)
{
	HASH_SORT(hash, sort_by_value);

	struct hashtable *s;
	for (s = hash; s != NULL; s = (struct hashtable *)s->hh.next) {
		printf("%16s  %9.5f\n", s->k, s->v);
	}
}

/* Convert the upper to lower */
inline int lc(int c) {
	return (isupper(c))? c - 'A' + 'a' : c;
}


/* The dictionary loader, pull data from redis to word weights */
int dictloader()
{
	char **iter, *words[] = {
		"a", "an", "the", "that", "this", "be", "is", "are", "was", "were",
		"i", "he", "she", "we", "they", "his", "her", "it", "of", "in",
		"at", "on", "to", "by", "for", "from", "with", "as", "or", "not",
		"and", "but", "also", "will", "than", "have", "has", "had", "can", "do",
		"education", "school", "student", "students", "study",
		NULL
	};
	int i = 0, weights[] = {
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
		44,	35,	31,	31,	26
	};


	struct hashtable *s;
	for (iter = words; *iter != NULL; iter++) {
		s = (struct hashtable *)malloc(sizeof(struct hashtable));
		strncpy(s->k, *iter, MAXLEN);
		s->v = weights[i++];
		HASH_ADD_STR(ww, k, s);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp, *fout;

	if (argc == 2)
		if ((fp = fopen(*++argv, "r")) == NULL) {
			fprintf(stderr, "can't open %s\n", *argv);
			return 1;
		}
		else {
			/* load the dictionary first */
			if (dictloader()) return 1;
			/* article handle block */
			char ch, token[64];
			int len, total = 0;

			while ((ch = fgetc(fp)) != EOF) {

				if (isalnum(ch)) {
					len = 0;
					do {
						if (len < MAXLEN)
							token[len++] = lc(ch);
						ch = fgetc(fp);
					} while (ch != EOF && isalnum(ch));
					token[len] = '\0';

					counter(token);
					total += 1;
				}
			}

			tfcomputer(total);
			/* print term frequency */
			hashprinter(tf);
			/* print the analystics table */
			if ((fout = fopen("result", "w")) != NULL) {
				struct hashtable *iter, *count;

				fprintf(fout, "             word      count         tf      value\n");
				fprintf(fout, " -----------------  ---------  ---------  ---------\n");
				for (iter = tf; iter != NULL; iter = (struct hashtable *)iter->hh.next) {
					HASH_FIND_STR(wc, iter->k, count);
					fprintf(fout, " %16s  %9d  %9.5f  %9.5f\n",
							iter->k, (int)count->v, count->v / total, iter->v);
				}
			}
			return 0;
		}
	else {
		fprintf(stderr, "arg error\n");
		return 1;
	}
}
