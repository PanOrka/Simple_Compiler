#include "std_oper.h"
#include "../generators/stack_generator.h"
#include "../../parser_func/getters.h"

reg_allocator oper_load_variable(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc = reg_m_get(r_set, addr, true);
    if (!r_alloc.was_allocated) {
        if (r_alloc.r->flags & REG_MODIFIED) {
            stack_ptr_generate(r_alloc.r->addr, file);
            fprintf(file, "STORE %c %c\n", r_alloc.r->id, r_set->stack_ptr.id);
        }

        r_alloc.r->addr = addr;
        r_alloc.r->flags = REG_NO_FLAGS;
    }
}
