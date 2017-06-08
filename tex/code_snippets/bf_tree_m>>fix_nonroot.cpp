w_rc_t bf_tree_m::fix_nonroot(generic_page *&page, 
        generic_page *parent, PageID pid, latch_mode_t mode, 
        bool conditional, bool virgin_page, bool only_if_hit, 
        lsn_t emlsn) {
    INC_TSTAT(bf_fix_nonroot_count);
    u_long start;
    if (_logstats_fix && (std::strcmp(me()->name(), "") == 0
     || std::strncmp(me()->name(), "w", 1) == 0)) {
        start = std::chrono::high_resolution_clock::now()
                                        .time_since_epoch().count();
    }
    
    w_rc_t return_code = fix(parent, page, pid, mode, conditional,
                             virgin_page, only_if_hit, emlsn);
    
    if (_logstats_fix && (std::strcmp(me()->name(), "") == 0
     || std::strncmp(me()->name(), "w", 1) == 0) &&
        !return_code.is_error()) {
        u_long finish = std::chrono::high_resolution_clock::now()
                                        .time_since_epoch().count();
        LOGSTATS_FIX_NONROOT(xct()->tid(), page->pid, parent->pid, 
         mode, conditional, virgin_page,only_if_hit, start, finish);
    }
    
    return return_code;
}
