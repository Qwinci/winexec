#include "winternl.h"
#ifdef __linux__
#include <asm/prctl.h>
#include <sys/syscall.h>
#endif
#ifdef __managarm__
#include <hel.h>
#include <hel-syscalls.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include "loaderlib/loaderlib.h"
#include <stdio.h>

int main(int argc, const char** argv, const char** envp) {
	const char* name;
	if (argc > 1) {
		name = argv[1];
	}
	else {
		name = "../size64.exe";
	}

	TIB* tib = calloc(1, sizeof(TIB));
	TEB* teb = calloc(1, sizeof(TEB));
	tib->teb = teb;
	PEB* peb = calloc(1, sizeof(PEB));
	teb->ProcessEnvironmentBlock = peb;
	RTL_USER_PROCESS_PARAMETERS* params = calloc(1, sizeof(RTL_USER_PROCESS_PARAMETERS));
	peb->ProcessParameters = params;
	params->ImagePathName.Buffer = L"hello.exe";
	params->ImagePathName.Length = 9;
	params->ImagePathName.MaximumLength = 9;
	params->CommandLine.Buffer = NULL;
	params->CommandLine.Length = 0;
	params->CommandLine.MaximumLength = 0;

#ifdef CONFIG_64BIT
	syscall(SYS_arch_prctl, ARCH_SET_GS, tib);
#else
	syscall(SYS_arch_prctl, ARCH_SET_FS, tib);
#endif

	loaderlib_add_import_path("./");

	PeFile pe;
	Status status = loaderlib_execute(&pe, name, argc, argv, envp);
	if (status == STATUS_BAD_ARCH) {
		fprintf(stderr, "invalid arch, try the other one from (winexec, winexec32)\n");
		return EXIT_FAILURE;
	}
	else if (status != STATUS_SUCCESS) {
		fprintf(stderr, "loading file failed with code %d (%s)\n", status, status_to_str(status));
		return EXIT_FAILURE;
	}

	return 0;
}
