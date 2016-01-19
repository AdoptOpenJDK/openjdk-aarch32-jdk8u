/*
 * Copyright (c) 1997, 2011, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2014, 2015, Red Hat Inc. All rights reserved.
 * Copyright (c) 2015, Linaro Ltd. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_AARCH32_VM_ASSEMBLER_AARCH32_HPP
#define CPU_AARCH32_VM_ASSEMBLER_AARCH32_HPP

#include "asm/register.hpp"

// definitions of various symbolic names for machine registers

// First intercalls between C and Java which use 8 general registers
// and 8 floating registers

// we also have to copy between x86 and ARM registers but that's a
// secondary complication -- not all code employing C call convention
// executes as x86 code though -- we generate some of it


/* Calling conventions : this is perhaps not always what is wanted however currently all
  JNI code needs to use the same calling convention as OpenJDK */

class Argument VALUE_OBJ_CLASS_SPEC {
 public:
  enum {
    n_int_register_parameters_c   = 4,  // r0, r1, ... r3 (c_rarg0, c_rarg1, ...)
    // These are number of double registers, each double register can instead be
    // used as two single precision registers.
#ifdef __VFP_FP__
    n_float_register_parameters_c = 8,  // d0, d1, ... d7 (c_farg0, c_farg1, ... )
#define HARD_FLOAT_CC
#elif defined(__SOFTFP__)
    n_float_register_parameters_c = 0,  // Not realy used, in this case
    // pass_float -> pass_int & pass_double -> pass_long
#else
  #error "Floating point ABI not supported";
#endif

    // Not that these only make sense for compiled
    // value are irrelevant currently, if c1, c2, ... will go ahead these need to be
    // sorted out.
    n_int_register_parameters_j   = 4, // r1, ... r7, r0 (rj_rarg0, j_rarg1, ...
    n_float_register_parameters_j = 4  // d0, d1, ... d7 (j_farg0, j_farg1, ...
  };
};

REGISTER_DECLARATION(Register, c_rarg0, r0);
REGISTER_DECLARATION(Register, c_rarg1, r1);
REGISTER_DECLARATION(Register, c_rarg2, r2);
REGISTER_DECLARATION(Register, c_rarg3, r3);

REGISTER_DECLARATION(FloatRegister, c_farg0, d0);
REGISTER_DECLARATION(FloatRegister, c_farg1, d1);
REGISTER_DECLARATION(FloatRegister, c_farg2, d2);
REGISTER_DECLARATION(FloatRegister, c_farg3, d3);
REGISTER_DECLARATION(FloatRegister, c_farg4, d4);
REGISTER_DECLARATION(FloatRegister, c_farg5, d5);
REGISTER_DECLARATION(FloatRegister, c_farg6, d6);
REGISTER_DECLARATION(FloatRegister, c_farg7, d7);

// Symbolically name the register arguments used by the Java calling convention.
// We have control over the convention for java so we can do what we please.
// What pleases us is to offset the java calling convention so that when
// we call a suitable jni method the arguments are lined up and we don't
// have to do much shuffling. A suitable jni method is non-static and a
// small number of arguments
//
//  |-----------------------------------|
//  | c_rarg0  c_rarg1  c_rarg2 c_rarg3 |
//  |-----------------------------------|
//  | r0       r1       r2      r3      |
//  |-----------------------------------|
//  | j_rarg3  j_rarg0  j_rarg1 j_rarg2 |
//  |-----------------------------------|


REGISTER_DECLARATION(Register, j_rarg0, c_rarg1);
REGISTER_DECLARATION(Register, j_rarg1, c_rarg2);
REGISTER_DECLARATION(Register, j_rarg2, c_rarg3);
REGISTER_DECLARATION(Register, j_rarg3, c_rarg0);

// Java floating args are passed as per C

REGISTER_DECLARATION(FloatRegister, j_farg0, d0);
REGISTER_DECLARATION(FloatRegister, j_farg1, d1);
REGISTER_DECLARATION(FloatRegister, j_farg2, d2);
REGISTER_DECLARATION(FloatRegister, j_farg3, d3);
REGISTER_DECLARATION(FloatRegister, j_farg4, d4);
REGISTER_DECLARATION(FloatRegister, j_farg5, d5);
REGISTER_DECLARATION(FloatRegister, j_farg6, d6);
REGISTER_DECLARATION(FloatRegister, j_farg7, d7);

// registers used to hold VM data either temporarily within a method
// or across method calls
// non-volatile (callee-save) registers are r4-r11,r13,r14
// of which the following are dedicated global state

//Program counter
REGISTER_DECLARATION(Register, r15_pc,    r15);
// link register
REGISTER_DECLARATION(Register, lr,        r14);
// Java stack pointer
REGISTER_DECLARATION(Register, sp,        r13);
//TODO REMOVE THIS AND REPLACE ALL OCCURANCES
// WITH JUST SP
REGISTER_DECLARATION(Register, esp,       r13);
//r4 & r5 are callee saved but used as scratch registers
REGISTER_DECLARATION(Register, rscratch2, r12);
REGISTER_DECLARATION(Register, rscratch1, r9); //NOTE this is not actually a scratch register!
// current method -- must be in a call-clobbered register
//Potentially FIXME - it's not!
REGISTER_DECLARATION(Register, rmethod,   r10);
// frame pointer
REGISTER_DECLARATION(Register, rfp,    r11);
// current thread
REGISTER_DECLARATION(Register, rthread,   r8);
// constant pool cache
REGISTER_DECLARATION(Register, rcpool,       r7);
// locals on stack
REGISTER_DECLARATION(Register, rlocals,   r6);
// bytecode pointer
REGISTER_DECLARATION(Register, rbcp,      r5);
// Dispatch table base
REGISTER_DECLARATION(Register, rdispatch, r4);


extern "C" void entry(CodeBuffer *cb);


#define assert_cond(ARG1) assert(ARG1, #ARG1)

namespace asm_util {
  uint32_t encode_logical_immediate(bool is32, uint64_t imm);
};

using namespace asm_util;


class Assembler;

class Instruction_aarch32 {
  unsigned insn;
#ifdef ASSERT
  unsigned bits;
#endif
  Assembler *assem;

public:

  Instruction_aarch32(class Assembler *as) {
#ifdef ASSERT
    bits = 0;
#endif
    insn = 0;
    assem = as;
  }

  inline ~Instruction_aarch32();

  unsigned &get_insn() { return insn; }
#ifdef ASSERT
  unsigned &get_bits() { return bits; }
#endif

  static inline int32_t extend(unsigned val, int hi = 31, int lo = 0) {
    union {
      unsigned u;
      int n;
    };

    u = val << (31 - hi);
    n = n >> (31 - hi + lo);
    return n;
  }

  static inline uint32_t extract(uint32_t val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    assert_cond(msb >= lsb);
    uint32_t mask = (1U << nbits) - 1;
    uint32_t result = val >> lsb;
    result &= mask;
    return result;
  }

  static inline int32_t sextract(uint32_t val, int msb, int lsb) {
    uint32_t uval = extract(val, msb, lsb);
    return extend(uval, msb - lsb);
  }

  static void patch(address a, int msb, int lsb, unsigned long val) {
    int nbits = msb - lsb + 1;
    guarantee(val < (1U << nbits), "Field too big for insn");
    assert_cond(msb >= lsb);
    unsigned mask = (1U << nbits) - 1;
    val <<= lsb;
    mask <<= lsb;
    unsigned target = *(unsigned *)a;
    target &= ~mask;
    target |= val;
    *(unsigned *)a = target;
  }

  static void spatch(address a, int msb, int lsb, long val) {
    int nbits = msb - lsb + 1;
    long chk = val >> (nbits - 1);
    guarantee (chk == -1 || chk == 0, "Field too big for insn");
    unsigned uval = val;
    unsigned mask = (1U << nbits) - 1;
    uval &= mask;
    uval <<= lsb;
    mask <<= lsb;
    unsigned target = *(unsigned *)a;
    target &= ~mask;
    target |= uval;
    *(unsigned *)a = target;
  }

/*  void f(unsigned val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    guarantee(val < (1U << nbits), "Field too big for insn");
    assert_cond(msb >= lsb);
    unsigned mask = (1U << nbits) - 1;
    val <<= lsb;
    mask <<= lsb;
    insn |= val;
    assert_cond((bits & mask) == 0);
#ifdef ASSERT
    bits |= mask;
#endif
  }*/

  void f(unsigned val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    guarantee(val < (1U << nbits), "Field too big for insn");
    assert_cond(msb >= lsb);
    unsigned mask = (1U << nbits) - 1;
    val <<= lsb;
    mask <<= lsb;
    insn &= ~mask;
    insn |= val;
#ifdef ASSERT
    bits |= mask;
#endif
  }

  void f(unsigned val, int bit) {
    f(val, bit, bit);
  }

  void sf(long val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    long chk = val >> (nbits - 1);
    guarantee (chk == -1 || chk == 0, "Field too big for insn");
    unsigned uval = val;
    unsigned mask = (1U << nbits) - 1;
    uval &= mask;
    f(uval, lsb + nbits - 1, lsb);
  }

  void rf(Register r, int lsb) {
    f(r->encoding_nocheck(), lsb + 3, lsb);
  }

  void rf(FloatRegister r, int lsb) {
    f(r->encoding_nocheck(), lsb + 4, lsb);
  }

  unsigned get(int msb = 31, int lsb = 0) {
    int nbits = msb - lsb + 1;
    unsigned mask = ((1U << nbits) - 1) << lsb;
    assert_cond((bits & mask) == mask);
    return (insn & mask) >> lsb;
  }

  void fixed(unsigned value, unsigned mask) {
    assert_cond ((mask & bits) == 0);
#ifdef ASSERT
    bits |= mask;
#endif
    insn |= value;
  }
};

