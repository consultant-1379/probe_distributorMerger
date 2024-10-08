#ifndef _ASM_X86_KVM_PARA_H
#define _ASM_X86_KVM_PARA_H

#include <linux/types.h>

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE	0x40000000

/* This CPUID returns a feature bitmap in eax.  Before enabling a particular
 * paravirtualization, the appropriate feature bit should be checked.
 */
#define KVM_CPUID_FEATURES	0x40000001
#define KVM_FEATURE_CLOCKSOURCE		0
#define KVM_FEATURE_NOP_IO_DELAY	1
#define KVM_FEATURE_MMU_OP		2
/* This indicates that the new set of kvmclock msrs
 * are available. The use of 0x11 and 0x12 is deprecated
 */
#define KVM_FEATURE_CLOCKSOURCE2        3
#define KVM_FEATURE_STEAL_TIME		5

/* The last 8 bits are used to indicate how to interpret the flags field
 * in pvclock structure. If no bits are set, all flags are ignored.
 */
#define KVM_FEATURE_CLOCKSOURCE_STABLE_BIT	24

#define MSR_KVM_WALL_CLOCK  0x11
#define MSR_KVM_SYSTEM_TIME 0x12

#define KVM_MSR_ENABLED 1
/* Custom MSRs falls in the range 0x4b564d00-0x4b564dff */
#define MSR_KVM_WALL_CLOCK_NEW  0x4b564d00
#define MSR_KVM_SYSTEM_TIME_NEW 0x4b564d01
#define MSR_KVM_STEAL_TIME	0x4b564d03

struct kvm_steal_time {
	__u64 steal;
	__u32 version;
	__u32 flags;
	__u32 pad[12];
};

#define KVM_STEAL_ALIGNMENT_BITS 5
#define KVM_STEAL_VALID_BITS ((-1ULL << (KVM_STEAL_ALIGNMENT_BITS + 1)))
#define KVM_STEAL_RESERVED_MASK (((1 << KVM_STEAL_ALIGNMENT_BITS) - 1 ) << 1)

#define KVM_MAX_MMU_OP_BATCH           32

/* Operations for KVM_HC_MMU_OP */
#define KVM_MMU_OP_WRITE_PTE            1
#define KVM_MMU_OP_FLUSH_TLB	        2
#define KVM_MMU_OP_RELEASE_PT	        3

/* Payload for KVM_HC_MMU_OP */
struct kvm_mmu_op_header {
	__u32 op;
	__u32 pad;
};

struct kvm_mmu_op_write_pte {
	struct kvm_mmu_op_header header;
	__u64 pte_phys;
	__u64 pte_val;
};

struct kvm_mmu_op_flush_tlb {
	struct kvm_mmu_op_header header;
};

struct kvm_mmu_op_release_pt {
	struct kvm_mmu_op_header header;
	__u64 pt_phys;
};


#endif /* _ASM_X86_KVM_PARA_H */
