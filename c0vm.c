/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2022                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S;   /* Operand stack of C0 values */
  ubyte *P;        /* Function body */
  size_t pc;       /* Program counter */
  c0_value *V;     /* The local variables */
};

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); /* Operand stack of C0 values */
  ubyte *P = bc0 -> function_pool -> code;      /* Array of bytes that make up the current function */
  size_t pc = 0;     /* Current location within the current byte array P */
  c0_value *V = xcalloc(sizeof(c0_value), bc0 -> function_pool -> num_vars);   /* Local variables (you won't need this till Task 2) */
  (void) V;      // silences compilation errors about V being currently unused

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack = stack_new();
  (void) callStack; // silences compilation errors about callStack being currently unused

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: {
      pc++;
      c0_value first = c0v_pop(S);
      c0_value second = c0v_pop(S);
      c0v_push(S, first);
      c0v_push(S, second);
      break;
    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to revise it further when you write INVOKESTATIC. */

    case RETURN: {
      if (stack_empty(callStack)) {
      int retval = val2int(c0v_pop(S));
      assert(c0v_stack_empty(S));
      c0v_stack_free(S);
      free(V);
      stack_free(callStack, NULL);
// Another way to print only in DEBUG mode
IF_DEBUG(fprintf(stderr, "Returning %d from execute()\n", retval));
      // Free everything before returning from the execute function!
      return retval;
      } else {
        c0_value output = c0v_pop(S);
        c0v_stack_free(S);
        free(V);
        frame *new = (frame*) pop(callStack);
        S = new -> S;   
        P = new -> P;       
        pc = new -> pc;      
        V = new -> V;
        c0v_push(S, output);
        free(new);
      }
      break;
    }

    /* Arithmetic and Logical operations */

    case IADD: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(first + second));
      break;
    }

    case ISUB: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(second - first));
      break;
    }

    case IMUL: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(first * second));
      break;
    }

    case IDIV: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if (first == (int32_t)0 || (second == (int32_t)0x80000000 
      && first == (int32_t)-1)) { 
        c0_arith_error("Invalid Division!"); }
      c0v_push(S, int2val(second / first));
      break;
    }

    case IREM: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if (first == (int32_t)0 || (second == (int32_t)0x80000000 
      && first == (int32_t)-1)) {
         c0_arith_error("Invalid Mod!"); }
      c0v_push(S, int2val(second % first));
      break;
    }

    case IAND: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(first & second));
      break;
    }
  
    case IOR: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(first | second));
      break;
    }

    case IXOR: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      c0v_push(S, int2val(first ^ second));
      break;
    }

    case ISHR: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if (first < 0 || first >= 32) { c0_arith_error("Invalid Shift!"); }
      c0v_push(S, int2val(second >> first));
      break;
    }

    case ISHL: {
      pc++;
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if (first < 0 || first >= 32) { c0_arith_error("Invalid Shift!"); }
      c0v_push(S, int2val(second << first));
      break;
    }
    /* Pushing constants */

    case BIPUSH: {
      pc++;
      int32_t x = (int8_t) P[pc];
      c0v_push(S, int2val(x));
      pc++;
      break;
    }

    case ILDC: {
      pc++;
      uint32_t c1 = (uint32_t) P[pc];
      pc++;
      uint32_t c2 = (uint32_t) P[pc];
      pc++;
      int32_t x = (int32_t) (bc0 -> int_pool[(c1 << 8) | c2]); 
      c0v_push(S, int2val(x));
      break;
    }

    case ALDC: {
      pc++;
      uint32_t c1 = (uint32_t) P[pc];
      pc++;
      uint32_t c2 = (uint32_t) P[pc];
      pc++;
      void *x = &(bc0 -> string_pool[(c1 << 8) | c2]); 
      c0v_push(S, ptr2val(x));
      break;
    }

    case ACONST_NULL: {
      pc++;
      void *x = NULL;
      c0v_push(S, ptr2val(x));
      break;
    }

    /* Operations on local variables */

    case VLOAD: {
      pc++;
      c0_value v = V[P[pc]];
      c0v_push(S, v);
      pc++;
      break;
    }

    case VSTORE: {
      pc++;
      c0_value v = c0v_pop(S);
      V[P[pc]] = v;
      pc++;
      break;
    }

    /* Assertions and errors */

    case ATHROW: {
      pc++;
      void *a = val2ptr(c0v_pop(S));
      char *s = (char*) a;
      c0_user_error(s);
      break;
    }

    case ASSERT: {
      pc++;
      void *a = val2ptr(c0v_pop(S));
      char *s = (char*) a;
      int32_t x = val2int(c0v_pop(S));
      if (x == 0) { c0_assertion_failure(s); }
      break; 
    }


    /* Control flow operations */

    case NOP: {
      pc++;
      break;
    }

    case IF_CMPEQ: {
      c0_value v1 = c0v_pop(S);
      c0_value v2 = c0v_pop(S);
      if(val_equal(v1, v2)) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case IF_CMPNE: {
      c0_value v1 = c0v_pop(S);
      c0_value v2 = c0v_pop(S);
      if(!val_equal(v1, v2)) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case IF_ICMPLT: {
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if(second < first) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case IF_ICMPGE:{
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if(second >= first) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case IF_ICMPGT: {
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if(second > first) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case IF_ICMPLE: {
      int32_t first = val2int(c0v_pop(S));
      int32_t second = val2int(c0v_pop(S));
      if(second <= first) {
        size_t i = pc;
        i++;
        int32_t o1 = (int32_t) P[i];
        i++;
        int32_t o2 = (int32_t) P[i];
        pc = pc + (int16_t) (o1 << 8 | o2);
      } else { pc = pc + 3; }
      break;
    }

    case GOTO: {
      size_t i = pc;
      i++;
      int32_t o1 = (int32_t) P[i];
      i++;
      int32_t o2 = (int32_t) P[i];
      pc = pc + (int16_t) (o1 << 8 | o2);
      break;
    }


    /* Function call operations: */

    case INVOKESTATIC: {
      pc++;
      uint32_t c1 = (uint32_t) P[pc];
      pc++;
      uint32_t c2 = (uint32_t) P[pc];
      pc++;
      struct function_info f = bc0 -> function_pool[(c1 << 8 | c2)];
      frame *new = xmalloc(sizeof(frame));
      new -> S = S;
      new -> P = P;
      new -> pc = pc; 
      new -> V = V; 
      push(callStack, (void*) new);
      pc = 0; 
      V = xcalloc(sizeof(c0_value), f.num_vars);
      for (uint8_t i = 0; i < f.num_args; i++) {
        V[f.num_args - i - 1] = c0v_pop(S);
      }
      S = c0v_stack_new();
      P = f.code;
      break;
    }

    case INVOKENATIVE: {
      pc++;
      uint32_t c1 = (uint32_t) P[pc];
      pc++;
      uint32_t c2 = (uint32_t) P[pc];
      pc++;
      struct native_info f = bc0 -> native_pool[(c1 << 8) | c2];
      uint8_t i = f.num_args;
      c0_value *new_V = xcalloc(sizeof(c0_value), i);
      for (uint8_t j = 0; j < i; j++) {
        new_V[i - j - 1] = c0v_pop(S);
      }
      uint16_t j = f.function_table_index;
      native_fn *address = native_function_table[j];
      c0_value res = (*address) (new_V);
      c0v_push(S, res);
      free(new_V);
      break;
    }


    /* Memory allocation and access operations: */

    case NEW: {
      pc++;
      ubyte s = (ubyte) P[pc];
      pc++;
      void* p = xmalloc(s);
      c0v_push(S, ptr2val(p));
      break;

    }

    case IMLOAD: {
      pc++;
      void *a = val2ptr(c0v_pop(S));
      if (a == NULL) {
        c0_memory_error("Invalid Access!");
      }
      int32_t x = (int32_t) (*(int*) a);
      c0v_push(S, int2val(x));
      break;
    }

    case IMSTORE: {
      pc++;
      int32_t x = val2int(c0v_pop(S));
      void *a = val2ptr(c0v_pop(S));
      if (a == NULL) {
        c0_memory_error("Invalid Access!");
      }
      *((int*) a) = x;
      break;
    }

    case AMLOAD: {
      pc++;
      void *temp = val2ptr(c0v_pop(S));
      void **a = (void**) temp;
      if (a == NULL) {
        c0_memory_error("Invalid Access!");
      }
      void *b = (void*) (*a);
      c0v_push(S, ptr2val(b));
      break;
    }

    case AMSTORE: {
      pc++;
      void *b = val2ptr(c0v_pop(S));
      void **a = (void**) val2ptr(c0v_pop(S));
      if (a == NULL) {
        c0_memory_error("Invalid Access!");
      }
      *a = b;
      break;
    }

    case CMLOAD:{
      pc++;
      void *temp = val2ptr(c0v_pop(S));
      if (temp == NULL) {
        c0_memory_error("Invalid Access!");
      }
      char *a = (char*)temp;
      int32_t x = (int32_t) *a;
      c0v_push(S, int2val(x));
      break;
    }

    case CMSTORE: {
      pc++;
      int32_t x = val2int(c0v_pop(S));
      void *temp = val2ptr(c0v_pop(S));
      if (temp == NULL) {
        c0_memory_error("Invalid Access!");
      }
      char *a = (char*) temp;
      *a = x & 0x7f;
      break;
    }

    case AADDF: {
      pc++;
      void *temp = val2ptr(c0v_pop(S));
      if (temp == NULL) {
        c0_memory_error("Invalid Access!");
      }
      ubyte *a = (ubyte*)temp;
      ubyte f = (ubyte) P[pc];
      pc++;
      c0v_push(S, ptr2val((void*) (a + f)));
      break;
    }


    /* Array operations: */

    case NEWARRAY: {
      pc++;
      int32_t s = (int32_t) P[pc];
      pc++;
      int32_t n = val2int(c0v_pop(S));
      if (n < 0) {
        c0_memory_error("Invalid Element!");
      }
      c0_array *a = xcalloc(sizeof(c0_array), 1);
      char *elem = xcalloc(sizeof(char), s * n);
      a -> count = (uint32_t) n;
      a -> elt_size = (uint32_t) s;
      a -> elems = (void*) elem;
      c0v_push(S, ptr2val((void*) a));
      break;
    }

    case ARRAYLENGTH:{
      pc++;
      void *temp = val2ptr(c0v_pop(S));
      if (temp == NULL) {
        c0_memory_error("Invalid Access!");
      }
      c0_array *a = (c0_array*) temp;
      int32_t res = (int32_t) (a -> count);
      if (res < 0) {
        c0_memory_error("Invalid Length!");
      }
      c0v_push(S, int2val(res));
      break;
    }

    case AADDS: {
      pc++;
      int32_t i = val2int(c0v_pop(S));
      void *temp = val2ptr(c0v_pop(S));
      c0_array *a = (c0_array*) temp;
      if (a == NULL || i < 0 || ((int32_t) (a -> count)) <= i) {
        c0_memory_error("Error!");
      }
      void *b = (void*) (((char*) (a -> elems)) + (a -> elt_size) * i);
      c0v_push(S, ptr2val(b));
      break;
    }


    /* BONUS -- C1 operations */

    case CHECKTAG:

    case HASTAG:

    case ADDTAG:

    case ADDROF_STATIC:

    case ADDROF_NATIVE:

    case INVOKEDYNAMIC:

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