#define starti Instruction_aarch32 do_not_use(this); set_current(&do_not_use)

// abs methods which cannot overflow and so are well-defined across
// the entire domain of integer types.
static inline unsigned int uabs(unsigned int n) {
  union {
    unsigned int result;
    int value;
  };
  result = n;
  if (value < 0) result = -result;
  return result;
}
static inline unsigned long uabs(unsigned long n) {
  union {
    unsigned long result;
    long value;
  };
  result = n;
  if (value < 0) result = -result;
  return result;
}
static inline unsigned long uabs(long n) { return uabs((unsigned long)n); }
static inline unsigned long uabs(int n) { return uabs((unsigned int)n); }

#define S_DFLT ::lsl()
#define C_DFLT AL


// Shift for base reg + reg offset addressing
class shift_op {
 public:
  enum shift_kind { LSL, LSR, ASR, ROR };
 private:
  enum shift_source { imm_s, reg_s };
  enum shift_source _source;
  enum shift_kind _op;
  int _shift;
  Register _reg;

  bool check_valid() {
    if(imm_s == _source) {
      switch(_op) {
        case LSL: return _shift >= 0 && _shift <= 31;
        case ROR: return _shift >= 1 && _shift <= 32;
        default:  return _shift >= 1 && _shift <= 32;
      }
    }
    return true; //Don't check register shifts
  }
 public:
  // Default shift is lsl(0)
  shift_op()
    : _source(imm_s), _op(LSL), _shift(0) { }
  shift_op(enum shift_kind op, int shift)
    : _source(imm_s), _op(op), _shift(shift) {
    if(!shift) {
      // All zero shift encodings map to LSL 0
      _shift = 0;
      _op = LSL;
    }
    int pshift = _shift;
    if(-1 == _shift && ROR == _op) {
      // This is an RRX, make shift valid for the check
      _shift = 1;
      pshift = 0; //set to zero
    }
    assert(check_valid(), "Invalid shift quantity");
    _shift = pshift; //restore shift
  }
  shift_op(enum shift_kind op, Register r)
    : _source(reg_s), _op(op), _reg(r) {}

  shift_kind kind() const {
    return _op;
  }

  int shift() const {
    assert(imm_s == _source, "Not an immediate shift");
    return _shift;
  }
  Register reg() const {
    assert(reg_s == _source, "Not a register shift");
    return _reg;
  }
  bool is_register() {
    return reg_s == _source;
  }
  bool operator==(const shift_op& other) const {
    if(imm_s == _source && imm_s == other._source) {
      return _op == other._op && _shift == other._shift;
    } else if (reg_s == _source && imm_s == _source) {
      return _op == other._op && _reg == other._reg;
    }
    return false;
  }
  bool operator!=(const shift_op& other) const {
    return !( *this == other);
  }
};
class lsl : public shift_op {
 public:
  lsl(int sft = 0): shift_op(LSL, sft) { }
  lsl(Register r): shift_op(LSL, r) { }
};
class lsr : public shift_op {
 public:
  lsr(int sft = 0): shift_op(LSR, sft) { }
  lsr(Register r): shift_op(LSR, r) { }
};
class asr : public shift_op {
 public:
  asr(int sft = 0): shift_op(ASR, sft) { }
  asr(Register r): shift_op(ASR, r) { }
};
class ror : public shift_op {
 public:
  ror(int sft = 0): shift_op(ROR, sft) {}
  ror(Register r): shift_op(ROR, r) { }
};
class rrx : public shift_op {
 public:
  rrx(): shift_op(ROR, -1) {}
};


// Addressing modes
class Address VALUE_OBJ_CLASS_SPEC {
 public:
  enum access_mode { no_mode, imm, reg, lit };
  //literal is class of imm? -> potentially have to split later if some instructions work
  // with one but not other although can be determined from registers.
  enum wb_mode { off, pre, post };

  enum reg_op { ADD, SUB };

 private:
  Register _base;
  Register _index;
  int _offset;
  enum access_mode _acc_mode;
  enum wb_mode _wb_mode;
  enum reg_op _as_op;
  shift_op _shift;

  RelocationHolder _rspec;

  // Typically we use AddressLiterals we want to use their rval
  // However in some situations we want the lval (effect address) of
  // the item.  We provide a special factory for making those lvals.
  bool _is_lval;

  // If the target is far we'll need to load the ea of this to a
  // register to reach it. Otherwise if near we can do PC-relative
  // addressing.
  address _target;

 public:
  Address()
    : _acc_mode(no_mode) { }
  //immediate & literal
  Address(Register r, enum wb_mode mode = off)
    : _base(r), _index(noreg), _offset(0), _acc_mode(imm), _wb_mode(mode),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
  Address(Register r, int o, enum wb_mode mode = off)
    : _base(r), _index(noreg), _offset(o), _acc_mode(imm), _wb_mode(mode),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
  Address(Register r, long o, enum wb_mode mode = off)
    : _base(r), _index(noreg), _offset(o), _acc_mode(imm), _wb_mode(mode),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
  Address(Register r, unsigned long o, enum wb_mode mode = off)
    : _base(r), _index(noreg), _offset(o), _acc_mode(imm), _wb_mode(mode),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
  Address(Register r, unsigned int o, enum wb_mode mode = off)
    : _base(r), _index(noreg), _offset(o), _acc_mode(imm), _wb_mode(mode),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
#ifdef ASSERT
  Address(Register r, ByteSize disp)
    : _base(r), _index(noreg), _offset(in_bytes(disp)), _acc_mode(imm), _wb_mode(off),
      _shift(lsl()), _target(0) {
    assert(!(r == r15_pc && _wb_mode == pre), "The PC can't be pre-indexed.");
  }
#endif


  //Register-offset
  Address(Register r, Register r1, shift_op shift = lsl(), enum reg_op op = ADD,
          enum wb_mode wbm = off)
    : _base(r), _index(r1), _offset(0), _acc_mode(reg), _wb_mode(wbm), _as_op(op),
      _shift(shift), _target(0) {
        assert(!shift.is_register(), "Can't shift a register-offset address by a register");
  }

  Address(address target, RelocationHolder const& rspec)
    : _acc_mode(lit),
      _base(sp),
      _wb_mode(off),
      _rspec(rspec),
      _is_lval(false),
      _target(target)
      { }
  Address(address target, relocInfo::relocType rtype = relocInfo::external_word_type);

 private:
  //Could be either
  void AddressConstruct(Register base, RegisterOrConstant index, enum reg_op op, shift_op shift,
                        enum wb_mode mode);
 public:

  Address(Register base, RegisterOrConstant index, enum reg_op op, enum wb_mode mode) {
    AddressConstruct(base, index, op, lsl(), mode);
  }
  Address(Register base, RegisterOrConstant index, shift_op shift = lsl(), enum reg_op op = ADD,
          enum wb_mode mode = off) {
    if(shift.kind() != lsl().kind()) {
      assert(index.is_register(), "should be");
    }
    AddressConstruct(base, index, op, shift, mode);
  }


  Register base() const {
    //in aarch64 this didn't apply to preindex mode -> why?
    guarantee(_acc_mode == imm || _acc_mode == reg, "wrong mode");
    return _base;
  }
  long offset() const {
    return _offset;
  }
  Register index() const {
    return _index;
  }
  access_mode get_mode() const {
    return _acc_mode;
  }
  wb_mode get_wb_mode() const {
    return _wb_mode;
  }
  bool uses(Register reg) const { return _base == reg || _index == reg; }
  address target() const { return _target; }
  const RelocationHolder& rspec() const { return _rspec; }

  void encode(Instruction_aarch32 *i, CodeSection *sec, address pc) const;

  void fp_encode(Instruction_aarch32 *i, CodeSection *sec, address pc) const;

  void lea(MacroAssembler *, Register) const;

  static bool offset_ok_for_immed(long imm12);
};

// Convience classes
class RuntimeAddress: public Address {
  public:
    RuntimeAddress(address target) : Address(target, relocInfo::runtime_call_type) {}
};

class OopAddress: public Address {
  public:
    OopAddress(address target) : Address(target, relocInfo::oop_type){}
};

class ExternalAddress: public Address {
 private:
  static relocInfo::relocType reloc_for_target(address target) {
    // Sometimes ExternalAddress is used for values which aren't
    // exactly addresses, like the card table base.
    // external_word_type can't be used for values in the first page
    // so just skip the reloc in that case.
    return external_word_Relocation::can_be_relocated(target) ? relocInfo::external_word_type : relocInfo::none;
  }

 public:
    ExternalAddress(address target) : Address(target, reloc_for_target(target)) {}
};

class InternalAddress: public Address {
  public:
    InternalAddress(address target) : Address(target, relocInfo::internal_word_type) {}
};



//TODO Change this back to 16 * 2
const int FPUStateSizeInWords = 32 * 2;
typedef enum {
  PLDL1KEEP = 0b00000, PLDL1STRM, PLDL2KEEP, PLDL2STRM, PLDL3KEEP, PLDL3STRM,
  PSTL1KEEP = 0b10000, PSTL1STRM, PSTL2KEEP, PSTL2STRM, PSTL3KEEP, PSTL3STRM,
  PLIL1KEEP = 0b01000, PLIL1STRM, PLIL2KEEP, PLIL2STRM, PLIL3KEEP, PLIL3STRM
} prfop;



class Assembler : public AbstractAssembler {
#ifndef PRODUCT
  static const unsigned long asm_bp;

  void emit_long(jint x) {
    if ((unsigned long)pc() == asm_bp)
      asm volatile ("nop");
    AbstractAssembler::emit_int32(x);
  }
#else
  void emit_long(jint x) {
    AbstractAssembler::emit_int32(x);
  }
#endif

public:
  //TODO REMOVE shift_kind from here once done
  enum shift_kind { LSL, LSR, ASR, ROR };
  // NOTE RRX is a special case of ROR with shift = 0#

