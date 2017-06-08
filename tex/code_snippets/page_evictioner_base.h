class page_evictioner_base {
public:
    page_evictioner_base(bf_tree_m* bufferpool, 
                         const sm_options& options);
    virtual ~page_evictioner_base();

    virtual void    ref(bf_idx idx);
    virtual void    miss_ref(bf_idx b_idx, PageID pid);

    virtual void    evict();

protected:
    bf_tree_m*      _bufferpool;
    bool            _swizziling_enabled;

    virtual bf_idx  pick_victim();
    
    bool            evict_page(bf_idx idx, PageID &evicted_page);

private:
    const float     EVICT_BATCH_RATIO = 0.01;
    bf_idx          _current_frame;
    
    bool            unswizzle_and_update_emlsn(bf_idx idx);
};
