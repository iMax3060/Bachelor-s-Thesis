class smthread_t : public sthread_t {
    friend class smthread_init_t;
    
    struct tcb_t {
        xct_t *xct;
        int pin_count;
        int prev_pin_count;
        timeout_in_ms lock_timeout;
        bool _in_sm;
        bool _is_update_thread;
        
        int16_t _depth;
        tcb_t *_outer;
        
        xct_log_t *_xct_log;
        sm_stats_info_t *_TL_stats;
        
        sm_stats_logstats_t *_TL_stats_logstats;
        
        queue_based_lock_t::ext_qnode _me1;
        queue_based_lock_t::ext_qnode _me2;
        queue_based_lock_t::ext_qnode _me3;
        
        queue_based_lock_t::ext_qnode _1thread_xct_me;
        queue_based_lock_t::ext_qnode _xlist_mutex_node;
        
        void create_TL_stats();
        
        void clear_TL_stats();
        
        void destroy_TL_stats();
        
        inline sm_stats_info_t &TL_stats() { return *_TL_stats; }
        
        inline const sm_stats_info_t &TL_stats_const() const {
            return *_TL_stats;
        }
        
        inline sm_stats_logstats_t *TL_stats_logstats() {
            return _TL_stats_logstats;
        }
        
        tcb_t(tcb_t *outer) :
                xct(0),
                pin_count(0),
                prev_pin_count(0),
                lock_timeout(WAIT_FOREVER),
                _in_sm(false),
                _is_update_thread(false),
                _depth(outer == NULL ? 1 : outer->_depth + 1),
                _outer(outer),
                _xct_log(0),
                _TL_stats(0),
                _TL_stats_logstats(0) {
            QUEUE_EXT_QNODE_INITIALIZE(_me1);
            QUEUE_EXT_QNODE_INITIALIZE(_me2);
            QUEUE_EXT_QNODE_INITIALIZE(_me3);
            QUEUE_EXT_QNODE_INITIALIZE(_1thread_xct_me);
            QUEUE_EXT_QNODE_INITIALIZE(_xlist_mutex_node);
            
            create_TL_stats();
            
            if (sm_stats_logstats_t::activate) {
                _TL_stats_logstats = new sm_stats_logstats_t();
            }
        }
        
        ~tcb_t() {
            destroy_TL_stats();
            
            if (_TL_stats_logstats) {
                delete _TL_stats_logstats;
            }
        }
    };
    
    tcb_t *_tcb_tail;
    st_proc_t *const _proc;
    void *const _arg;
    
    bool _try_initialize_fingerprint();
    void _initialize_fingerprint();
    
    void _uninitialize_fingerprint();
    
    short _fingerprint[FINGER_BITS];
    atomic_thread_map_t _fingerprint_map;

public:
    const atomic_thread_map_t &get_fingerprint_map() const { return _fingerprint_map; }

public:
    
    NORET smthread_t(
            st_proc_t *f,
            void *arg,
            priority_t priority = t_regular,
            const char *name = 0,
            timeout_in_ms lockto = WAIT_FOREVER,
            unsigned stack_size = default_stack
    );
    
    NORET smthread_t(
            priority_t priority = t_regular,
            const char *name = 0,
            timeout_in_ms lockto = WAIT_FOREVER,
            unsigned stack_size = default_stack
    );
    
    w_rc_t join(timeout_in_ms timeout = WAIT_FOREVER);
    
    NORET ~smthread_t();
    
    static void init_fingerprint_map();

    virtual void before_run();
    
    virtual void run() = 0;
    
    virtual void after_run();
    
    virtual smthread_t *dynamic_cast_to_smthread();
    
    virtual const smthread_t *dynamic_cast_to_const_smthread() const;
    
    enum SmThreadTypes {
        smThreadType = 1, smLastThreadType
    };
    
    virtual int thread_type() { return smThreadType; }
    
    static void for_each_smthread(SmthreadFunc &f);
    
    void attach_xct(xct_t *x);
    
    void detach_xct(xct_t *x);
    
    inline
    timeout_in_ms lock_timeout() {
        return tcb().lock_timeout;
    }
    
    inline
    void lock_timeout(timeout_in_ms i) {
        tcb().lock_timeout = i;
    }
    
    inline
    xct_t *xct() { return tcb().xct; }
    
    inline
    xct_t *xct() const { return tcb().xct; }
    
    static smthread_t *me() { return (smthread_t *) sthread_t::me(); }
    
    inline sm_stats_info_t &TL_stats() {
        return tcb().TL_stats();
    }
    
    inline sm_stats_logstats_t *TL_stats_logstats() {
        return tcb().TL_stats_logstats();
    }
    
    void add_from_TL_stats(sm_stats_info_t &w) const;
    

#define GET_TSTAT(x) me()->TL_stats().sm.x
#define INC_TSTAT(x) me()->TL_stats().sm.x++
#define ADD_TSTAT(x, y) me()->TL_stats().sm.x += (y)
#define SET_TSTAT(x, y) me()->TL_stats().sm.x = (y)

#define LOGSTATS_FIX_NONROOT(tid, page, parent, mode, conditional, 
                virgin_page, only_if_hit, start, finish) 
            me()->TL_stats_logstats()->log_fix_nonroot(tid, page, 
                      parent, mode, conditional, virgin_page, 
                      only_if_hit, start, finish)