  // Helper functions for shifts
  // Here to allow compiler to find global shift_op without :: prefix as lsl is a
  // standalone instruction
#define HELPER(NAME)                                                                \
  shift_op NAME(int sft = 0) { return ::NAME(sft); }                                \
  shift_op NAME(Register r) { return ::NAME(r); }
  HELPER(lsl);
  HELPER(lsr);
  HELPER(asr);
  HELPER(ror);
  shift_op rrx() { return ::rrx(); }
#undef HELPER

  typedef enum {
    EQ, NE, HS, CS=HS, LO, CC=LO, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV
  } Condition;

  enum { instruction_size = 4 };

  Address adjust(Register base, int offset, bool preIncrement) {
    if (preIncrement)
      return Address(base, offset, Address::pre);
    else
      return Address(base, offset, Address::post);
  }

  Address pre(Register base, int offset) {
    return adjust(base, offset, true);
  }

  Address post (Register base, int offset) {
    return adjust(base, offset, false);
  }

  Instruction_aarch32* current;

  void set_current(Instruction_aarch32* i) { current = i; }

  void f(unsigned val, int msb, int lsb) {
    current->f(val, msb, lsb);
  }
  void f(unsigned val, int msb) {
    current->f(val, msb, msb);
  }
  void sf(long val, int msb, int lsb) {
    current->sf(val, msb, lsb);
  }
  void rf(Register reg, int lsb) {
    current->rf(reg, lsb);
  }
  void rf(FloatRegister reg, int lsb) {
    current->rf(reg, lsb);
  }
  void fixed(unsigned value, unsigned mask) {
    current->fixed(value, mask);
  }

  void emit() {
    emit_long(current->get_insn());
    assert_cond(current->get_bits() == 0xffffffff);
    current = NULL;
  }

  typedef void (Assembler::* uncond_branch_insn)(address dest);
  typedef void (Assembler::* cond_branch_insn)(address dest, Condition cond);
  typedef void (Assembler::* cond_ldst_insn)(Register Rt, address dest, Condition cond);
  typedef void (Assembler::* cond_fp_ldst_insn)(FloatRegister Vd, address dest, Condition cond);

  void wrap_label(Label &L, uncond_branch_insn insn);
  void wrap_label(Label &L, Condition cond, cond_branch_insn insn);
  void wrap_label(Register r, Label &L, Condition cond, cond_ldst_insn insn);
  void wrap_label(FloatRegister r, Label &L, Condition cond, cond_fp_ldst_insn insn);

#undef INSN

// AARCH32 Instructions
// Defined roughly in the order they are found in
// ARM Archicture Reference Manual, section 5

#define ZERO_ADDR_REG r0
#define ONES_ADDR_REG r15

// Data processing (register & register-shifted-register)
  void reg_instr(int decode, shift_op shift, Condition cond, bool s) {
    f(cond, 31, 28), f(0b000, 27, 25), f(decode, 24, 21), f(s, 20);
    f(shift.shift(), 11, 7), f(shift.kind(), 6, 5), f(0, 4);
  }
  void reg_shift_reg_instr(int decode, enum shift_op::shift_kind kind,
                           Condition cond, bool s) {
    f(cond, 31, 28), f(0b000, 27, 25), f(decode, 24, 21), f(s, 20);
    f(0, 7), f(kind, 6, 5), f(1, 4);
  }

#define INSN(NAME, decode, s_flg)                                                   \
  void NAME(Register Rd, Register Rn, Register Rm, shift_op shift = S_DFLT,         \
            Condition cond = C_DFLT) {                                              \
    starti;                                                                         \
    if(shift.is_register()) {                                                       \
      reg_shift_reg_instr(decode, shift.kind(), cond, s_flg);                       \
      rf(Rn, 16), rf(Rd, 12), rf(shift.reg(), 8), rf(Rm, 0);                        \
    } else {                                                                        \
      reg_instr(decode, shift, cond, s_flg);                                        \
      rf(Rn, 16), rf(Rd, 12), rf(Rm, 0);                                            \
    }                                                                               \
  }
  INSN(andr, 0b0000, 0);
  INSN(eor,  0b0001, 0);
  INSN(sub,  0b0010, 0);
  INSN(rsb,  0b0011, 0);
  INSN(add,  0b0100, 0);
  INSN(adc,  0b0101, 0);
  INSN(sbc,  0b0110, 0);
  INSN(rsc,  0b0111, 0);
  INSN(orr,  0b1100, 0);
  INSN(bic,  0b1110, 0);
  INSN(mvn,  0b1111, 0);

  INSN(ands, 0b0000, 1);
  INSN(eors, 0b0001, 1);
  INSN(subs, 0b0010, 1);
  INSN(rsbs, 0b0011, 1);
  INSN(adds, 0b0100, 1);
  INSN(adcs, 0b0101, 1);
  INSN(sbcs, 0b0110, 1);
  INSN(rscs, 0b0111, 1);
  INSN(orrs, 0b1100, 1);
  INSN(bics, 0b1110, 1);
  INSN(mvns, 0b1111, 1);

#undef INSN

#define INSN(NAME, decode)                                                           \
  void NAME(Register Rn, Register Rm, shift_op shift = S_DFLT,                       \
            Condition cond = C_DFLT) {                                               \
    starti;                                                                          \
    if(shift.is_register()) {                                                        \
        reg_shift_reg_instr(decode, shift.kind(), cond, true);                       \
    rf(Rn, 16), f(0b0000, 15, 12), rf(shift.reg(), 8), rf(Rm, 0);                    \
    } else {                                                                         \
      reg_instr(decode, shift, cond, true);                                          \
      rf(Rn, 16), f(0, 15, 12), rf(Rm, 0);                                           \
    }                                                                                \
  }
  INSN(tst, 0b1000);
  INSN(teq, 0b1001);
  INSN(cmp, 0b1010);
  INSN(cmn, 0b1011);
#undef INSN

// TODO appears that if Rd = 15 and s flag set then perhaps different method
void mov_internal(Register Rd, Register Rnm, shift_op shift, bool s, Condition cond) {
  starti;
  if(shift.is_register()) {
    reg_shift_reg_instr(0b1101, shift.kind(), cond, s);
    f(0b0000, 19, 16), rf(Rd, 12), rf(shift.reg(), 8), rf(Rnm, 0);
  } else {
    reg_instr(0b1101, shift, cond, s);
    f(0, 19, 16), rf(Rd, 12), rf(Rnm, 0);
  }
}
void mov(Register Rd, Register Rm, shift_op shift, Condition cond = C_DFLT) {
  mov_internal(Rd, Rm, shift, false, cond);
}
void movs(Register Rd, Register Rm, shift_op shift, Condition cond = C_DFLT) {
  mov_internal(Rd, Rm, shift, true, cond);
}
void mov(Register Rd, Register Rm, Condition cond = C_DFLT) {
  mov_internal(Rd, Rm, S_DFLT, false, cond);
}
void movs(Register Rd, Register Rm, Condition cond = C_DFLT) {
  mov_internal(Rd, Rm, S_DFLT, true, cond);
}

#define INSN(NAME, type, s_flg, ASSERTION)                                           \
  void NAME(Register Rd, Register Rm, unsigned shift, Condition cond = C_DFLT) {     \
    assert_cond(ASSERTION);                                                          \
    if(s_flg) movs(Rd, Rm, shift_op(type, shift), cond);                             \
    else       mov(Rd, Rm, shift_op(type, shift), cond);                             \
  }
  INSN(lsl, shift_op::LSL, 0, true);
  INSN(lsr, shift_op::LSR, 0, true);
  INSN(asr, shift_op::ASR, 0, true);
  INSN(ror, shift_op::ROR, 0, shift != 0); //shift == 0 => RRX

