#ifndef __HOOK
#define __HOOK
char* TrampHook32(char* src, char* dst, const intptr_t len);
bool Detour32(char* src, char* dst, const intptr_t len);

#define InitTrampHook(name, len) name = (name##_t)TrampHook32((char*)name##_Addr, (char*)name##_Hook, len)
#endif
