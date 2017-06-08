class page_evictioner_gclock : public page_evictioner_base {
public:
    page_evictioner_gclock(bf_tree_m* bufferpool, const sm_options& options);
    virtual ~page_evictioner_gclock();

    virtual void            hit_ref(bf_idx idx);
    virtual void            miss_ref(bf_idx b_idx, PageID pid);
    virtual void            used_ref(bf_idx idx);
    virtual void            dirty_ref(bf_idx idx);
    virtual void            block_ref(bf_idx idx);
    virtual void            swizzle_ref(bf_idx idx);
    virtual void            unbuffered(bf_idx idx);

protected:
    virtual bf_idx          pick_victim();

private:
    uint16_t            _k;
    uint16_t*           _counts;
    bf_idx              _current_frame;
};