  INSN(lsls, shift_op::LSL, 1, true);
  INSN(lsrs, shift_op::LSR, 1, true);
  INSN(asrs, shift_op::ASR, 1, true);
  INSN(rors, shift_op::ROR, 1, shift != 0); //shift == 0 => RRX
#undef INSN

#define INSN(NAME, type, s_flg)                                                      \
  void NAME(Register Rd, Register Rm, Condition cond = C_DFLT) {                     \
    if(s_flg) movs(Rd, Rm, shift_op(type, 0), cond);                                 \
    else       mov(Rd, Rm, shift_op(type, 0), cond);                                 \
  }
  INSN(rrx,  shift_op::LSR, 0);
  INSN(rrxs, shift_op::LSR, 1);
#undef INSN

//Data processing (register-shifted-register)
#define INSN(NAME, type, s_flg)                                                      \
  void NAME(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {        \
    if(s_flg) movs(Rd, Rn, shift_op(type, Rm), cond);                                \
    else       mov(Rd, Rn, shift_op(type, Rm), cond);                                \
  }
  INSN(lsl, shift_op::LSL, 0);
  INSN(lsr, shift_op::LSR, 0);
  INSN(asr, shift_op::ASR, 0);
  INSN(ror, shift_op::ROR, 0);

  INSN(lsls, shift_op::LSL, 1);
  INSN(lsrs, shift_op::LSR, 1);
  INSN(asrs, shift_op::ASR, 1);
  INSN(rors, shift_op::ROR, 1);
#undef INSN



//Data processing (immediate)
static bool operand_valid_immediate12(int imm);
static u_int32_t encode_immediate12(int imm);
static u_int32_t decode_immediate12(int imm);


  bool imm_instr(int decode, Register Rd, Register Rn, int imm, Condition cond,
                 bool s) {
    if(!operand_valid_immediate12(imm))
      return false;
    {
      starti;
      f(cond, 31, 28), f(0b001, 27, 25), f(decode, 24, 21), f(s, 20), rf(Rn, 16);
      int imm12 = encode_immediate12(imm);
      rf(Rd, 12), f(imm12, 11, 0);
    }
    return true;
  }

#define INSN(NAME, decode, s_flg)                                                    \
  inline void NAME(Register Rd, Register Rn, unsigned imm, Condition cond = C_DFLT) {\
    bool status = imm_instr(decode, Rd, Rn, imm, cond, s_flg);                       \
    assert(status, "invalid imm");                                                   \
  }
  INSN(andr, 0b0000, 0);
  INSN(eor,  0b0001, 0);
  INSN(orr,  0b1100, 0);
  INSN(bic,  0b1110, 0);

  INSN(ands, 0b0000, 1);
  INSN(eors, 0b0001, 1);
  INSN(orrs, 0b1100, 1);
  INSN(bics, 0b1110, 1);
  //NOTE: arithmetic immediate instructions are defined below to allow dispatch.
#undef INSN
 protected:
  // Mov data to destination register in the shortest number of instructions
  // possible.
  void mov_immediate(Register dst, u_int32_t imm32, Condition cond, bool s);
  // Mov data to destination register but always emit enough instructions that would
  // permit any 32-bit constant to be loaded. (Allow for rewriting later).
  void mov_immediate32(Register dst, u_int32_t imm32, Condition cond, bool s);

   void add_sub_imm(int decode, Register Rd, Register Rn, int imm,
                   Condition cond, bool s);

 public:
#define INSN(NAME, decode, s_flg)                                                    \
  inline void NAME(Register Rd, Register Rn, int imm, Condition cond = C_DFLT) {     \
    assert(Rn->encoding_nocheck() != 0b1111, "add/sub with PC is adr");              \
    add_sub_imm(decode, Rd, Rn, imm, cond, s_flg);                                   \
  }                                                                                  \
  inline void NAME(Register Rd, Register Rn, unsigned imm,                           \
                   Condition cond = C_DFLT) {                                        \
    assert(Rn->encoding_nocheck() != 0b1111, "add/sub with PC is adr");              \
    add_sub_imm(decode, Rd, Rn, imm, cond, s_flg);                                   \
  }                                                                                  \
  inline void NAME(Register Rd, Register Rn, long imm, Condition cond = C_DFLT) {    \
    assert(Rn->encoding_nocheck() != 0b1111, "add/sub with PC is adr");              \
    add_sub_imm(decode, Rd, Rn, imm, cond, s_flg);                                   \
  }                                                                                  \
  inline void NAME(Register Rd, Register Rn, unsigned long imm,                      \
                   Condition cond = C_DFLT) {                                        \
    assert(Rn->encoding_nocheck() != 0b1111, "add/sub with PC is adr");              \
    add_sub_imm(decode, Rd, Rn, imm, cond, s_flg);                                   \
  }                                                                                  \
  /*Addition dispatch - place in macroassembler?*/                                   \
  void NAME(Register Rd, Register Rn, RegisterOrConstant operand,                    \
           Condition cond = C_DFLT) {                                                \
    if(operand.is_register()) {                                                      \
      NAME(Rd, Rn, (Register)operand.as_register(), lsl(), cond);                    \
    } else {                                                                         \
      NAME(Rd, Rn, (unsigned)operand.as_constant(), cond);                           \
    }                                                                                \
  }
  //Note that the RegisterOrConstant version can't take a shift even though
  // one of the instructions dispatched to can
  INSN(sub,  0b0010, 0);
  INSN(rsb,  0b0011, 0);
  INSN(add,  0b0100, 0);
  INSN(adc,  0b0101, 0);
  INSN(sbc,  0b0110, 0);
  INSN(rsc,  0b0111, 0);

  INSN(subs, 0b0010, 1);
  INSN(rsbs, 0b0011, 1);
  INSN(adds, 0b0100, 1);
  INSN(adcs, 0b0101, 1);
  INSN(sbcs, 0b0110, 1);
  INSN(rscs, 0b0111, 1);
#undef INSN
  //No need to do reverse as register subtracted from immediate

  //Helper op
  void inv(Register Rd, Register Rn, Condition cond = C_DFLT) {
    rsb(Rd, Rn, 0, cond);
    sub(Rd, Rd, 1);
  }
  //alias for rsb
  void neg(Register Rd, Register Rn, Condition cond = C_DFLT) {
    rsb(Rd, Rn, 0, cond);
  }
  void negs(Register Rd, Register Rn, Condition cond = C_DFLT) {
    rsbs(Rd, Rn, 0, cond);
  }

  // PC-rel. addressing
  void adr_encode(Register Rd, int imm, Condition cond) {
    add_sub_imm(0b0100, Rd, r15_pc, imm, cond, false); //opcode for add
  }

  void adr(Register Rd, address dest, Condition cond = C_DFLT);

  void adr(Register Rd, const Address &dest, Condition cond = C_DFLT);

  void adr(Register Rd, Label &L, Condition cond = C_DFLT) {
    wrap_label(Rd, L, cond, &Assembler::Assembler::adr);
  }

private:
  friend void entry(CodeBuffer *cb);
#define INSN(NAME, decode, s_flg)                                                    \
  inline void NAME(Register Rd, unsigned imm, Condition cond = C_DFLT) {             \
    bool status = imm_instr(decode, Rd, ZERO_ADDR_REG, imm, cond, s_flg);            \
    assert(status, "invalid imm");                                                   \
  }                                                                                  \
  inline void NAME(Register Rd, int imm, Condition cond = C_DFLT) {                  \
   bool status = imm_instr(decode, Rd, ZERO_ADDR_REG, imm, cond, s_flg);             \
   assert(status, "invalid imm");                                                    \
  }
  INSN(mov_i, 0b1101, 0);
  INSN(mvn_i, 0b1111, 0);

  INSN(movs_i, 0b1101, 1);
  INSN(mvns_i, 0b1111, 1);
#undef INSN

void movw_i(Register Rd, unsigned imm, Condition cond = C_DFLT) {
  starti;
  assert(imm < (1 << 16), "Immediate too big for movw");
  f(cond, 31, 28), f(0b00110000, 27, 20), f(imm >> 12, 19, 16);
  rf(Rd, 12), f(imm & 0xfff, 11, 0);
}

void movt_i(Register Rd, unsigned imm, Condition cond = C_DFLT) {
  starti;
  assert(imm < (1 << 16), "Immediate too big for movt");
  f(cond, 31, 28), f(0b00110100, 27, 20), f(imm >> 12, 19, 16);
  rf(Rd, 12), f(imm & 0xfff, 11, 0);
}
 public:

#define INSN(NAME, decode)                                                           \
  inline void NAME(Register Rn, int imm, Condition cond = C_DFLT) {                  \
    bool status = imm_instr(decode, ZERO_ADDR_REG, Rn, imm, cond, true);             \
    assert(status, "invalid imm");                                                   \
  }                                                                                  \
  inline void NAME(Register Rn, unsigned imm, Condition cond = C_DFLT) {             \
    bool status = imm_instr(decode, ZERO_ADDR_REG, Rn, imm, cond, true);             \
    assert(status, "invalid imm");                                                   \
  }
  INSN(tst, 0b1000);
  INSN(teq, 0b1001);
  INSN(cmp, 0b1010);
  INSN(cmn, 0b1011);
#undef INSN


// Multiply and multiply accumulate
  void mult_instr(int decode, Register a, Register b, Register c,
                  Register d, Condition cond, bool s) {
    starti;
    f(cond, 31, 28), f(0b0000, 27, 24), f(decode, 23, 21), f(s, 20);
    rf(a, 16), rf(b, 12), rf(c, 8), rf(d, 0), f(0b1001, 7, 4);
  }

  void mul(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {
    mult_instr(0b000, Rd, ZERO_ADDR_REG, Rm, Rn, cond, false);
  }
  void muls(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {
    mult_instr(0b000, Rd, ZERO_ADDR_REG, Rm, Rn, cond, true);
  }

  void mla(Register Rd, Register Rn, Register Rm, Register Ra, Condition cond = C_DFLT) {
    mult_instr(0b001, Rd, Ra, Rm, Rn, cond, false);
  }
  void mlas(Register Rd, Register Rn, Register Rm, Register Ra, Condition cond = C_DFLT) {
    mult_instr(0b001, Rd, Ra, Rm, Rn, cond, true);
  }

  void mls(Register Rd, Register Rn, Register Rm, Register Ra, Condition cond = C_DFLT) {
    mult_instr(0b011, Rd, Ra, Rm, Rn, cond, false);
  }

  void umaal(Register RdLo, Register RdHi, Register Rn, Register Rm, Condition cond = C_DFLT) {
    mult_instr(0b010, RdHi, RdLo, Rm, Rn, cond, false);
  }

#define INSN(NAME, decode, s_flg)                                                    \
  void NAME(Register RdLo, Register RdHi, Register Rn, Register Rm,                  \
            Condition cond = C_DFLT) {                                               \
    mult_instr(decode, RdHi, RdLo, Rm, Rn, cond, s_flg);                             \
  }
  INSN(umull, 0b100, 0);
  INSN(umlal, 0b101, 0);
  INSN(smull, 0b110, 0);
  INSN(smlal, 0b111, 0);

  INSN(umulls, 0b100, 1);
  INSN(umlals, 0b101, 1);
  INSN(smulls, 0b110, 1);
  INSN(smlals, 0b111, 1);

#undef INSN

//Saturating addition and subtraction
#define INSN(NAME, decode)                                                           \
  void NAME(Register Rd, Register Rm, Register Rn, Condition cond = C_DFLT) {        \
    starti;                                                                          \
    f(cond, 31, 28), f( 0b00010, 27, 23), f(decode, 22, 21), f(0, 20);               \
    rf(Rn, 16), rf(Rd, 12), f( 0b00000101, 11, 4),  rf(Rm, 0);                       \
  }
  INSN(qadd,  0b00);
  INSN(qsub,  0b01);
  INSN(qdadd, 0b10);
  INSN(qdsub, 0b11);
#undef INSN

// Halfword multiply and multiply accumulate
  void mul_instr(int decode, Register Ra, Register Rb, Register Rc, Register Rd,
                 bool N, bool M, Condition cond) {
      starti;
      f(cond, 31, 28), f(0b00010, 27, 23), f(decode, 22, 21), f(0, 20);
      rf(Ra, 16), rf(Rb, 12), rf(Rc, 8), f(1, 7), f(M, 6), f(N, 5), f(0, 4);
      rf(Rd, 0);
  }

#define INSN(NAME, decode, N, M)                                                     \
  void NAME(Register Rd, Register Rn, Register Rm, Register Ra,                      \
            Condition cond = C_DFLT) {                                               \
    mul_instr(decode, Rd, Ra, Rm, Rn, N, M, cond);                                   \
  }
  INSN(smlabb, 0b00, 0, 0);
  INSN(smlabt, 0b00, 0, 1)
  INSN(smlatb, 0b00, 1, 0)
  INSN(smlatt, 0b00, 1, 1)

  INSN(smlawb, 0b01, 0, 0);
  INSN(smlawt, 0b01, 0, 1);
#undef INSN

#define INSN(NAME, decode, N, M)                                                     \
  void NAME(Register RdLo, Register RdHi, Register Rn, Register Rm,                  \
            Condition cond = C_DFLT) {                                               \
    mul_instr(decode, RdHi, RdLo, Rm, Rn, N, M, cond);                               \
  }
  INSN(smlalbb, 0b10, 0, 0);
  INSN(smlalbt, 0b10, 0, 1);
  INSN(smlaltb, 0b10, 1, 0);
  INSN(smlaltt, 0b10, 1, 1);
#undef INSN

#define INSN(NAME, decode, N, M)                                                     \
  void NAME(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {        \
    mul_instr(decode, Rd, ZERO_ADDR_REG, Rm, Rn, N, M, cond);                        \
  }
  INSN(smulwb, 0b01, 1, 0);
  INSN(smulwt, 0b01, 1, 1);

  INSN(smulbb, 0b11, 0, 0);
  INSN(smulbt, 0b11, 0, 1);
  INSN(smultb, 0b11, 1, 0);
  INSN(smultt, 0b11, 1, 1);
#undef INSN

// For Extra load/store instructions, see load/store section
// For Synchronization primitives, see load/store section

// MSR(immediate), and hints
#define INSN(NAME, decode)                                                           \
  void NAME(Condition cond = C_DFLT) {                                               \
    starti;                                                                          \
    f(cond, 31, 28), f(0b001100100000, 27, 16), f(0b11110000, 15, 8);                \
    f(decode, 7, 0);                                                                 \
  }
  INSN(nop,   0b000);
  INSN(yield, 0b001);
  INSN(wfe,   0b010);
  INSN(wfi,   0b011);
  INSN(sev,   0b100);
  void dbg(int dbg_hint, Condition cond = C_DFLT) {
    f(cond, 31, 28), f(0b001100100000, 27, 16), f(0b11110000, 15, 8);
    f(0b1111, 7, 4); f(dbg_hint, 3, 0);
  }
#undef INSN

  //TODO Misc instructions
  void bkpt(unsigned imm) {
    starti;
    f(AL, 31, 28), f(0b00010010, 27, 20);
    f(imm >> 4, 19, 8), f(0b0111, 7, 4), f(imm & 0xf, 3, 0);
  }
  void hlt(unsigned imm) {
    bkpt(imm);
    // FIXME This seemed like the best option!
  }

  // Load/store register (all modes)
  void load_store_instr(Register Rt, const Address &adr, int op, int op2, int a, int b,
                        Condition cond) {
    starti;
    f(cond, 31, 28), f(op, 27, 25), f(a, 22), f(b, 20);
    if(op2 >= 0)
      f(op2, 7, 4);
    //Destination
    rf(Rt, 12);
    adr.encode(current, code_section(), pc());
  }

  bool encodeable(int decode, address dest) {
    long offset = dest - pc();
    switch(decode) {
      case 0b010:
        // LDR, LDRB, STR, STRB
        return uabs(offset) < ((1 << 12) - 1);
      case 0b000:
        //LDRD, LDRH, LDRSB, LDRSH, STRH, STRD
        return uabs(offset) < ((1 << 8) - 1);
      default:
        ShouldNotReachHere();
    }
    return false;
  }



#define INSN_INT(NAME, op, op2, a, b, isload)                                        \
  void NAME(Register Rt, address dest, Condition cond = C_DFLT) {                    \
    if(encodeable(op, dest)) { /* Plan A */                                          \
      long offset = dest - pc();                                                     \
      NAME(Rt, Address(r15_pc, offset), cond);                                       \
    } else if(isload){ /* Plan B */                                                  \
      /* TODO check we don't have to relocate this*/                                 \
      mov_immediate(Rt, (u_int32_t)dest, cond, false);                               \
      NAME(Rt, Address(Rt, 0), cond);                                                \
    } else { /* There is no plan C */                                                \
      ShouldNotReachHere();                                                          \
    }                                                                                \
  }                                                                                  \
  void NAME(Register Rt, address dest, relocInfo::relocType rtype,                   \
            Condition cond = C_DFLT) {                                               \
    guarantee(rtype == relocInfo::internal_word_type,                                \
              "only internal_word_type relocs make sense here");                     \
    NAME(Rt, InternalAddress(dest), cond);                                           \
  }                                                                                  \
  void NAME(Register Rt, Label &L, Condition cond = C_DFLT) {                        \
    wrap_label(Rt, L, cond, &Assembler::NAME);                                       \
  }

#define INSN(NAME, op, op2, a, b, isload)                                            \
  void NAME(Register Rt, const Address &adr, Condition cond = C_DFLT) {              \
    load_store_instr(Rt, adr, op, op2, a, b, cond);                                  \
  }                                                                                  \
  INSN_INT(NAME, op, op2, a, b, isload);
  INSN(ldr,   0b010,     -1, 0, 1, 1);
  INSN(ldrb,  0b010,     -1, 1, 1, 1);

  INSN(ldrsb, 0b000, 0b1101, 0, 1, 1);
  INSN(ldrh,  0b000, 0b1011, 0, 1, 1);
  INSN(ldrsh, 0b000, 0b1111, 0, 1, 1);

  INSN(str,   0b010,     -1, 0, 0, 0);
  INSN(strb,  0b010,     -1, 1, 0, 0);
  INSN(strh,  0b000, 0b1011, 0, 0, 0);
  //Note LDRD & STRD are defined with the load/store multiple instructions

  //TODO Need to introduce ldrsb ldrsh - then check that the encoding works properly!
#undef INSN


  //Synchronization primitives
  void sync_instr(int decode, Register Ra, Register Rb, Register Rc, Register Rd,
             Condition cond) {
    starti;
    f(cond, 31, 28), f(0b0001, 27, 24), f(decode, 23, 20), rf(Ra, 16), rf(Rb, 12);
    rf(Rc, 8), f(0b1001, 7, 4), rf(Rd, 0);
  }

#define INSN(NAME, decode)                                                           \
  void NAME(Register Rd, Register Rt, Register Rn, Condition cond = C_DFLT) {        \
    assert(r15_pc != Rn, "Unpredictable");                                           \
    sync_instr(decode, Rn, Rd, ONES_ADDR_REG, Rt, cond);                             \
  }
  INSN( strex, 0b1000);
  INSN(strexd, 0b1010);
  INSN(strexb, 0b1100);
  INSN(strexh, 0b1110);
#undef INSN

#define INSN(NAME, decode)                                                           \
  void NAME(Register Rt, Register Rn, Condition cond = C_DFLT) {                     \
    assert(r15_pc != Rn, "Unpredictable");                                           \
    sync_instr(decode, Rn, Rt, ONES_ADDR_REG, ONES_ADDR_REG, cond);                  \
  }
  INSN(ldrex,  0b1001);
  INSN(ldrexd, 0b1011);
  INSN(ldrexb, 0b1101);
  INSN(ldrexh, 0b1111);
#undef INSN

// Media instructions
void media_instr(int decode, int decode2, Condition cond) {
  f(cond, 31, 28), f(0b011, 27, 25), f(decode, 24, 20);
  f(decode2, 7, 5), f(1, 4);
}

#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {        \
    starti;                                                                          \
    media_instr(0b00000 | decode, decode2, cond);                                    \
    rf(Rn, 16), rf(Rd, 12), f(0b1111, 11, 8), rf(Rm, 0);                             \
  }
  INSN(sadd16, 0b01, 0b000);
  INSN(sasx,   0b01, 0b001);
  INSN(ssax,   0b01, 0b010);
  INSN(ssub16, 0b01, 0b011);
  INSN(sadd8,  0b01, 0b100);
  INSN(ssub8,  0b01, 0b111);
  //Saturating
  INSN(qadd16, 0b10, 0b000);
  INSN(qasx,   0b10, 0b001);
  INSN(qsax,   0b10, 0b010);
  INSN(qsub16, 0b10, 0b011);
  INSN(qadd8,  0b10, 0b100);
  INSN(qsub8,  0b10, 0b111);
  //Halving
  INSN(shadd16, 0b11, 0b000);
  INSN(shasx,   0b11, 0b001);
  INSN(shsax,   0b11, 0b010);
  INSN(shsub16, 0b11, 0b011);
  INSN(shadd8,  0b11, 0b100);
  INSN(shsub8,  0b11, 0b111);

  //Now unsigned
  INSN(uadd16, 0b101, 0b000);
  INSN(uasx,   0b101, 0b001);
  INSN(usax,   0b101, 0b010);
  INSN(usub16, 0b101, 0b011);
  INSN(uadd8,  0b101, 0b100);
  INSN(usub8,  0b101, 0b111);
  //Saturating
  INSN(uqadd16, 0b110, 0b000);
  INSN(uqasx,   0b110, 0b001);
  INSN(uqsax,   0b110, 0b010);
  INSN(uqsub16, 0b110, 0b011);
  INSN(uqadd8,  0b110, 0b100);
  INSN(uqsub8,  0b110, 0b111);
  //Halving
  INSN(uhadd16, 0b111, 0b000);
  INSN(uhasx,   0b111, 0b001);
  INSN(uhsax,   0b111, 0b010);
  INSN(uhsub16, 0b111, 0b011);
  INSN(uhadd8,  0b111, 0b100);
  INSN(uhsub8,  0b111, 0b111);
#undef INSN

//Packing, unpacking, saturation and reversal
// Note rotation can only be one of ROR #0 ROR #8 ROR #16 ROR #24
void extend_instr(int decode, int decode2, int decode3, Register Rd, Register Rn,
                  Register Rm, shift_op shift, Condition cond) {
  starti;
  assert(0 == shift.shift() ||
         shift_op::ROR == shift.kind(), "Only ROR may be used for op");
  // All zero shifts are mapped to LSL #0
  int shift_enc = 0;
  switch(shift.shift()) {
    case 0:                 break;
    case 8:  shift_enc = 1; break;
    case 16: shift_enc = 2; break;
    case 24: shift_enc = 3; break;
    default: assert(false, "Invalid shift quantity");
  }
  media_instr(0b01000 | decode, decode2, cond);
  rf(Rn, 16), rf(Rd, 12), f(shift_enc, 11, 10), f(decode3, 9, 8), rf(Rm, 0);
}

#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rn, Register Rm, shift_op shift = ::ror(),         \
            Condition cond = C_DFLT) {                                               \
    assert(0xf != Rn->encoding_nocheck(), "Rn = pc makes different instruction");    \
    extend_instr(decode, decode2, 0b00, Rd, Rn, Rm, shift, cond);                    \
  }
  INSN(sxtab16, 0b000, 0b011);
  INSN(sxtab,   0b010, 0b011);
  INSN(sxtah,   0b011, 0b011);
  INSN(uxtab16, 0b100, 0b011);
  INSN(uxtab,   0b110, 0b011);
  INSN(uxtah,   0b111, 0b011);
#undef INSN

#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rm, shift_op shift = ::ror(),                      \
            Condition cond = C_DFLT) {                                               \
    extend_instr(decode, decode2, 0b00, Rd, ONES_ADDR_REG, Rm, shift, cond);         \
  }
  INSN(sxtb16, 0b000, 0b011);
  INSN(sxtb,   0b010, 0b011);
  INSN(sxth,   0b011, 0b011);
  INSN(uxtb16, 0b100, 0b011);
  INSN(uxtb,   0b110, 0b011);
  INSN(uxth,   0b111, 0b011);
#undef INSN

  //Reverse instructions
#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rm, Condition cond = C_DFLT) {                     \
    extend_instr(decode, decode2, 0b11, Rd, ONES_ADDR_REG, Rm, ::ror(24), cond);     \
  }
  INSN(rev,   0b011, 0b001);
  INSN(rev16, 0b011, 0b101);
  INSN(rbit,  0b111, 0b001);
  INSN(revsh, 0b111, 0b101);
#undef INSN

// Signed multiply, signed and unsigned divide
#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rn, Register Rm, Condition cond = C_DFLT) {        \
    starti;                                                                          \
    media_instr(0b10000 | decode, decode2, cond);                                    \
    rf(Rd, 16), f(0b1111, 15, 12), rf(Rm, 8), rf(Rn, 0);                             \
  }
  INSN(sdiv, 0b001, 0b000);
  INSN(udiv, 0b011, 0b000);
  //TODO ALL THE REST!
