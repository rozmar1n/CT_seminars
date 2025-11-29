#include <stdio.h>
#include <string.h>
#include <ctype.h>

void check(char *a, char *b,
           int (*cmp)(const char *, const char *))
{
  printf("Comparison in progress...\n");
  if(!(*cmp)(a, b)) printf("Strings are equal\n");
  else printf("Strings are NOT equal\n");
}

int compare_letter_count(const char *a, const char *b)
{
  int count_a = 0, count_b = 0;

  for(const char *p = a; *p; ++p)
    if(isalpha((unsigned char)*p)) ++count_a;

  for(const char *p = b; *p; ++p)
    if(isalpha((unsigned char)*p)) ++count_b;

  return count_a - count_b;
}

int main(int argc, char *argv[])
{
  const int MAX_SIZE = 256;
  char s1[MAX_SIZE], s2[MAX_SIZE];
  int (*p)(const char *, const char *);

  if(argc > 1 && strcmp(argv[1], "letters") == 0)
    p = compare_letter_count;
  else
    p = strcmp;

  printf("Enter two strings.\n");
  if(!fgets(s1, sizeof s1, stdin) || !fgets(s2, sizeof s2, stdin)) {
    printf("Input error\n");
    return 1;
  }

  s1[strcspn(s1, "\n")] = '\0';
  s2[strcspn(s2, "\n")] = '\0';

  if(p == strcmp)
    printf("Using lexicographic comparison.\n");
  else
    printf("Comparing by letter count only.\n");

  check(s1, s2, p); 

  return 0;
}
