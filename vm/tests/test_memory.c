#include <setjmp.h>
#include <stdio.h>

#include <cmocka.h>

// include the .c file to have access to static variables and functions
#include "../src/memory.c"

static void test_memory(void **state __attribute__((unused)))
{
    const struct section_s sections[NUM_SECTIONS] = {
        { 0x00010000, 0x0001A800 }, // code
        { 0x7FFF0000, 0x80000000 }, // stack
        { 0x0001B700, 0x0002BA00 }, // data
    };
    init_memory_sections(sections);

    const uint32_t sp = sections[SECTION_STACK].end;
    const uint32_t bss = 0x0001ba00;
    init_memory_addresses(bss, sp);

    init_caches();

    // insert a page into the code cache
    assert_non_null(get_page(0x00010000, PAGE_PROT_RO));
    assert_int_equal(memory.code.pages[NPAGE_CODE - 1].addr, 0x00010000);

    // insert a 2nd page into the code cache
    assert_non_null(get_page(0x00010100, PAGE_PROT_RO));
    assert_int_equal(memory.code.pages[NPAGE_CODE - 1].addr, 0x00010100);
    assert_int_equal(memory.code.pages[NPAGE_CODE - 2].addr, 0x00010000);

    // can't get a writeable page from the code cache
    assert_null(get_page(0x00010000, PAGE_PROT_RW));

    // can't get a code page from the stack
    assert_null(get_code_page(0x7FFF0000));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_memory),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
