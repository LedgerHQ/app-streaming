#include "../src/stream.h"

bool stream_commit_page(struct page_s *page, bool insert)
{
    return true;
}

bool stream_request_page(struct page_s *page, const uint32_t addr, const bool read_only)
{
    page->addr = addr;
    return true;
}
