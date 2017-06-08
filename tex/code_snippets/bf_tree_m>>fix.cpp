w_rc_t bf_tree_m::fix(generic_page *parent, generic_page *&page,
                      PageID pid, latch_mode_t mode,
                      bool conditional, bool virgin_page,
                      bool only_if_hit, lsn_t emlsn)
{
    u_long start;
    bool hit = true;
    bool evict = false;
    if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
        start = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    }
    if (is_swizzled_pointer(pid)) {
        w_assert1(!virgin_page);

        bf_idx idx = pid ^ SWIZZLED_PID_BIT;
        w_assert1(_is_valid_idx(idx));
        bf_tree_cb_t &cb = get_cb(idx);

        W_DO(cb.latch().latch_acquire(mode,
              conditional ? sthread_t::WAIT_IMMEDIATE
                          : sthread_t::WAIT_FOREVER));

        w_assert1(_is_active_idx(idx));
        w_assert1(cb._swizzled);
        w_assert1(cb._pid == _buffer[idx].pid);

        cb.pin();
        cb.inc_ref_count();
        if(_evictioner) {
            _evictioner->ref(idx);
        }
        if (mode == LATCH_EX) {
            cb.inc_ref_count_ex();
        }

        page = &(_buffer[idx]);

        if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
            u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page, only_if_hit, hit, evict, start, finish);
        }

        return RCOK;
    }

    while (true)
    {
        bf_idx_pair p;
        bf_idx idx = 0;
        if (_hashtable->lookup(pid, p)) {
            idx = p.first;
        }

        if (idx == 0) {
            if (only_if_hit) {
                return RC(stINUSE);
            }

            // STEP 1) Grab a free frame to read into
            W_DO(_grab_free_block(idx));
            w_assert1(_is_valid_idx(idx));
            bf_tree_cb_t &cb = get_cb(idx);
            w_assert1(!cb._used);

            // STEP 2) Acquire EX latch before hash table insert, to make sure
            // nobody will access this page until we're done
            w_rc_t check_rc = cb.latch().latch_acquire(LATCH_EX,
                    sthread_t::WAIT_IMMEDIATE);
            if (check_rc.is_error()) {
                _add_free_block(idx);
                continue;
            }

            // Register the page on the hashtable atomically. This guarantees
            // that only one thread will attempt to read the page
            bf_idx parent_idx = parent ? parent - _buffer : 0;
            bool registered = _hashtable->insert_if_not_exists(pid,
                    bf_idx_pair(idx, parent_idx));
            if (!registered) {
                cb.latch().latch_release();
                _add_free_block(idx);
                continue;
            }

            // Read page from disk
            page = &_buffer[idx];
            cb.init(pid, lsn_t::null);

            if (!virgin_page) {
                INC_TSTAT(bf_fix_nonroot_miss_count);

                if (parent && emlsn.is_null()) {
                    // Get emlsn from parent
                    general_recordid_t recordid = 
                        find_page_id_slot(parent, pid);
                    btree_page_h parent_h;
                    parent_h.fix_nonbufferpool_page(parent);
                    emlsn = parent_h.get_emlsn_general(recordid);
                }

                w_rc_t read_rc = smlevel_0::vol->
                    read_page_verify(pid, page, emlsn);
                if (read_rc.is_error()) {
                    _hashtable->remove(pid);
                    cb.clear_except_latch();
                    cb.latch().latch_release();
                    _add_free_block(idx);
                    return read_rc;
                }
                cb.init(pid, page->lsn);
            }

            w_assert1(_is_active_idx(idx));

            // STEP 6) Fix successful -- pin page and downgrade latch
            cb.pin();
            w_assert1(cb.latch().is_mine());
            w_assert1(cb._pin_cnt > 0);
            DBG(<< "Fixed page " << pid << " (miss) to frame " << idx);

            if (mode != LATCH_EX) {
                w_assert1(mode == LATCH_SH);
                cb.latch().downgrade();
            }
        }
        else {
            // Page index is registered in hash table
            bf_tree_cb_t &cb = get_cb(idx);

            // Page is registered in hash table and it is not an in_doubt page,
            // meaning the actual page is in buffer pool already

            W_DO(cb.latch().latch_acquire(mode,
                  conditional ? sthread_t::WAIT_IMMEDIATE
                              : sthread_t::WAIT_FOREVER));

            if (cb._pin_cnt < 0 || cb._pid != pid) {
                // Page was evicted between hash table probe and latching
                DBG(<< "Page evicted right before latching. Retrying.");
                cb.latch().latch_release();
                continue;
            }

            w_assert1(_is_active_idx(idx));
            cb.pin();
            cb.inc_ref_count();
            if(_evictioner) _evictioner->ref(idx);
            if (mode == LATCH_EX) {
                cb.inc_ref_count_ex();
            }

            page = &(_buffer[idx]);

            w_assert1(cb.latch().held_by_me());
            DBG(<< "Fixed page " << pid << " (hit) to frame " << idx);
            w_assert1(cb._pin_cnt > 0);
        }

        if (!is_swizzled(page) && _enable_swizzling && parent) {
            // swizzle pointer for next invocations
            bf_tree_cb_t &cb = get_cb(idx);

            // Get slot on parent page
            w_assert1(_is_active_idx(parent - _buffer));
            w_assert1(latch_mode(parent) != LATCH_NL);
            fixable_page_h p;
            p.fix_nonbufferpool_page(parent);
            general_recordid_t slot = 
                find_page_id_slot(parent, pid);

            if (!is_swizzled(parent)) {
                if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
                    u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page, only_if_hit, hit, evict, start, finish);
                }
    
                return RCOK;
            }

            // Either a virgin page which hasn't been linked yet, or some other
            // thread won the race and already swizzled the pointer
            if (slot == GeneralRecordIds::INVALID) {
                if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
                    u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page, only_if_hit, hit, evict, start, finish);
                }
    
                return RCOK;
            }
            // Not worth swizzling foster children, since they will soon be
            // adopted (an thus unswizzled)
            if (slot == GeneralRecordIds::FOSTER_CHILD) {
                if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
                    u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page, only_if_hit, hit, evict, start, finish);
                }
    
                return RCOK;
            }
            w_assert1(slot > GeneralRecordIds::FOSTER_CHILD);
            w_assert1(slot <= p.max_child_slot());

            // Update _swizzled flag atomically
            bool old_value = false;
            if (!std::atomic_compare_exchange_strong(&cb._swizzled
                                             , &old_value, true)) {
                if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
                    u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page,
                                 only_if_hit, hit, evict, start, finish);
                }

                // CAS failed -- some other thread is swizzling
                return RCOK;
            }
            w_assert1(is_swizzled(page));

            // Replace pointer with swizzled version
            PageID* addr = p.child_slot_address(slot);
            *addr = idx | SWIZZLED_PID_BIT;

        }

        if (_logstats_fix && (std::strcmp(me()->name(), "") == 0 || std::strncmp(me()->name(), "w", 1) == 0)) {
            u_long finish = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            if (parent) {
                LOGSTATS_FIX(xct()->tid(), page->pid, parent->pid, mode, conditional, virgin_page,
                             only_if_hit, hit, evict, start, finish);
            } else {
                LOGSTATS_FIX(xct()->tid(), page->pid, 0, mode, conditional, virgin_page,
                             only_if_hit, hit, evict, start, finish);
            }
        }
    
        return RCOK;
    }
}
