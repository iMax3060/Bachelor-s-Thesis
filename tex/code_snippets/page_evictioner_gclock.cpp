page_evictioner_gclock::page_evictioner_gclock(bf_tree_m* bufferpool,
                            const sm_options& options)
    : page_evictioner_base(bufferpool, options) {
    _k = options.get_int_option("sm_bufferpool_gclock_k", 10);
    _counts = new uint16_t[_bufferpool->_block_cnt];
    _current_frame = 0;

}

page_evictioner_gclock::~page_evictioner_gclock() {
    delete[] _counts;
}

void page_evictioner_gclock::hit_ref(bf_idx idx) {
    _counts[idx] = _k;
}

void page_evictioner_gclock::miss_ref(bf_idx b_idx, PageID pid) {}

void page_evictioner_gclock::used_ref(bf_idx idx) {
    hit_ref(idx);
}

void page_evictioner_gclock::dirty_ref(bf_idx idx) {}

void page_evictioner_gclock::block_ref(bf_idx idx) {
    _counts[idx] = std::numeric_limits<uint16_t>::max();
}

void page_evictioner_gclock::swizzle_ref(bf_idx idx) {}

void page_evictioner_gclock::unbuffered(bf_idx idx) {
    _counts[idx] = 0;
}

bf_idx page_evictioner_gclock::pick_victim() {
    u_long start;
    if (_logstats_evict && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
        start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }
    
    bf_idx idx = _current_frame;
    while(true) {
        idx = (idx % (_bufferpool->_block_cnt - 1)) + 1;
        w_assert1(idx != 0);

        bf_idx next_idx
              = ((idx + 1) % (_bufferpool->_block_cnt - 1)) + 1;
        __builtin_prefetch(&_bufferpool->_buffer[next_idx]);
        __builtin_prefetch(_bufferpool->get_cbp(next_idx));

        bf_tree_cb_t& cb = _bufferpool->get_cb(idx);

        rc_t latch_rc = cb.latch().latch_acquire(LATCH_SH, 
                                     sthread_t::WAIT_IMMEDIATE);
        if (latch_rc.is_error()) {
            idx++;
            continue;
        }

        w_assert1(cb.latch().held_by_me());

        btree_page_h p;
        p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
        if (p.tag() != t_btree_p || cb.is_dirty()
         || !cb._used || p.pid() == p.root()) {
            cb.latch().latch_release();
            idx++;
            continue;
        }

        if(_swizziling_enabled 
        && _bufferpool->has_swizzled_child(idx)) {
            cb.latch().latch_release();
            idx++;
            continue;
        }

        if(_counts[idx] <= 0)
        {
            bool would_block;
            cb.latch().upgrade_if_not_block(would_block);
            if(!would_block) {
                w_assert1(cb.latch().is_mine());

                if (cb._pin_cnt != 0) {
                    cb.latch().latch_release();
                    idx++;
                    continue;
                }

                _current_frame = idx + 1;
    
                if (_logstats_evict && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
                    u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    LOGSTATS_PICK_VICTIM_GCLOCK(xct()->tid(), idx, _current_frame, start, finish);
                }
    
                return idx;
            }
        }
        cb.latch().latch_release();
        --_counts[idx];
        idx++;
    }
}