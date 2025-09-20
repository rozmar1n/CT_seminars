#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

wchar_t* input_string()
{
    wchar_t* buffer = malloc(4096 * sizeof(wchar_t));
    fgetws(buffer, 4096, stdin);
    size_t len = wcslen(buffer);
    if (len > 0 && buffer[len - 1] == L'\n') {
        buffer[len - 1] = L'\0';
    }
    return buffer;
}

struct word_t {
    wchar_t* word;
    size_t ndig;
    size_t nrus;
    size_t neng;
};

size_t count_words(wchar_t* string) {
    wchar_t *lptr = string;
    size_t count = 0;
    while (*lptr != L'\0') {
        while(iswspace(*lptr)) {
            lptr++;
        }
        count++;

        while(*lptr != L'\0' && !iswspace(*lptr)) {
            lptr++;
        }
    }
    return count;
}

int is_rus_letter(wchar_t c) {
    if((c>=L'а' && c<=L'я') || (c>=L'А' && c<=L'Я') || c == L'ё' || c == L'Ё') {
        return 1;
    } else {
        return 0;
    }
}
int is_eng_letter(wchar_t c) {
    if((c>=L'a' && c<=L'z') || (c>=L'A' && c<=L'Z')) return 1;
    else return 0;
}

struct word_t* make_words_array(wchar_t* string, size_t nwords) {
    struct word_t* words = calloc(nwords, sizeof(struct word_t));
    size_t i = 0;
    while (*string != L'\0') {
        while (iswspace(*string)) {
            *string = L'\0';
            string++;
        }
        words[i].word = string;
        while (*string != ' ' && *string != '\t' && *string != '\n' && *string != '\0') {
            if (iswdigit(*string)) 
                words[i].ndig++;

            if (is_rus_letter(*string)) 
                words[i].nrus++;
            
            if (is_eng_letter(*string)) 
                words[i].neng++;
            string++;
        }
        i++;
    }
    return words;
}

void print_words(struct word_t* words, size_t nwords) {
    for (size_t i = 0; i < nwords; i++) {
        wprintf(L"%ls %ld %ld %ld\n", words[i].word, words[i].ndig, words[i].nrus, words[i].neng);
    }
}

int main() 
{
    setlocale(LC_ALL, "");
    wchar_t* string = input_string();
    wprintf(L"%ls\n", string);
    size_t nwords = count_words(string);
    wprintf(L"nwords = %ld\n", nwords);
    struct word_t* words = make_words_array(string, nwords);
    print_words(words, nwords);
    
    free(string);
}
