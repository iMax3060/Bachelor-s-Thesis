class page_evictioner_car : public page_evictioner_base {
public:
    page_evictioner_car(bf_tree_m *bufferpool,
                        const sm_options &options);
    virtual             ~page_evictioner_car();
    
    virtual void        hit_ref(bf_idx idx);
    virtual void        miss_ref(bf_idx b_idx, PageID pid);
    virtual void        used_ref(bf_idx idx);
    virtual void        dirty_ref(bf_idx idx);
    virtual void        block_ref(bf_idx idx);
    virtual void        swizzle_ref(bf_idx idx);
    virtual void        unbuffered(bf_idx idx);

protected:
    bf_idx              pick_victim();

protected:
    multi_clock<bf_idx, bool>*      _clocks;
    hashtable_queue<PageID>*        _b1;
    hashtable_queue<PageID>*        _b2;
    
    u_int32_t                       _p;
    u_int32_t                       _c;
    bf_idx                          _hand_movement;
    
    pthread_mutex_t                 _lock;

    enum clock_index {
        T_1 = 0,
        T_2 = 1
    };
};