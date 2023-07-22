#pragma once

typedef struct LIST_ENTRY {
	struct LIST_ENTRY* Flink;
	struct LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY, PRLIST_ENTRY;
