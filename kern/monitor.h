#ifndef JOS_KERN_MONITOR_H
#define JOS_KERN_MONITOR_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

#define MON_CMD_DECL(name) \
	int mon_##name(int argc, char **argv, struct Trapframe *tf)

// Declare functions implementing monitor commands.

MON_CMD_DECL(help);
MON_CMD_DECL(kerninfo);
MON_CMD_DECL(backtrace);

MON_CMD_DECL(showmappings);
MON_CMD_DECL(changeperms);
MON_CMD_DECL(dumpcontents);

MON_CMD_DECL(exit);

#undef MON_CMD_DECL

#endif	// !JOS_KERN_MONITOR_H
