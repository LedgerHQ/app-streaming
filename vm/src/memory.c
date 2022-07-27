#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "error.h"
#include "lfsr.h"
#include "memory.h"
#include "stream.h"

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#define NPAGE_CODE  65
#define NPAGE_STACK 10
#define NPAGE_DATA  10
#else
#define NPAGE_CODE  3
#define NPAGE_STACK 3
#define NPAGE_DATA  1
#endif

struct cache_s {
    struct page_s *pages;
    size_t npage;
    bool writeable;
    // Only used for the code page cache currently. It doesn't improve
    // performances for stack and data pages.
    struct page_s *current_page;
};

struct memory_s {
    struct section_s sections[NUM_SECTIONS];

    struct page_s code_pages[NPAGE_CODE];
    struct page_s stack_pages[NPAGE_STACK];
    struct page_s data_pages[NPAGE_DATA];

    struct cache_s code;
    struct cache_s stack;
    struct cache_s data;

    uint32_t bss_max;
    uint32_t stack_min;
};

static struct memory_s memory;

void init_memory_sections(const struct section_s *sections)
{
    memcpy(memory.sections, sections, sizeof(memory.sections));
}

static void init_cache(struct cache_s *cache,
                       struct page_s *pages,
                       const size_t npage,
                       const bool writeable)
{
    cache->pages = pages;
    cache->npage = npage;
    cache->writeable = writeable;
    cache->current_page = NULL;
}

void init_caches(void)
{
    init_cache(&memory.code, memory.code_pages, NPAGE_CODE, false);
    init_cache(&memory.stack, memory.stack_pages, NPAGE_STACK, true);
    init_cache(&memory.data, memory.data_pages, NPAGE_DATA, true);

    lfsr_init();
}

void init_memory_addresses(uint32_t bss, uint32_t sp)
{
    memory.stack_min = sp;
    memory.bss_max = PAGE_START(bss);
}

