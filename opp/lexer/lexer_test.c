#include "../memory/memory.h"
#include <assert.h>
#include "lexer.h"
#include "../util/util.h"

static const char tests[][50] = {
    {
        "123   + 34 - -34;"
    }
};
    #include <windows.h>

int main() {
    assert(allocator_init(malloc, free));

    struct Opp_State state = {0};

    enum Status s = ERROR_RECOVER(state.error_buf);

    if (s != OPP_ERROR) {
        struct Opp_Scan *const s = opp_init_lex(&state);
        // opp_init_from_buffer(s, "123\n6325435432.5 + 2354.4.\n");
        opp_init_from_buffer(s, "123\na + b - 4 + k--\n");

        dump_tokens(s);
    }
    else {
        // printf("Error recovered\n");
    }


    allocator_free();
}