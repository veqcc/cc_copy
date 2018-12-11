#include "9cc.h"

static bool *used;
static int reg_map[8192];
static int reg_map_sz = sizeof(reg_map) / sizeof(*reg_map);

static int alloc(int ir_reg) {
    if (reg_map_sz <= ir_reg) {
        error("program too big");
    }
    if (reg_map[ir_reg] != -1) {
        int r = reg_map[ir_reg];
        assert(used[r]);
        return r;
    }

    for (int i = 0; i < num_regs; i++) {
        if (used[i]) continue;
        reg_map[ir_reg] = i;
        used[i] = true;
        return i;
    }

    error("register exhausted");
    return -1;
}

static void kill(int r) {
    assert(used[r]);
    used[r] = false;
}

static void visit(IR *ir) {
    if (ir->lhs) {
        ir->lhs = alloc(ir->lhs);
    }
    if (ir->rhs) {
        ir->rhs = alloc(ir->rhs);
    }
    if (ir->op == IR_CALL) {
        for (int i = 0; i < ir->nargs; i++) {
            ir->args[i] = alloc(ir->args[i]);
        }
    }

    for (int i = 0; i < ir->kill->len; i++) {
        int r = (intptr_t)ir->kill->data[i];
        kill(reg_map[r]);
    }
}

void alloc_regs(Program *prog) {
    used = calloc(1, num_regs);
    for (int i = 0; i < reg_map_sz; i++) {
        reg_map[i] = -1;
    }

    for (int i = 0; i < prog->funcs->len; i++) {
        Function *fn = prog->funcs->data[i];
        for (int i = 0; i < fn->bbs->len; i++) {
            BB *bb = fn->bbs->data[i];
            for (int i = 0; i < bb->ir->len; i++) {
                IR *ir = bb->ir->data[i];
                visit(ir);
            }
        }
    }
}