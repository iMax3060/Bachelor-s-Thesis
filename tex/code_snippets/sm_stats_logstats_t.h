class sm_stats_logstats_t {
private:
    std::ofstream*         logstats;
public:
    sm_stats_logstats_t() {
        logstats = new std::ofstream (sm_stats_logstats_t::filepath, 
                                      std::ofstream::app);
        w_assert0(logstats->is_open());
    };
    virtual ~sm_stats_logstats_t();
    
    static bool activate;
    static char* filepath;
public:
    void log_fix_nonroot(tid_t tid, PageID page, PageID parent, 
              latch_mode_t mode, bool conditional, bool virgin_page, 
              bool only_if_hit, u_long start, u_long finish);
    void log_fix_root(tid_t tid, PageID page, StoreID store, 
              latch_mode_t mode, bool conditional, u_long start, 
              u_long finish);
    void log_fix(tid_t tid, PageID page, PageID parent, 
              latch_mode_t mode, bool conditional, bool virgin_page, 
              bool only_if_hit, bool hit, bool evict, u_long start, 
              u_long finish);
    void log_unfix_nonroot(tid_t tid, PageID page, PageID parent, 
              bool evict, u_long start, u_long finish);
    void log_unfix_root(tid_t tid, PageID page, bool evict, 
              u_long start, u_long finish);
    void log_refix(tid_t tid, PageID page, latch_mode_t mode, 
              bool conditional, u_long start, u_long finish);
    void log_pin(tid_t tid, PageID page, u_long start,
              u_long finish);
    void log_unpin(tid_t tid, PageID page, u_long start, 
              u_long finish);
    void log_pick_victim_gclock(tid_t tid, bf_idx b_idx, 
              bf_idx index, u_long start, u_long finish);
    void log_miss_ref_car(tid_t tid, bf_idx b_idx, PageID page, 
              u_int32_t p, u_int32_t b1_length, u_int32_t b2_length,
              bf_idx t1_length, bf_idx t2_length, bf_idx t1_index, 
              bf_idx t2_index, u_long start, u_long finish);
    void log_pick_victim_car(tid_t tid, bf_idx b_idx,
             u_int32_t t1_movements, u_int32_t t2_movements, 
             u_int32_t p, u_int32_t b1_length, u_int32_t b2_length, 
             bf_idx t1_length, bf_idx t2_length, bf_idx t1_index, 
             bf_idx t2_index, u_long start, u_long finish);
};