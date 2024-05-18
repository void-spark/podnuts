#ifndef WORDFIND_H
#define WORDFIND_H

#define MAX_WORDS      200

struct words_struct
{
   int word_count;
   char *word[MAX_WORDS];
   char word_buffer[ARR_SIZE];
   char punct_buffer[2];
};

extern struct words_struct words;

struct words_struct *wordfind(const char *input);
int clear_words(struct words_struct *words_ptr);

#endif /* !WORDFIND_H */