#undef INSN

// Remainder of things
//TODO USAD8
#define INSN(NAME, decode, decode2)                                                  \
  void NAME(Register Rd, Register Rn, int lsb, int width,                            \
            Condition cond = C_DFLT) {                                               \
    starti;                                                                          \
    assert(lsb >= 0 && lsb < 32, "lsb out of range");                                \
    assert(width > 0 && width <= 32 - lsb, "width out of range");                    \
    media_instr(decode, decode2, cond);                                              \
    f(width - 1, 20, 16), rf(Rd, 12), f(lsb, 11, 7), rf(Rn, 0);                      \
  }
  INSN(sbfx, 0b11010, 0b010);
  INSN(ubfx, 0b11110, 0b010);
#undef INSN

void bfi(Register Rd, Register Rn, int lsb, int width, Condition cond = C_DFLT) {
  int msb = lsb + width - 1;
  assert(lsb >= 0 && lsb < 32, "lsb out of range");
  assert(msb < 32 && msb >= lsb, "width out of range");
  starti;
  media_instr(0b11100, 0b000, cond);
  f(msb, 20, 16), rf(Rd, 12), f(lsb, 11, 7), rf(Rn, 0);
}

void bfc(Register Rd, int lsb, int width, Condition cond = C_DFLT) {
  int msb = lsb + width - 1;
  assert(lsb >= 0 && lsb < 32, "lsb out of range");
  assert(msb < 32 && msb >= lsb, "width out of range");
  starti;
  media_instr(0b11100, 0b000, cond);
  f(msb, 20, 16), rf(Rd, 12), f(lsb, 11, 7), f(0b1111, 3, 0);
}