static int binary_search(const struct page_s *pages, const size_t npage, const uint32_t addr)
{
    int l = 0;
    int r = npage - 1;

    while (l <= r) {
        int m = l + (r - l) / 2;
        if (pages[m].addr == addr) {
            return m;
        } else if (pages[m].addr < addr) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    return -1;
}

/**
 * Reorder an array of pages given an unsorted page. It requires the other pages
 * to be sorted in ascending order according to their addresses.
 *
 * @return the page given as argument, after reordering
 */
static struct page_s *reorder_pages(struct page_s *pages, const size_t npage, struct page_s *page)
{
    const uint32_t addr = page->addr;
    const size_t n = page - pages;

    if (n > 0 && addr < pages[n - 1].addr) {
        for (size_t i = 0; i < n; i++) {
            if (addr < pages[i].addr) {
                struct page_s tmp;
                memcpy(&tmp, page, sizeof(*page));
                memmove(&pages[i + 1], &pages[i], (n - i) * sizeof(*pages));
                memcpy(&pages[i], &tmp, sizeof(tmp));
                return &pages[i];
            }
        }
    } else if (n < npage - 1 && addr > pages[n + 1].addr) {
        for (size_t i = npage - 1; i > n; i--) {
            if (addr > pages[i].addr) {
                struct page_s tmp;
                memcpy(&tmp, page, sizeof(*page));
                memmove(&pages[n], &pages[n + 1], (i - n) * sizeof(*pages));
                memcpy(&pages[i], &tmp, sizeof(tmp));
                return &pages[i];
            }
        }
    }

    return page;
}

static struct page_s *reorder_cache(struct cache_s *cache, struct page_s *page)
{
    return reorder_pages(cache->pages, cache->npage, page);
}

static struct page_s *find_page(struct cache_s *cache, uint32_t addr)
{
    int n = binary_search(cache->pages, cache->npage, addr);

    if (n < 0) {
        return NULL;
    } else {
        return &cache->pages[n];
    }
}

static struct page_s *evict_page(struct cache_s *cache)
{
    size_t n;

    /* since pages are sorted by their addresses, free pages are the first array entries */
    if (cache->pages[0].addr == 0) {
        n = 0;
    } else {
        n = lfsr_get_random() % cache->npage;
    }

    return &cache->pages[n];
}

/**
 * The page argument is just here to avoid declaring this structure on the stack
 * since this struct is quite large.
 *
 * @return true on success, false otherwise
 */
static bool create_empty_pages(uint32_t from, uint32_t to, struct page_s *page)
{
    uint32_t addr;

    for (addr = from; addr < to; addr += PAGE_SIZE) {
        memset(page->data, '\x00', sizeof(page->data));
        page->addr = addr;
        /* During the first commit, the IV will be incremented to 1 and the
         * dynamic keys will be used for decryption and HMAC. */
        page->iv = 0;
        if (!stream_commit_page(page, true)) {
            return false;
        }
    }

    return true;
}

static struct page_s *create_page(struct cache_s *cache, const uint32_t addr)
{
    struct page_s *page = evict_page(cache);

    /* don't commit page if it never was retrieved (its address is zero) */
    if (cache->writeable && page->addr != 0) {
        if (!stream_commit_page(page, false)) {
            return NULL;
        }
    }

    /*
     * If a heap/stack page is accessed, create unexisting pages if they don't
     * exist (initialized with zeroes).
     */
    bool zero_page = false;
    if (cache == &memory.data) {
        if (addr >= memory.bss_max) {
            if (!create_empty_pages(memory.bss_max, addr + PAGE_SIZE, page)) {
                return NULL;
            }
            memory.bss_max = addr + PAGE_SIZE;
            zero_page = true;
        }
    } else if (cache == &memory.stack) {
        if (addr < memory.stack_min) {
            if (!create_empty_pages(PAGE_START(addr), memory.stack_min, page)) {
                return NULL;
            }
            memory.stack_min = addr;
            zero_page = true;
        }
    }

    if (!zero_page) {
        if (!stream_request_page(page, addr, !cache->writeable)) {
            return NULL;
        }
    } else {
        page->addr = addr;
        /* the IV was incremented by 1 during commit */
        page->iv = 1;
        memset(page->data, '\x00', sizeof(page->data));
    }

    // The code pages will be sorted and cache->current_page might point to
    // another page. It has no consequences.

    return reorder_cache(cache, page);
}

static bool in_section(enum section_e section, uint32_t addr)
{
    return addr >= memory.sections[section].start && addr < memory.sections[section].end;
}

static struct cache_s *get_cache(const uint32_t addr, const enum page_prot_e page_prot)
{
    if (in_section(SECTION_CODE, addr)) {
        if (page_prot != PAGE_PROT_RO) {
            err("write access to code page\n");
            return NULL;
        }
        return &memory.code;
    } else if (in_section(SECTION_DATA, addr)) {
        return &memory.data;
    } else if (in_section(SECTION_STACK, addr)) {
        return &memory.stack;
    } else {
        err("invalid addr (no section found)\n");
        return NULL;
    }
}

/**
 * @return NULL on error
 */
struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot)
{
    addr = PAGE_START(addr);

    struct cache_s *cache = get_cache(addr, page_prot);
    if (cache == NULL) {
        return NULL;
    }

    struct page_s *page = find_page(cache, addr);
    if (page != NULL) {
        return page;
    }

    // If the page isn't in the cache, evict a random page from the cache and
    // initialize it.
    return create_page(cache, addr);
}

struct page_s *get_code_page(const uint32_t addr)
{
    if (memory.code.current_page != NULL && memory.code.current_page->addr == addr) {
        return memory.code.current_page;
    }

    if (!in_section(SECTION_CODE, addr)) {
        return NULL;
    }

    struct page_s *page = find_page(&memory.code, addr);
    if (page != NULL) {
        return page;
    }

    page = evict_page(&memory.code);
    if (!stream_request_page(page, addr, true)) {
        return NULL;
    }

    memory.code.current_page = reorder_cache(&memory.code, page);

    return memory.code.current_page;
}
