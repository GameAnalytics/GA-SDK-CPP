#if USE_UWP
typedef std::wstring STRING_TYPE;
#define CAT(A, B) A##B
#define TEXT(A) CAT(L, #A)
#define SQLITE(A) { #A, A ## 16 }
#else
typedef std::string STRING_TYPE;
#define TEXT(A) A
#define SQLITE(A) A
#endif