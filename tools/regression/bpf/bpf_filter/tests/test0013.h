/*-
 * Test 0013:	BPF_ST & BPF_LDX+BPF_MEM
 *
 * $FreeBSD: src/tools/regression/bpf/bpf_filter/tests/test0013.h,v 1.2.2.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 */

/* BPF program */
struct bpf_insn pc[] = {
	BPF_STMT(BPF_LD+BPF_IMM, 0xdeadc0de),
	BPF_STMT(BPF_ST, 7),
	BPF_STMT(BPF_LDX+BPF_MEM, 7),
	BPF_STMT(BPF_MISC+BPF_TXA, 0),
	BPF_STMT(BPF_RET+BPF_A, 0),
};

/* Packet */
u_char	pkt[] = {
	0x00,
};

/* Packet length seen on wire */
u_int	wirelen =	sizeof(pkt);

/* Packet length passed on buffer */
u_int	buflen =	sizeof(pkt);

/* Invalid instruction */
int	invalid =	0;

/* Expected return value */
u_int	expect =	0xdeadc0de;

/* Expected signal */
int	expect_signal =	0;
