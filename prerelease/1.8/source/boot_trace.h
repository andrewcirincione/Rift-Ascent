#ifndef RIFT_BOOT_TRACE_H
#define RIFT_BOOT_TRACE_H
void boot_trace_reset(void);
void boot_trace_text(const char *stage,const char *value);
void boot_trace(const char *stage,int result);
#endif
