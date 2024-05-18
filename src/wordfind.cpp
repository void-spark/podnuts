#include "pod_string.h"
#include "wordfind.h"

struct words_struct words;

char NULL_STRING[]="";     /* define a null string to point to */

/* This function allows us to have a large number of words in a
   sentence, without wasting huge amounts of memory, or worrying
   about dynamic allocation. It requires a buffer of space equal
   to the size of the original input line, and works by converting
   spaces to '\0's, and assigning the word elements apropriately.

   Since punctuation except for a '.' indicates a separate word,
   (to allow for macro expansions and stuff), we toss such a
   leading character into a punctuation string buffer, assign
   the first word to such a buffer, and proceed as usual */

/* global variables: word, word_buffer, punct_buffer, word_count */

struct words_struct *wordfind(const char *input)
{
   char *cur_char;

   clear_words(&words);
   strcpy(words.word_buffer,input);

   if (ispunct (words.word_buffer[0]) && words.word_buffer[0] != '.' )
   {
      words.punct_buffer[0] = words.word_buffer[0];
      words.word[0] = words.punct_buffer;
      words.word_count++;
      cur_char = words.word_buffer+1;
   }
   else cur_char = words.word_buffer; /* point to start of sentence */

   while(words.word_count < MAX_WORDS)
   {
   /* skip spaces */
      while( *cur_char && isspace(*cur_char) ) cur_char++;
   /* by crandonkphin, if end of string hit quit */
      if (*cur_char == '\0') return &words;
   /* point current word to start */
      words.word[words.word_count++] = cur_char;
   /* find end of nonspace word, if end of string, quit */
      while(!isspace(*cur_char) ) if (*cur_char++ == '\0') return &words;
   /* terminate word */
      *cur_char++ = '\0';
   }
   return &words;
}

int clear_words(struct words_struct *words_ptr)
{
  int w;
  for(w=0;w<MAX_WORDS;++w) words_ptr->word[w]=NULL_STRING;
  words_ptr->word_count=0;
  words_ptr->word_buffer[0]='\0';
  words_ptr->punct_buffer[0]='\0';
  return 0;
}


