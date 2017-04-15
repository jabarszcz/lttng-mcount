#define _GNU_SOURCE
#include <link.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "util/compiler.h"
#include "dynamic.h"

#define ALIGN(n, a)  (((n) + (a) - 1) & ~((a) - 1))
#define PAGE_SIZE  4096
#define PAGE_MASK  (PAGE_SIZE - 1)
#define debug(s, args...)
#define err(s) puts(s)

// Target function
extern void __fentry__(void);

/* In order to jump to __fentry__ which is defined in the code segment
   of lttng-mcount from a function defined in another code segment, we
   need a trampoline as a replacement for the .plt entry that is
   absent with the -mnop-mcount option to gcc. The next structures and
   functions have that purpose.
*/
struct arch_dynamic_info {
	struct arch_dynamic_info *next;
	char *mod_name;
	unsigned long addr;
	unsigned long size;
	unsigned long trampoline;
};

static struct arch_dynamic_info *adinfo;

/* callback for dl_iterate_phdr() */
static int find_dynamic_module(struct dl_phdr_info *info,
			       size_t sz, void *data)
{
	const char *name = info->dlpi_name;
	struct arch_dynamic_info *adi;
	unsigned i;

	if ((data == NULL && name[0] == '\0') || strstr(name, data)) {
		adi = malloc(sizeof(*adi));
		adi->mod_name = strdup(name);

		for (i = 0; i < info->dlpi_phnum; i++) {
			if (info->dlpi_phdr[i].p_type != PT_LOAD)
				continue;

			if (!(info->dlpi_phdr[i].p_flags & PF_X))
				continue;

			/* find address and size of code segment */
			adi->addr = info->dlpi_phdr[i].p_vaddr + info->dlpi_addr;
			adi->size = info->dlpi_phdr[i].p_memsz;
			break;
		}
		adi->next = adinfo;
		adinfo = adi;

		return 1;
	}

	return 0;
}

static void setup_fentry_trampoline(struct arch_dynamic_info *adi)
{
	unsigned char trampoline[] = { 0xff, 0x25, 0x02, 0x00, 0x00, 0x00, 0xcc, 0xcc };
	unsigned long fentry_addr = (unsigned long)__fentry__;

	/* find unused 16-byte at the end of the code segment */
	adi->trampoline = ALIGN(adi->addr + adi->size, PAGE_SIZE) - 16;

	if (unlikely(adi->trampoline < adi->addr + adi->size)) {
		adi->trampoline += 16;
		adi->size += PAGE_SIZE;

		debug("adding a page for fentry trampoline at %#lx\n",
		      adi->trampoline);

		mmap((void *)adi->trampoline, PAGE_SIZE,
		     PROT_READ | PROT_WRITE,
		     MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}

	if (mprotect((void *)adi->addr, adi->size,
		     PROT_READ | PROT_WRITE | PROT_EXEC))
		err("cannot setup trampoline due to protection");

	/* jmpq  *0x2(%rip)     # <fentry_addr> */
	memcpy((void *)adi->trampoline, trampoline, sizeof(trampoline));
	memcpy((void *)adi->trampoline + sizeof(trampoline),
	       &fentry_addr, sizeof(fentry_addr));
}

static unsigned long get_target_addr(unsigned long addr)
{
	struct arch_dynamic_info *adi = adinfo;

	while (adi) {
		if (adi->addr <= addr && addr < adi->addr + adi->size)
			return adi->trampoline - (addr + 5);

		adi = adi->next;
	}
	return 0;
}

static void prepare_dynamic_update(void)
{
	struct arch_dynamic_info *adi;

	dl_iterate_phdr(find_dynamic_module, NULL);

	for(adi = adinfo; adi; adi = adi->next) {
		setup_fentry_trampoline(adi);
	}
}

void dynamic_init(void)
{
	prepare_dynamic_update();
}

int set_instrumentation(unsigned long addr, int enable)
{
	unsigned char nop[] = { 0x67, 0x0f, 0x1f, 0x04, 0x00 };
	unsigned char call[5] = { 0xe8, 0x00, 0x00, 0x00, 0x00 };
	unsigned long new, old;
	unsigned long *insn = (void *)addr;
	unsigned int target_addr;

	target_addr = get_target_addr(addr);
	if (target_addr == 0)
		return -1;

	/* complete call instruction */
	memcpy(&call[1], &target_addr, sizeof(target_addr));

	/* loop for cas success */
	do {
		/* get original bytes */
		new = old = *insn;

		if (!enable && !memcmp(insn, call, sizeof(call))) {
			memcpy(&new, nop, sizeof(nop));
		} else if (enable && !memcmp(insn, nop, sizeof(nop))) {
			memcpy(&new, call, sizeof(call));
		} else return -1;
	} while (!atomic_compare_exchange_weak(insn, &old, new));

	return 0;
}

int get_instrumentation(unsigned long addr,
			enum lttng_mcount_patch *status)
{
	unsigned char nop[] = { 0x67, 0x0f, 0x1f, 0x04, 0x00 };
	unsigned char call[5] = { 0xe8, 0x00, 0x00, 0x00, 0x00 };
	unsigned char *insn = (void *)addr;
	unsigned int target_addr;

	target_addr = get_target_addr(addr);
	if (target_addr == 0)
		return -1;

	/* complete call instruction */
	memcpy(&call[1], &target_addr, sizeof(target_addr));

	if (!memcmp(insn, call, sizeof(call))) {
		*status = CALL;
	} else if (!memcmp(insn, nop, sizeof(nop))) {
		*status = NOP;
	} else {
		*status = NO_PATCH;
	}

	return 0;
}
