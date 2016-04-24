// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands",  mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display stack backtrace", mon_backtrace},
	{ "showmappings", "Display the physical page mappings", mon_showmappings },
	{ "changeperms", "Change permissions of page mappings", mon_changeperms },
    { "dumpcontents", "Dump contents of a range of memory", mon_dumpcontents },
	{ "exit", "Exit the kernel monitor", mon_exit },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t *stack_top = (uint32_t *) read_ebp();
	uint32_t *args, argi, argn = 5;
	
	struct Eipdebuginfo info;
	
	cprintf("Stack backtrace:\n");
	while (stack_top) {
		uint32_t eip = *(stack_top + 1);
		cprintf("  ebp %08x  eip %08x  args", (uint32_t)stack_top, *(stack_top + 1));
		args = stack_top + 2;
		for (argi = 0; argi < argn; argi++) 
			cprintf(" %08x", args[argi]);
		cprintf("\n");
		debuginfo_eip(eip, &info);
		cprintf("         %s:%d: %.*s+%u\n", 
			info.eip_file, info.eip_line, 
			info.eip_fn_namelen, info.eip_fn_name, 
			eip - info.eip_fn_addr);
		stack_top = (uint32_t *)(*stack_top);
	}
	return 0;
}

static int
str2addr(char *str, void **result) {
	char *end = str;
	*result = (void *)strtol(str, &end, 16);
	if (strlen(str) != (int)(end - str)) 
		return 	0;
	return 1;
}

inline static int 
usage_exit(const char *usage) 
{
	cprintf(usage);
	return 0;
}

static void
show_mapping(void *vaddr)
{
	pte_t *pte;
	if (page_lookup(kern_pgdir, vaddr, &pte)) { 
		void *paddr = (void *)PTE_ADDR(*pte);
		cprintf("va: %08p  pa (of page): %08p  perms: ", vaddr, paddr);
		/* PTE_P ommitted */
		if (*pte & PTE_W)    cprintf("W ");
		if (*pte & PTE_U)    cprintf("U ");
		if (*pte & PTE_PWT)  cprintf("WT ");
		if (*pte & PTE_PCD)  cprintf("CD ");
		if (*pte & PTE_A)    cprintf("A ");
		if (*pte & PTE_D)    cprintf("D ");
		/* PTE_PS ommitted */
		if (*pte & PTE_G)    cprintf("G ");
		cprintf("\n");
	} else {
		cprintf("No physical page mapping at %08p\n", vaddr);
	}
}

int
mon_showmappings(int argc, char** argv, struct Trapframe *tf) 
{	
	const char *usage = "Usage: showmappings vaddr1 [vaddr2]\n";
	void *vaddr1, *vaddr2;

	if (argc < 2 || argc > 3) 	
		return usage_exit(usage);

	if (!str2addr(argv[1], &vaddr1)) 
		return usage_exit(usage);
	
			
	if (argc == 2) {
		show_mapping(vaddr1);
	} else { // argc == 3
		char* vaddr;

		if (!str2addr(argv[2], &vaddr2)) 
			return usage_exit(usage);
		
		for (vaddr = (char *) vaddr1; vaddr < (char *) vaddr2; vaddr += PGSIZE) 
			show_mapping((void *) vaddr);
	}
	return 0;
}

static int 
str2perm(const char* s) {
	if (!strcmp(s, "W"))
		return PTE_W;
	if (!strcmp(s, "U"))
		return PTE_U;
	return -1;
}

int 
mon_changeperms(int argc, char** argv, struct Trapframe *tf) 
{
	const char *usage = "Usage: changeperms vaddr (W|U)+\n";
	void *vaddr;
	pte_t *pte;
	int perms = PTE_P;
	int i;
	
	if (argc < 3) 
		return usage_exit(usage);
	
	if (!str2addr(argv[1], &vaddr))
		return usage_exit(usage);
	
	if (!page_lookup(kern_pgdir, vaddr, &pte)) {
		cprintf("No physical page mapping at %08p\n", vaddr);
		return 0;
	}

	for (i = 2; i < argc; i++) {
		int perm = str2perm(argv[i]);
		if (perm < 0) 
			return usage_exit(usage);
		perms |= perm;
	}
	
	*pte = (*pte & (~0xFFF)) | perms;
		
	return 0;
}

static void 
dump_contents_v(void* va1, void* va2) 
{
	typedef unsigned char byte;

	int count = 0;
	byte *va;

	for (va = (byte *) va1; va < (byte *) va2; va++) {
		if (count == 0) 
			cprintf("%08p:", va);

		cprintf(" %02x", *va);

		if (++count == 16) {
			count = 0;
			cprintf("\n");
		}
	}
	if (count != 0)
		cprintf("\n");
}

static void
dump_contents_p(physaddr_t pa1, physaddr_t pa2) 
{
	//panic("dump_contents_p is not implemented");	
	dump_contents_v(KADDR(pa1), KADDR(pa2));
}

int 
mon_dumpcontents(int argc, char **argv, struct Trapframe *tf) 
{
	const char *usage = "Usage: dumpcontents v vaddr1 vaddr2\n"
	                    "       dumpcontents p paddr1 paddr2\n";
	void *addr1, *addr2;
	if (argc != 4)
		return usage_exit(usage);
	
	if (!str2addr(argv[2], &addr1) || !str2addr(argv[3], &addr2))
		return usage_exit(usage);	

	if (argv[1][0] == 'v' && argv[1][1] == '\0') 
		dump_contents_v(addr1, addr2);
	else if (argv[1][0] == 'p' && argv[1][1] == '\0')
		dump_contents_p((physaddr_t)addr1, (physaddr_t)addr2); 
	else 
		return usage_exit(usage);

	return 0;	
}


int 
mon_exit(int argc, char **argv, struct Trapframe *tf)
{
	return -1;
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