#define LOGSTATS_FIX_ROOT(tid, page, store, mode, conditional, 
                start, finish) 
            me()->TL_stats_logstats()->log_fix_root(tid, page, 
                      store, mode, conditional, start, finish)
#define LOGSTATS_FIX(tid, page, parent, mode, conditional, 
                virgin_page, only_if_hit, hit, evict, start, finish)
            me()->TL_stats_logstats()->log_fix(tid, page, parent, 
                      mode, conditional, virgin_page, only_if_hit, 
                      hit, evict, start, finish)
#define LOGSTATS_UNFIX_NONROOT(tid, page, parent, evict, start, 
                finish) 
            me()->TL_stats_logstats()->log_unfix_nonroot(tid, page, 
                      parent, evict, start, finish);
#define LOGSTATS_UNFIX_ROOT(tid, page, evict, start, finish) 
            me()->TL_stats_logstats()->log_unfix_root(tid, page, 
                      evict, start, finish);
#define LOGSTATS_REFIX(tid, page, mode, conditional, start, finish) 
            me()->TL_stats_logstats()->log_refix(tid, page, mode, 
                      conditional, start, finish)
#define LOGSTATS_PIN(tid, page, start, finish) 
            me()->TL_stats_logstats()->log_pin(tid, page, start, 
                      finish)
#define LOGSTATS_UNPIN(tid, page, start, finish) 
            me()->TL_stats_logstats()->log_unpin(tid, page, start, 
                      finish)
#define LOGSTATS_PICK_VICTIM_GCLOCK(tid, b_idx, index, start, 
                finish) 
            me()->TL_stats_logstats()->log_pick_victim_gclock(tid, 
                      b_idx, index, start, finish)
#define LOGSTATS_MISS_REF_CAR(tid, b_idx, page, p, b1_length, 
                b2_length, t1_length, t2_length, t1_index, t2_index, 
                start, finish) 
            me()->TL_stats_logstats()->log_miss_ref_car(tid, b_idx, 
                      page, p, b1_length, b2_length, t1_length, 
                      t2_length, t1_index, t2_index, start, finish)
#define LOGSTATS_PICK_VICTIM_CAR(tid, b_idx, t1_movements, 
                t2_movements, p, b1_length, b2_length, t1_length, 
                t2_length, t1_index, t2_index, start, finish) 
            me()->TL_stats_logstats()->log_pick_victim_car(tid, 
                      b_idx, t1_movements, t2_movements, p, 
                      b1_length, b2_length, t1_length, t2_length, 
                      t1_index, t2_index, start, finish)
    
    void mark_pin_count();
    
    void check_pin_count(int change);
    
    void check_actual_pin_count(int actual);
    
    void incr_pin_count(int amount);
    
    int pin_count();
    
    inline
    void in_sm(bool in) { tcb()._in_sm = in; }
    
    inline
    bool is_in_sm() const { return tcb()._in_sm; }
    
    inline
    bool is_update_thread() const {
        return tcb()._is_update_thread;
    }
    
    inline
    void set_is_update_thread(bool in) { tcb()._is_update_thread = in; }
    
    void new_xct(xct_t *);
    
    void no_xct(xct_t *);
    
    inline
    xct_log_t *xct_log() { return tcb()._xct_log; }
    
    virtual void _dump(ostream &) const; // to be over-ridden
    
    w_error_codes smthread_block(timeout_in_ms WAIT_FOREVER,
                                 const char *const caller = 0,
                                 const void *id = 0);
    
    w_rc_t smthread_unblock(w_error_codes e);
    
    int sampling;
private:
    w_error_codes _smthread_block(timeout_in_ms WAIT_FOREVER,
                                  const char *const why = 0);
    
    w_rc_t _smthread_unblock(w_error_codes e);

public:
    bool generate_log_warnings() const { return _gen_log_warnings; }
    
    void set_generate_log_warnings(bool b) { _gen_log_warnings = b; }
    
    queue_based_lock_t::ext_qnode &get_me3() { return tcb()._me3; }
    
    queue_based_lock_t::ext_qnode &get_me2() { return tcb()._me2; }
    
    queue_based_lock_t::ext_qnode &get_me1() { return tcb()._me1; }
    
    queue_based_lock_t::ext_qnode &get_xlist_mutex_node() {
        return tcb()._xlist_mutex_node;
    }
    
    queue_based_lock_t::ext_qnode &get_1thread_xct_me() {
        return tcb()._1thread_xct_me;
    }

private:
    bool _waiting;
    
    bool _gen_log_warnings;
    
    inline tcb_t &tcb() {
        w_assert3 (_tcb_tail != NULL);
        return *_tcb_tail;
    }
    
    inline const tcb_t &tcb() const {
        w_assert3 (_tcb_tail != NULL);
        return *_tcb_tail;
    }

public:
    inline size_t get_tcb_depth() const {
        w_assert3 (_tcb_tail != NULL);
        return _tcb_tail->_depth;
    }
};
