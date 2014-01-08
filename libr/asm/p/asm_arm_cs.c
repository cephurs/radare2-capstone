/* radare2 - LGPL - Copyright 2013-2014 - pancake */

#include <r_asm.h>
#include <r_lib.h>
#include <capstone.h>

static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len) {
	int n, ret, mode;
	cs_insn* insn;
	csh handle;
	mode = (a->bits==16)? CS_MODE_THUMB: CS_MODE_ARM;
	if (a->big_endian)
		mode |= CS_MODE_BIG_ENDIAN;
	else
		mode |= CS_MODE_LITTLE_ENDIAN;
	ret = (a->bits==64)?
		cs_open (CS_ARCH_ARM64, mode, &handle):
		cs_open (CS_ARCH_ARM, mode, &handle);
	op->size = 4;
	strcpy (op->buf_asm, "invalid");
	if (ret) {
		ret = -1;
		goto beach;
	}
	cs_option (handle, CS_OPT_DETAIL, CS_OPT_OFF);
	n = cs_disasm_ex (handle, (ut8*)buf, R_MIN (4, len),
		a->pc, 1, &insn);
	if (n<1) {
		ret = -1;
		goto beach;
	}
	if (insn->size<1) {
		ret = -1;
		goto beach;
	}
	op->size = insn->size;
	snprintf (op->buf_asm, R_ASM_BUFSIZE, "%s%s%s",
		insn->mnemonic,
		insn->op_str[0]?" ":"",
		insn->op_str);
	r_str_rmch (op->buf_asm, '#');
	beach:
	cs_free (insn, n);
	cs_close (handle);
	return op->size;
}

RAsmPlugin r_asm_plugin_arm_cs = {
	.name = "arm.cs",
	.desc = "Capstone ARM disassembler",
	.license = "BSD",
	.arch = "arm",
	.bits = 16|32|64,
	.init = NULL,
	.fini = NULL,
	.disassemble = &disassemble,
	.assemble = NULL
};

#ifndef CORELIB
struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_arm_cs
};
#endif