//Branch, branch with link, and block data transfer

void block_imm_instr(int decode, int w, Register Rn, unsigned regset,
                     Condition cond) {
  starti;
  f(cond, 31, 28), f(0b10, 27, 26), f(decode | (w << 1), 25, 20);
  rf(Rn, 16), f(regset, 15, 0);
}
#define INSN(NAME, decode)                                                           \
  void NAME(Register Rn, unsigned regset, bool wb = true, Condition cond = C_DFLT) { \
    block_imm_instr(decode, wb, Rn, regset, cond);                                   \
  }
  INSN(stmda, 0b000000);
  INSN(stmed, 0b000000);

  INSN(ldmda, 0b000001);
  INSN(ldmfa, 0b000001);

  //INSN(stm,   0b001000);
  INSN(stmia, 0b001000);
  INSN(stmea, 0b001000);

  //INSN(ldm,   0b001001);
  INSN(ldmia, 0b001001);
  INSN(ldmfd, 0b001001);

  INSN(stmdb, 0b010000);
  INSN(stmfd, 0b010000);

  INSN(ldmdb, 0b010001);
  INSN(ldmea, 0b010001);

  INSN(stmib, 0b011000);
  INSN(stmfa, 0b011000);

  INSN(ldmib, 0b011001);
  INSN(ldmed, 0b011001);
#undef INSN

unsigned count_bits(unsigned val);
bool can_ldst_multiple( unsigned regset, const Address& adr);

//NOTE!! Have repurposed stm and ldm for auto dispatch instructions
#define INSN(NAME, PREFIX)                                                           \
  void NAME(unsigned regset, const Address& adr, Condition cond = C_DFLT) {          \
    assert(can_ldst_multiple(regset, adr), "Can't do anything with this!");          \
    int offset = adr.offset();                                                       \
    switch(adr.get_wb_mode()) {                                                      \
      case Address::pre:                                                             \
        if(offset > 0) PREFIX##mib(adr.base(), regset, true, cond);                  \
        else           PREFIX##mdb(adr.base(), regset, true, cond);                  \
        break;                                                                       \
      case Address::post:                                                            \
        if(offset > 0) PREFIX##mia(adr.base(), regset, true, cond);                  \
        else           PREFIX##mda(adr.base(), regset, offset != 0, cond);           \
        break;                                                                       \
      case Address::off:                                                             \
        if(offset > 0)   PREFIX##mib(adr.base(), regset, false, cond);               \
        else if(!offset) PREFIX##mia(adr.base(), regset, false, cond);               \
        else             PREFIX##mdb(adr.base(), regset, false, cond);               \
        break;                                                                       \
      default:                                                                       \
        ShouldNotReachHere();                                                        \
    }                                                                                \
  }
  INSN(ldm, ld);
  INSN(stm, st);
#undef INSN

//Made push and pop operate on full descending stacks
#define INSN(NAME, CNAME)                                                            \
  inline void NAME(unsigned regset, Condition cond = C_DFLT) {                       \
    CNAME(r13, regset, true, cond);                                                  \
  }
  INSN(pop,  ldmia);
  INSN(push, stmdb);
#undef INSN

 private:
  void double_ldst_failed_dispatch(Register Rt, Register Rt2, const Address& adr,
                            void (Assembler::* mul)(unsigned, const Address&, Condition),
                            void (Assembler::* sgl)(Register, const Address&, Condition),
                            Condition cond);
 public:

