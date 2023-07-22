#include "common.h"

WINAPI void InitCommonControls() {
	// after comctl32 6.0 does nothing
}

STUB(ImageList_AddMasked)
STUB(ImageList_Destroy)
STUB(ImageList_Create)

static Ordinal ORDINALS[] = {
{
		.num = 17,
		.fn = InitCommonControls
	}
};

__attribute__((used)) Ordinals g_ordinals = {
	.ptr = ORDINALS,
	.count = 1
};
