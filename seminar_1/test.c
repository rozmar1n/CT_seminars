#include <stdio.h>
#include <wchar.h>

int main() {
    wchar_t* str = L"Hello, world!";
    
    printf("checking this\n");
    wprintf(L"%ls\n", str);

    return 0;
}
