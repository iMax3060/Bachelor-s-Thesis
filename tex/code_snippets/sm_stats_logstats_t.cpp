bool sm_stats_logstats_t::activate = false;
char *sm_stats_logstats_t::filepath = "";

void sm_stats_logstats_t::log_fix_nonroot(tid_t tid, PageID page, 
          PageID parent, latch_mode_t mode, bool conditional,
          bool virgin_page, bool only_if_hit, u_long start, 
          u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix_nonroot,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << mode << ","
              << conditional << ","
              << virgin_page << ","
              << only_if_hit << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_fix_root(tid_t tid, PageID page, 
          StoreID store, latch_mode_t mode, bool conditional,
          u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix_root,"
              << tid.as_int64() << ","
              << page << ","
              << store << ","
              << mode << ","
              << conditional << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_fix(tid_t tid, PageID page, 
          PageID parent, latch_mode_t mode, bool conditional,
          bool virgin_page, bool only_if_hit, bool hit, bool evict,
           u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "fix,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << mode << ","
              << conditional << ","
              << virgin_page << ","
              << only_if_hit << ","
              << hit << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void
sm_stats_logstats_t::log_unfix_nonroot(tid_t tid, PageID page, 
          PageID parent, bool evict, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unfix_nonroot,"
              << tid.as_int64() << ","
              << page << ","
              << parent << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_unfix_root(tid_t tid, PageID page, 
          bool evict, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unfix_root,"
              << tid.as_int64() << ","
              << page << ","
              << evict << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_refix(tid_t tid, PageID page, 
          latch_mode_t mode, bool conditional, u_long start,
          u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "refix,"
              << tid.as_int64() << ","
              << page << ","
              << mode << ","
              << conditional << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_pin(tid_t tid, PageID page, 
          u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "pin,"
              << tid.as_int64() << ","
              << page << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_unpin(tid_t tid, PageID page, 
          u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "unpin,"
              << tid.as_int64() << ","
              << page << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_pick_victim_gclock(tid_t tid, 
          bf_idx b_idx, bf_idx index, u_long start, u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "pick_victim,"
              << tid.as_int64() << ","
              << b_idx << ","
              << index << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_miss_ref_car(tid_t tid, bf_idx b_idx, 
          PageID page, u_int32_t p, u_int32_t b1_length, 
          u_int32_t b2_length, bf_idx t1_length, bf_idx t2_length, 
          bf_idx t1_index, bf_idx t2_index, u_long start, 
          u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "miss_ref,"
              << tid.as_int64() << ","
              << b_idx << ","
              << page << ","
              << p << ","
              << b1_length << ","
              << b2_length << ","
              << t1_length << ","
              << t2_length << ","
              << t1_index << ","
              << t2_index << ","
              << start << ","
              << finish << std::endl;
}

void sm_stats_logstats_t::log_pick_victim_car(tid_t tid, 
          bf_idx b_idx, u_int32_t t1_movements, 
          u_int32_t t2_movements, u_int32_t p, u_int32_t b1_length,
          u_int32_t b2_length, bf_idx t1_length, bf_idx t2_length, 
          bf_idx t1_index, bf_idx t2_index, u_long start, 
          u_long finish) {
    w_assert1(logstats->is_open());
    w_assert1(sm_stats_logstats_t::activate);
    
    *logstats << "pick_victim,"
              << tid.as_int64() << ","
              << b_idx << ","
              << t1_movements << ","
              << t2_movements << ","
              << p << ","
              << b1_length << ","
              << b2_length << ","
              << t1_length << ","
              << t2_length << ","
              << t1_index << ","
              << t2_index << ","
              << start << ","
              << finish << std::endl;
}

sm_stats_logstats_t::~sm_stats_logstats_t() {
    logstats->close();
    delete logstats;
}