#define INSN(NAME, PREFIX, op, op2, a, b, isload)                                    \
  void NAME(Register Rt, Register Rt2, const Address& adr,                           \
            Condition cond = C_DFLT) {                                               \
    if(0 == Rt->encoding_nocheck() % 2 &&                                            \
       (Rt->encoding_nocheck() + 1 == Rt2->encoding_nocheck())) {                    \
      /* Good to go with a ldrd/strd */                                              \
      load_store_instr(Rt, adr, op, op2, a, b, cond);                                \
    } else {                                                                         \
      double_ldst_failed_dispatch(Rt, Rt2, adr, &Assembler::PREFIX##m,               \
                                  &Assembler::PREFIX##r, cond);                      \
    }                                                                                \
  }                                                                                  \
  void NAME(Register Rt, const Address& adr, Condition cond = C_DFLT) {              \
    NAME(Rt, (Register)(Rt + 1), adr, cond);                                         \
  }                                                                                  \
  INSN_INT(NAME, op, op2, a, b, isload);

  INSN(ldrd, ld, 0b000, 0b1101, 0, 0, 1);
  INSN(strd, st, 0b000, 0b1111, 0, 0, 0);
#undef INSN
#undef INSN_INT

  // Branches

  // For immediate branches:
  // The maximum range of a branch is fixed for the aarch32
  // architecture.  In debug mode we shrink it in order to test
  // trampolines, but not so small that branches in the interpreter
  // are out of range.
  static const unsigned long branch_range = NOT_DEBUG(32 * M) DEBUG_ONLY(2 * M);
  static bool reachable_from_branch_at(address branch, address target) {
    return uabs(target - branch) < branch_range;
  }

  void branch_imm_instr(int decode, address dest, Condition cond) {
    starti;
    // Correct PC for as it will be when executing this instruction
    int offset = (dest - (pc() + 8)) >> 2;
    assert(reachable_from_branch_at(pc(), dest), "branch target unreachable");
    f(cond, 31, 28), f(decode, 27, 24), sf(offset, 23, 0);
  }

  void branch_reg_instr(int decode, Register Rm, Condition cond) {
    starti;
    f(cond, 31, 28), f(0b00010010, 27, 20);
    f(0b111111111111, 19, 8), f(decode, 7, 4), rf(Rm, 0);
  }

#define INSN(NAME, decode_imm, decode_reg)                                           \
  void NAME(Register Rm, Condition cond = C_DFLT) {                                  \
    branch_reg_instr(decode_reg, Rm, cond);                                          \
  }                                                                                  \
  void NAME(address dest, Condition cond = C_DFLT) {                                 \
    branch_imm_instr(decode_imm, dest, cond);                                        \
  }                                                                                  \
  void NAME(Label &L, Condition cond = C_DFLT) {                                     \
    wrap_label(L, cond, &Assembler::NAME);                                           \
  }                                                                                  \
  void NAME(const Address &dest, Condition cond = C_DFLT) {                          \
    code_section()->relocate(pc(), dest.rspec());                                    \
    NAME(dest.target(), cond);                                                       \
  }
  //TODO assert type of address
  INSN(b,  0b1010, 0b0001); // B & BX
  INSN(bl, 0b1011, 0b0011); // BL & BLX
#undef INSN


//TODO Coprocessor instructions, and Supervisor Call


// Unconditional Instructions
  enum barrier {OSHST = 0b0010, OSH,
                NSHST = 0b0110, NSH,
                ISHST = 0b1010, ISH,
                   ST = 0b1110, SY};

  void sync_instr(int decode, enum barrier option) {
    starti;
    f(0b11110, 31, 27), f(0b1010111, 26, 20), f(0b111111110000, 19, 8);
    f(decode, 7, 4), f(option, 3, 0);
  }
  void clrex() {
    sync_instr(0b0001, SY);
  }
  void dsb(enum barrier option) {
    sync_instr(0b0100, option);
  }
  void dmb(enum barrier option) {
    sync_instr(0b0100, option);
  }
  void bkpt();
  void isb() {
    sync_instr(0b0110, SY);
  }

  // And the relevant instructions for ARMv6.

  // MCR<c> <coproc>, <opc1>, <Rt>, <CRn>, <CRm>{, <opc2>}
  void mcr(int cpc_dex, int opc1, Register Rt, int cpc_reg_dex1,
           int cpc_reg_dex2, int opc2, Condition cond = C_DFLT) {
    starti;
    f(cond, 31, 28), f(0b1110, 27, 24), f(opc1, 23, 21), f(0, 20);
    f(cpc_reg_dex1, 19, 16), rf(Rt, 12), f(cpc_dex, 11, 8);
    f(opc2, 7, 5), f(1, 4), f(cpc_reg_dex2, 3, 0);
  }

  // These instructions do not read the value of the register passed,
  // can be any. Chosen r0.
  void cp15dmb(Condition cond = C_DFLT) {
    mcr(15, 0, r0, 7, 10, 5, cond);
  }

  void cp15dsb(Condition cond = C_DFLT) {
    mcr(15, 0, r0, 7, 10, 4, cond);
  }

  void cp15isb(Condition cond = C_DFLT) {
    mcr(15, 0, r0, 7, 5, 4, cond);
  }

  enum Membar_mask_bits {
    // We can use ISH for a barrier because the ARM ARM says "This
    // architecture assumes that all Processing Elements that use the
    // same operating system or hypervisor are in the same Inner
    // Shareable shareability domain."
    StoreStore = ISHST,
    LoadStore  = ISH, //ISHLD, Changed to
    LoadLoad   = ISH, //ISHLD,
    StoreLoad  = ISH,
    AnyAny     = ISH
  };

// Floating point operations

// A note on floating point operations:
// As we're not short on floating point registers if we're given a bit D which
// in 64 bit mode allows access to the higher double support registers or in
// single precision mode allow access to the odd number registers. We always
// clear the bit reducing the number of single and double precision registers
// available but simplifiying encoding.
// the fp rencode function can be easily modified however to do something
// a bit snazzier!

enum fpscr_cond { FP_EQ = 0b0110 << 28,
                  FP_LT = 0b1000 << 28,
                  FP_GT = 0b0010 << 28,
                  FP_UN = 0b0011 << 28,
                  FP_MASK = 0b1111 << 28 };

  void fp_instr_base(bool is64bit, Condition cond) {
    f(cond, 31, 28), f(0b1110, 27, 24), f(0b101, 11, 9), f(is64bit, 8), f(0, 4);
  }

  void fp_rencode(FloatRegister reg, bool is64bit, int base, int bit) {
    int reg_val = reg->encoding_nocheck();
    /*if(is64bit) {
      f( reg_val >> 1, base + 3, base);
      f( reg_val & 1, bit);
    } else {
      f( reg_val & 0xf, base + 3, base);
      f( reg_val >> 4, bit);
    }*/
    f(reg_val & 0xf, base + 3, base);
    f(0, bit);
    // Force alignment to double registers
    // for all ops.
  }

  void fp_instr(int decode, int op, bool is64bit, FloatRegister Rd, FloatRegister Rn,
                FloatRegister Rm, Condition cond) {
    fp_instr_base(is64bit, cond);
    f(decode, 23, 20), f(op, 6);
    // Register encoding is a bit involved
    fp_rencode(Rn, is64bit, 16, 7);
    fp_rencode(Rd, is64bit, 12, 22);
    fp_rencode(Rm, is64bit,  0, 5);
  }

#define INSN(NAME, decode, op, is64bit)                                              \
  void NAME(FloatRegister Rd, FloatRegister Rn, FloatRegister Rm,                    \
            Condition cond = C_DFLT) {                                               \
    starti;                                                                          \
    fp_instr(decode, op, is64bit, Rd, Rn, Rm, cond);                                 \
  }
  INSN(vmla_f32,  0b0000, 0, 0);
  INSN(vmla_f64,  0b0000, 0, 1);
  INSN(vmls_f32,  0b0000, 1, 0);
  INSN(vmls_f64,  0b0000, 1, 1);

  INSN(vnmla_f32, 0b0001, 1, 0);
  INSN(vnmla_f64, 0b0001, 1, 1);
  INSN(vnmls_f32, 0b0001, 0, 0);
  INSN(vnmls_f64, 0b0001, 0, 1);
  INSN(vnmul_f32, 0b0010, 1, 0);
  INSN(vnmul_f64, 0b0010, 1, 1);
  INSN(vmul_f32,  0b0010, 0, 0);
  INSN(vmul_f64,  0b0010, 0, 1);

  INSN(vadd_f32,  0b0011, 0, 0);
  INSN(vadd_f64,  0b0011, 0, 1);
  INSN(vsub_f32,  0b0011, 1, 0);
  INSN(vsub_f64,  0b0011, 1, 1);

  INSN(vdiv_f32,  0b1000, 0, 0);
  INSN(vdiv_f64,  0b1000, 0, 1);

  INSN(vfnma_f32, 0b1001, 1, 0);
  INSN(vfnma_f64, 0b1001, 1, 1);
  INSN(vfnms_f32, 0b1001, 0, 0);
  INSN(vfnms_f64, 0b1001, 0, 1);

  INSN(vfma_f32,  0b1010, 0, 0);
  INSN(vfma_f64,  0b1010, 0, 1);
  INSN(vfms_f32,  0b1010, 1, 0);
  INSN(vfms_f64,  0b1010, 1, 1);
#undef INSN


  void vmov_imm(FloatRegister Rd, unsigned imm, bool is64bit, Condition cond);
  void vmov_imm_zero(FloatRegister Rd, bool is64bit, Condition cond);

#define INSN(NAME, is64bit, ntype)                                                   \
  unsigned encode_##ntype##_fp_imm(ntype imm_f);                                     \
  void NAME(FloatRegister Rd, ntype imm, Condition cond = C_DFLT) {                  \
    if(0.0 == imm) vmov_imm_zero(Rd, is64bit, cond);                                 \
    else           vmov_imm(Rd, encode_##ntype##_fp_imm(imm), is64bit, cond);        \
  }
  INSN(vmov_f32, false, float);
  INSN(vmov_f64, true, double);
#undef INSN

#define INSN(NAME, decode, op, is64bit)                                              \
  void NAME(FloatRegister Rd, FloatRegister Rm, Condition cond = C_DFLT) {           \
    starti;                                                                          \
    fp_instr_base(is64bit, cond);                                                    \
    f(0b1011, 23, 20), f(decode, 19, 16), f(op, 7, 6), f(0b00, 5, 4);                \
    fp_rencode(Rd, is64bit, 12, 22);                                                 \
    fp_rencode(Rm, is64bit, 0, 5);                                                   \
  }
  INSN(vmov_f32,  0b0000, 0b01, 0);
  INSN(vmov_f64,  0b0000, 0b01, 1);
  INSN(vabs_f32,  0b0000, 0b11, 0);
  INSN(vabs_f64,  0b0000, 0b11, 1);
  INSN(vneg_f32,  0b0001, 0b01, 0);
  INSN(vneg_f64,  0b0001, 0b01, 1);
  INSN(vsqrt_f32, 0b0001, 0b11, 0);
  INSN(vsqrt_f64, 0b0001, 0b11, 1);
#undef INSN

//ARM -> FP, FP -> ARM
// NOTE - Have only implemented the double precision variant as only operating on
// double registers - can still be used to copy single precision
void vmov64_instr_base(FloatRegister Rm, Register Rt, Register Rt2, int op,
                       Condition cond) {
  starti;
  f(cond, 31, 28), f(0b1100010, 27, 21), f(op, 20);
  rf(Rt2, 16), rf(Rt, 12), f(0b101100, 11, 6), f(1, 4);
  fp_rencode(Rm, true, 0, 5);
}

void vmov_f64(FloatRegister Rm, Register Rt, Register Rt2, Condition cond = C_DFLT) {
  vmov64_instr_base(Rm, Rt, Rt2, 0, cond);
}
void vmov_f64(Register Rt, Register Rt2, FloatRegister Rm, Condition cond = C_DFLT) {
  vmov64_instr_base(Rm, Rt, Rt2, 1, cond);
}

void vmov_f32(FloatRegister Rn, Register Rt, Condition cond = C_DFLT) {
  starti;
  fp_instr_base(false, cond);
  f(0b000, 23, 21), f(0, 20);
  rf(Rt, 12), f(0b101000010000, 11, 0);
  fp_rencode(Rn, false, 16, 7);
}
void vmov_f32(Register Rt, FloatRegister Rn, Condition cond = C_DFLT) {
  starti;
  fp_instr_base(false, cond);
  f(0b000, 23, 21), f(1, 20);
  rf(Rt, 12), f(0b101000010000, 11, 0);
  fp_rencode(Rn, false, 16, 7);
}

// Floating-point comparison
#define INSN(NAME, E, is64bit)                                                       \
  void NAME(FloatRegister Rd, int imm, Condition cond = C_DFLT) {                    \
    assert(0 == imm, "vector compare can only be with another vector or zero");      \
    starti;                                                                          \
    fp_instr_base(is64bit, cond);                                                    \
    f(0b10110101, 23, 16), f(E, 7), f(0b1000000, 6, 0);                              \
    fp_rencode(Rd, is64bit, 12, 22);                                                 \
  }                                                                                  \
  void NAME(FloatRegister Vd, FloatRegister Vm, Condition cond = C_DFLT) {           \
    starti;                                                                          \
    fp_instr_base(is64bit, cond);                                                    \
    f(0b10110100, 23, 16), f(E, 7), f(1, 6), f(0, 4);                                \
    fp_rencode(Vd, is64bit, 12, 22), fp_rencode(Vm, is64bit, 0, 5);                  \
  }
  INSN(vcmpe_f64, 1, 1);
  INSN(vcmpe_f32, 1, 0);
  INSN( vcmp_f64, 0, 1);
  INSN( vcmp_f32, 0, 0);
#undef INSN

//Move FPSCR to ARM register
void vmrs(Register Rt, Condition cond = C_DFLT) {
  starti;
  f(cond, 31, 28), f(0b111011110001, 27, 16), rf(Rt, 12), f(0b101000010000, 11, 0);
}

//Move ARM register to FPSCR
void vmsr(Register Rt, Condition cond = C_DFLT) {
  starti;
  f(cond, 31, 28), f(0b111011100001, 27, 16), rf(Rt, 12), f(0b101000010000, 11, 0);
}

// TODO These instructions use round towards zero mode. It is possible
//  for the mode to be taken from the FPSCR however it doesn't do it currently
#define INSN(NAME, decode2, b19, op, is64bit)                                        \
  void NAME(FloatRegister Rd, FloatRegister Rm, Condition cond = C_DFLT) {           \
    starti;                                                                          \
    fp_instr_base(is64bit, cond);                                                    \
    f(0b1011, 23, 20), f(b19, 19), f(decode2, 18, 16), f(op, 7), f(0b100, 6, 4);     \
    fp_rencode(Rd, is64bit, 12, 22);                                                 \
    fp_rencode(Rm, is64bit, 0, 5);                                                   \
  }
  INSN(vcvt_s32_f32, 0b101, 1, 1, 0);
  INSN(vcvt_s32_f64, 0b101, 1, 1, 1);
  INSN(vcvt_u32_f32, 0b100, 1, 1, 0);
  INSN(vcvt_u32_f64, 0b100, 1, 1, 1);

  INSN(vcvt_f64_s32, 0b000, 1, 1, 1);
  INSN(vcvt_f64_u32, 0b000, 1, 0, 1);
  INSN(vcvt_f32_s32, 0b000, 1, 1, 0);
  INSN(vcvt_f32_u32, 0b000, 1, 0, 0);

  INSN(vcvt_f32_f64, 0b111, 0, 1, 1);
  INSN(vcvt_f64_f32, 0b111, 0, 1, 0);
#undef INSN

//Vector load/store
 private:
  void fp_ldst_instr(int decode, bool is64bit, const Address& adr, Condition cond);
 public:

#define INSN(NAME, decode, is64bit)                                                  \
  void NAME(FloatRegister Vd, const Address &adr, Condition cond = C_DFLT) {         \
    starti;                                                                          \
    fp_ldst_instr(decode, is64bit, adr, cond);                                       \
    fp_rencode(Vd, is64bit, 12, 22);                                                 \
  }                                                                                  \
  void NAME(FloatRegister Vd, address dest, Condition cond = C_DFLT) {               \
    long offset = dest - pc();                                                       \
    NAME(Vd, Address(r15_pc, offset), cond);                                         \
  }                                                                                  \
  void NAME(FloatRegister Vd, address dest, relocInfo::relocType rtype,              \
            Condition cond = C_DFLT) {                                               \
    guarantee(rtype == relocInfo::internal_word_type,                                \
              "only internal_word_type relocs make sense here");                     \
    NAME(Vd, InternalAddress(dest), cond);                                           \
  }                                                                                  \
  void NAME(FloatRegister Vd, Label &L, Condition cond = C_DFLT) {                   \
    wrap_label(Vd, L, cond, &Assembler::NAME);                                       \
  }
  INSN(vstr_f64, 0b10000, 1);
  INSN(vstr_f32, 0b10000, 0);
  INSN(vldr_f64, 0b10001, 1);
  INSN(vldr_f32, 0b10001, 0);
#undef INSN

// This is a hack. Because of the way floating point registers have been used we reach a
// tricky situation when calling out the C land that doesn't expect all single precision
// arguments to be on a double boundary.
  void fp_rencode_real(int reg_val, bool is64bit, int base, int bit) {
    if(is64bit) {
      f( reg_val & 0xf, base + 3, base);
      f( reg_val >> 4, bit);
    } else {
      f( reg_val >> 1, base + 3, base);
      f( reg_val & 1, bit);
    }
  }

  void vldr_f32_real(int reg_val, const Address &adr, Condition cond = C_DFLT) {
    starti;
    fp_ldst_instr(0b10001, false, adr, cond);
    fp_rencode_real(reg_val, false, 12, 22);
  }

 private:
  enum fp_mode { ia_wb, ia, db_wb };
  void fp_ldst_mul(Register Rn, int regset, bool load, bool is64bit, enum fp_mode mode, Condition cond);
 public:
// A note here - load/store multiple only works on consecutive vector registers.
// The 32bit version therefore works on consecutive 32bit float registers.
// We are not using consecutive registers for single precision and so for single
// precision storage only one register is allowed.
#define INSN(NAME, EXT, is64bit, load)                                               \
  inline void NAME##ia##EXT(Register Rn, unsigned regset, bool wb = true,            \
                            Condition cond = C_DFLT) {                               \
    fp_ldst_mul(Rn, regset, load, is64bit,                                           \
                (enum fp_mode)( ia_wb + ( wb?0:1 )), cond);                          \
  }                                                                                  \
  inline void NAME##db##EXT(Register Rn, unsigned regset, Condition cond = C_DFLT) { \
    fp_ldst_mul(Rn, regset, load, is64bit, db_wb, cond);                             \
  }
  INSN(vldm, _f32, 0, 1);
  INSN(vldm, _f64, 1, 1);
  INSN(vstm, _f32, 0, 0);
  INSN(vstm, _f64, 1, 0);
