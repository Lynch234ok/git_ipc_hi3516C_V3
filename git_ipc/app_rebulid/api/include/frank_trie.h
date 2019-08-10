
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifndef FRANK_TRIE_H_
#define FRANK_TRIE_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fFRANK_TRIE_DUMP_DATA)(const char *keyword, void *data, char *dump_buff, int dump_max);
typedef struct FRANK_TRIE {

	int (*add)(struct FRANK_TRIE *const trie, const char *keyword, void *data);
	int (*del)(struct FRANK_TRIE *const trie, const char *keyword);
	void (*empty)(struct FRANK_TRIE *const trie);
	
	void * (*find)(struct FRANK_TRIE *const trie, const char *keyword);

	void (*dump)(struct FRANK_TRIE *const trie, const char *title);
	void (*dump2)(struct FRANK_TRIE *const trie, const char *title, fFRANK_TRIE_DUMP_DATA dump_data);
	
}ST_FRANK_TRIE, *LP_FRANK_TRIE;

extern LP_FRANK_TRIE FTRIE_create(int data_size);
extern void FTRIE_release(LP_FRANK_TRIE trie);

#if 0
typedef struct STUDENT_SCORE {
	int maths, physics, biology, chemistry;
}ST_STUDENT_SCORE;

static void student_score_dump(const char *student_name, void *data)
{
	struct STUDENT_SCORE *score = (struct STUDENT_SCORE *)(data);
	printf("%s score: Maths=%d Physics=%d Biology=%d Chemistry=%d",
		student_name, score->maths, score->physics, score->biology, score->chemistry);
}

int main(int argc, char *argv[])
{
	ST_STUDENT_SCORE mike = { 80, 66, 70, 50, };
	ST_STUDENT_SCORE mary = { 92, 55, 91, 72, };
	ST_STUDENT_SCORE lucy = { 71, 78, 88, 84, };
	LP_FRANK_TRIE trie = NULL;

	while(NULL != (trie = FTRIE_create(sizeof(ST_STUDENT_SCORE)))){
		// add the score
		trie->add(trie, "Mike", &mike);
		trie->add(trie, "Mary", &mary);
		trie->add(trie, "Lucy", &lucy);
		trie->dump2(trie, student_score_dump);
		// delete a student
		trie->del(trie, "Mary");
		trie->dump2(trie, student_score_dump);
		// adjust data
		ST_STUDENT_SCORE *score = trie->find(trie, "Mike");
		if(score){
			score->maths = 99;
			// duplicate score
			trie->add(trie, "Macro", score);
		}
		trie->dump2(trie, student_score_dump);
		trie->empty(trie);
		FTRIE_release(trie);

		// loop to check the possibly memory leak
		//break;
	}
	return 0;
}
#endif

#ifdef __cplusplus
};
#endif
#endif //FRANK_TREE_H_

