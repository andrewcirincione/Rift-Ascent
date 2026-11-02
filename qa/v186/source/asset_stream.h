#ifndef RIFT_ASSET_STREAM_H
#define RIFT_ASSET_STREAM_H
#include <stddef.h>
/* Returned pixels stay valid until a subsequent resolve call evicts them.
   Rendering consumes each image synchronously before resolving another. */
const void *rift_asset_resolve(const void *source,unsigned expected_bytes);
int rift_assets_open(void);
void rift_assets_close(void);
extern unsigned rift_asset_reads,rift_asset_errors,rift_asset_recoveries;
#ifndef __PSP__
void rift_assets_set_file(const char *path);
#endif
#endif
