bf_tree_m::bf_tree_m(const sm_options& options)
{
    long bufpoolsize = 
        options.get_int_option("sm_bufpoolsize" , 8192)
      * 1024 * 1024;
    uint32_t nbufpages = (bufpoolsize - 1)
                       / sizeof(generic_page) + 1;
    if (nbufpages < 10)  {
        cerr << "ERROR: buffer size ("
             << bufpoolsize
             << "-KB) is too small" << endl;
        cerr << "       at least "
             << 32 * sizeof(generic_page) / 1024
             << "-KB is needed" << endl;
        W_FATAL(eCRASH);
    }

    bool bufferpool_swizzle =
        options.get_bool_option("sm_bufferpool_swizzle"
                              , false);
    std::string replacement_policy =
        options.get_string_option("sm_bufferpool_replacement_policy"
                                , "clock");

    ::memset (this, 0, sizeof(bf_tree_m));

    _block_cnt = nbufpages;
    _enable_swizzling = bufferpool_swizzle;

    DBGOUT1 (<< "constructing bufferpool with " << nbufpages
             << " blocks of " << SM_PAGESIZE 
             << "-bytes pages... enable_swizzling="
             << _enable_swizzling);

    void *buf = NULL;
    if (::posix_memalign(&buf, SM_PAGESIZE
                       , SM_PAGESIZE * ((uint64_t) nbufpages)) != 0)
    {
        ERROUT (<< "failed to reserve " << nbufpages
                << " blocks of " << SM_PAGESIZE
                << "-bytes pages. ");
        W_FATAL(eOUTOFMEMORY);
    }
    _buffer = reinterpret_cast<generic_page*>(buf);

    ::memset (&_buffer[0], 0x27, sizeof(generic_page));

    BOOST_STATIC_ASSERT(sizeof(bf_tree_cb_t) == 64);
    BOOST_STATIC_ASSERT(sizeof(latch_t) == 64);
    size_t total_size = (sizeof(bf_tree_cb_t) + sizeof(latch_t))
                      * (((uint64_t) nbufpages) + 1LLU);
    if (::posix_memalign(&buf, sizeof(bf_tree_cb_t)
                             + sizeof(latch_t), total_size) != 0)
    {
        ERROUT (<< "failed to reserve " << nbufpages
                << " blocks of " << sizeof(bf_tree_cb_t)
                << "-bytes blocks.");
        W_FATAL(eOUTOFMEMORY);
    }
    ::memset (buf, 0, (sizeof(bf_tree_cb_t) + sizeof(latch_t))
                    * (((uint64_t) nbufpages) + 1LLU));
    _control_blocks = reinterpret_cast<bf_tree_cb_t*>
        (reinterpret_cast<char*>(buf) + sizeof(bf_tree_cb_t));
    w_assert0(_control_blocks != NULL);
    for (bf_idx i = 0; i < nbufpages; i++) {
        BOOST_STATIC_ASSERT(sizeof(bf_tree_cb_t) < SCHAR_MAX);
        if (i & 0x1) {
            get_cb(i)._latch_offset = 
                -static_cast<int8_t>(sizeof(bf_tree_cb_t));
        } else {
            get_cb(i)._latch_offset = sizeof(bf_tree_cb_t);
        }
    }

    _freelist = new bf_idx[nbufpages];
    w_assert0(_freelist != NULL);
    _freelist[0] = 1;
    for (bf_idx i = 1; i < nbufpages - 1; ++i) {
        _freelist[i] = i + 1;
    }
    _freelist[nbufpages - 1] = 0;
    _freelist_len = nbufpages - 1;

    int buckets = w_findprime(1024 + (nbufpages / 4));
    _hashtable = new bf_hashtable<bf_idx_pair>(buckets);
    w_assert0(_hashtable != NULL);

    _eviction_current_frame = 0;
    DO_PTHREAD(pthread_mutex_init(&_eviction_lock, NULL));

    _cleaner_decoupled = 
        options.get_bool_option("sm_cleaner_decoupled" , false);
    
    _logstats_fix = 
        options.get_bool_option("sm_fix_stats", false);
}
