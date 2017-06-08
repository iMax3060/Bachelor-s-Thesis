page_evictioner_base::page_evictioner_base(bf_tree_m* bufferpool,
                            const sm_options& options)
    : _bufferpool(bufferpool) {
    _swizziling_enabled
        = options.get_bool_option("sm_bufferpool_swizzle", false);
    _current_frame = 0;
    
    _logstats_evict
        = options.get_bool_option("sm_evict_stats", false);
}

page_evictioner_base::~page_evictioner_base() {}

void page_evictioner_base::evict() {
    uint32_t preferred_count
        = EVICT_BATCH_RATIO * _bufferpool->_block_cnt + 1;

    while(_bufferpool->_approx_freelist_length < preferred_count)
    {
        bf_idx victim = pick_victim();
    
        if(victim == 0) {
            return;
        }

        w_assert1(victim != 0);

        bf_tree_cb_t& cb = _bufferpool->get_cb(victim);
        w_assert1(cb.latch().is_mine());
    
        if(!unswizzle_and_update_emlsn(victim)) {
            cb.latch().latch_release();
            continue;
        }

        PageID pid = _bufferpool->_buffer[victim].pid;
        w_assert1(cb._pin_cnt < 0 || pid == cb._pid);

        bool removed = _bufferpool->_hashtable->remove(pid);
        w_assert1(removed);

        DBG2(<< "EVICTED " << victim << " pid " << pid
             << " log-tail " << smlevel_0::log->curr_lsn());
        cb.clear_except_latch();
        cb._pin_cnt = -1;

        _bufferpool->_add_free_block(victim);

        cb.latch().latch_release();

        INC_TSTAT(bf_evict);
    }
}

void page_evictioner_base::hit_ref(bf_idx idx) {}

void page_evictioner_base::miss_ref(bf_idx b_idx, PageID pid) {}

void page_evictioner_base::used_ref(bf_idx idx) {}

void page_evictioner_base::dirty_ref(bf_idx idx) {}

void page_evictioner_base::block_ref(bf_idx idx) {}

void page_evictioner_base::swizzle_ref(bf_idx idx) {}

void page_evictioner_base::unbuffered(bf_idx idx) {}

bf_idx page_evictioner_base::pick_victim() {
    bf_idx idx = _current_frame;
    while (true) {
        if (idx == _bufferpool->_block_cnt) {
            idx = 1;
        }
        
        if (idx == _current_frame - 1) {
            _bufferpool->get_cleaner()->wakeup(true);
        }
    
        PageID evicted_page;
        if (evict_page(idx, evicted_page)) {
            w_assert1(_bufferpool->_is_active_idx(idx));
            _current_frame = idx + 1;
            return idx;
        } else {
            idx++;
            continue;
        }
    }
}

bool page_evictioner_base::unswizzle_and_update_emlsn(bf_idx idx) {
    bf_tree_cb_t& cb = _bufferpool->get_cb(idx);
    w_assert1(cb.latch().is_mine());

    PageID pid = _bufferpool->_buffer[idx].pid;
    bf_idx_pair idx_pair;
    bool found = _bufferpool->_hashtable->lookup(pid, idx_pair);

    bf_idx parent_idx = idx_pair.second;
    w_assert1(!found || idx == idx_pair.first);

    if (!found || parent_idx == 0) {
        return false;
    }

    bf_tree_cb_t& parent_cb = _bufferpool->get_cb(parent_idx);
    rc_t r = parent_cb.latch().latch_acquire(LATCH_EX,
                                 sthread_t::WAIT_IMMEDIATE);
    if (r.is_error()) {
        return false;
    }
    w_assert1(parent_cb.latch().is_mine());

    w_assert1(_is_active_idx(parent_idx));
    generic_page *parent = &_bufferpool->_buffer[parent_idx];
    btree_page_h parent_h;
    parent_h.fix_nonbufferpool_page(parent);

    general_recordid_t child_slotid;
    if (_swizziling_enabled && cb._swizzled) {
        PageID swizzled_pid = idx | SWIZZLED_PID_BIT;
        child_slotid
            = _bufferpool->find_page_id_slot(parent, swizzled_pid);
    }
    else {
        child_slotid = _bufferpool->find_page_id_slot(parent, pid);
    }
    w_assert1 (child_slotid != GeneralRecordIds::INVALID);
    
    if (_swizziling_enabled && cb._swizzled) {
        bool ret = _bufferpool->unswizzle(parent, child_slotid);
        w_assert0(ret);
        w_assert1(!cb._swizzled);
    }

    lsn_t old = parent_h.get_emlsn_general(child_slotid);
    _bufferpool->_buffer[idx].lsn = cb.get_page_lsn();
    if (old < _bufferpool->_buffer[idx].lsn) {
        DBG3(<< "Updated EMLSN on page " << parent_h.pid()
                << " slot=" << child_slotid
                << " (child pid=" << pid << ")"
                << ", OldEMLSN=" << old
                << " NewEMLSN=" << _bufferpool->_buffer[idx].lsn);
    
        w_assert1(parent_cb.latch().is_mine());
        w_assert1(parent_cb.latch().mode() == LATCH_EX);
    
        W_COERCE(_bufferpool->_sx_update_child_emlsn(parent_h, 
                      child_slotid, _bufferpool->_buffer[idx].lsn));
    
        w_assert1(parent_h.get_emlsn_general(child_slotid)
                  == _bufferpool->_buffer[idx].lsn);
    }

    parent_cb.latch().latch_release();
    return true;
}

bool page_evictioner_base::evict_page(bf_idx idx, 
                                   PageID &evicted_page) {
    bf_tree_cb_t &cb = _bufferpool->get_cb(idx);
    evicted_page = cb._pid;
    
    rc_t latch_rc = cb.latch().latch_acquire(LATCH_EX,
                                 sthread_t::WAIT_IMMEDIATE);
    if (latch_rc.is_error()) {
        return false;
    }
    w_assert1(cb.latch().is_mine());
    
    btree_page_h p;
    p.fix_nonbufferpool_page(_bufferpool->_buffer + idx);
    if (!cb._used || p.get_foster() != 0) {
        used_ref(idx);
        cb.latch().latch_release();
        return false;
    }
    
    if (p.tag() != t_btree_p || p.pid() == p.root()) {
        block_ref(idx);
        cb.latch().latch_release();
        return false;
    }
    
    if (cb.is_dirty() && !_bufferpool->_cleaner_decoupled) {
        dirty_ref(idx);
        cb.latch().latch_release();
        return false;
    }
    
    if (_swizziling_enabled
     && _bufferpool->has_swizzled_child(idx)) {
        swizzle_ref(idx);
        cb.latch().latch_release();
        return false;
    }
    
    return true;
}