#undef INSN

#undef ZERO_ADDR_REG
#undef ONES_ADDR_REG


  Assembler(CodeBuffer* code) : AbstractAssembler(code) {}

  virtual RegisterOrConstant delayed_value_impl(intptr_t* delayed_value_addr,
                                                Register tmp,
                                                int offset) {
    ShouldNotCallThis();
    return RegisterOrConstant();
  }

  // Stack overflow checking
  virtual void bang_stack_with_offset(int offset);

  //static bool operand_valid_for_logical_immediate(bool is32, uint64_t imm);
  static bool operand_valid_for_add_sub_immediate(long imm);
  //static bool operand_valid_for_float_immediate(double imm);
  static bool operand_valid_for_mov_immediate(u_int32_t imm, bool s);

  void emit_data64(jlong data, relocInfo::relocType rtype, int format = 0);
  void emit_data64(jlong data, RelocationHolder const& rspec, int format = 0);
};

inline Assembler::Membar_mask_bits operator|(Assembler::Membar_mask_bits a,
                                             Assembler::Membar_mask_bits b) {
  return Assembler::Membar_mask_bits(unsigned(a)|unsigned(b));
}

Instruction_aarch32::~Instruction_aarch32() {
  assem->emit();
}

#undef starti

// Invert a condition
inline const Assembler::Condition operator~(const Assembler::Condition cond) {
  return Assembler::Condition(int(cond) ^ 1);
}

class BiasedLockingCounters;

extern "C" void das(uint64_t start, int len);

#endif // CPU_AARCH32_VM_ASSEMBLER_AARCH32_HPP
