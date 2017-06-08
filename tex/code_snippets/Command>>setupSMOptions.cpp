void Command::setupSMOptions(po::options_description& options) {
    boost::program_options::options_description smoptions("Storage Manager Options");
    smoptions.add_options()
    ("db-config-design", po::value<string>()->default_value("normal"),
       "")
    ("physical-hacks-enable", po::value<int>()->default_value(0),
        "Enables physical hacks, such as padding of records")
    ("db-worker-sli", po::value<bool>()->default_value(0),
        "Speculative Lock inheritance")
    ("db-loaders", po::value<int>()->default_value(10),
        "Specifies the number of threads that are used to load the db")
    ("db-worker-queueloops", po::value<int>()->default_value(10),
                "?")
    ("db-cl-batchsz", po::value<int>()->default_value(10),
                "Specify the batchsize of a client executing transactions")
    ("db-cl-thinktime", po::value<int>()->default_value(0),
            "Specify a 'thinktime' for a client")
    ("records-to-access", po::value<uint>()->default_value(0),
        "Used in the benchmarks for the secondary indexes")
    ("activation_delay", po::value<uint>()->default_value(0),
            "")
    ("db-workers", po::value<uint>()->default_value(1),
        "Specify the number of workers executing transactions")
    ("dir-trace", po::value<string>()->default_value("RAT"),
        "")
    /** System related options **/
    ("sys-maxcpucount", po::value<uint>()->default_value(0),
        "Maximum CPU Count of a system")
    ("sys-activecpucount", po::value<uint>()->default_value(0),
        "Active CPU Count of a system")
    /**SM Options**/
    ("sm_logdir", po::value<string>()->default_value("log"),
        "Path to log directory")
    ("sm_dbfile", po::value<string>()->default_value("db"),
        "Path to the file on which to store database pages")
    ("sm_format", po::value<bool>()->default_value(false),
        "Format SM by emptying logdir and truncating DB file")
    ("sm_truncate_log", po::value<bool>()->default_value(false)
        ->implicit_value(true),
        "Whether to truncate log partitions at SM shutdown")
    ("sm_truncate_archive", po::value<bool>()->default_value(false)
        ->implicit_value(true),
        "Whether to truncate log archive runs at SM shutdown")
    ("sm_log_partition_size", po::value<int>()->default_value(1024),
        "Size of a log partition in MB")
    ("sm_log_max_partitions", po::value<int>()->default_value(0),
        "Maximum number of partitions maintained in log directory")
    ("sm_log_delete_old_partitions", po::value<bool>()->default_value(true),
        "Whether to delete old log partitions as cleaner and chkpt make progress")
    ("sm_bufpoolsize", po::value<int>()->default_value(1024),
        "Size of buffer pool in MB")
    ("sm_fakeiodelay-enable", po::value<int>()->default_value(0),
        "Enables a artificial delay whenever there is a I/O operation")
    ("sm_fakeiodelay", po::value<uint>()->default_value(0),
            "Specify the imposed delay in usec")
    ("sm_errlog", po::value<string>()->default_value("shoremt.err.log"),
            "Path to the error log of the storage manager")
    ("sm_chkpt_interval", po::value<int>(),
            "Interval for checkpoint flushes")
    ("sm_log_fetch_buf_partitions", po::value<uint>()->default_value(0),
        "Number of partitions to buffer in memory for recovery")
    ("sm_log_page_flushers", po::value<uint>()->default_value(1),
        "Number of log page flushers")
    ("sm_preventive_chkpt", po::value<uint>()->default_value(1),
        "Disable/enable preventive checkpoints (0 to disable, 1 to enable)")
    ("sm_logbuf_seg_count", po::value<int>(),
        "Log Buffer Segment Count")
    ("sm_logbuf_flush_trigger", po::value<int>(),
        "?")
    ("sm_logbuf_block_size", po::value<int>(),
        "Log Buffer Block size")
    ("sm_logbuf_part_size", po::value<int>(),
        "Log Buffer part size")
    ("sm_carray_slots", po::value<int>(),
        "")
    ("sm_vol_log_reads", po::value<bool>(),
        "Generate log records for every page read")
    ("sm_vol_log_writes", po::value<bool>(),
        "Generate log records for every page write")
    ("sm_vol_readonly", po::value<bool>(),
        "Volume will be opened in read-only mode and all writes from buffer pool \
         will be ignored (uses write elision and single-page recovery)")
    ("sm_vol_o_direct", po::value<bool>(),
        "Whether to open volume (i.e., db file) with O_DIRECT")
    ("sm_restart_instant", po::value<bool>(),
        "Enable instant restart")
    ("sm_restart_log_based_redo", po::value<bool>(),
        "Perform non-instant restart with log-based redo instead of page-based")
    ("sm_rawlock_gc_interval_ms", po::value<int>(),
        "Garbage Collection Interval in ms")
    ("sm_rawlock_lockpool_segsize", po::value<int>(),
        "Segment size Lockpool")
    ("sm_rawlock_xctpool_segsize", po::value<int>(),
        "Segment size Transaction Pool")
    ("sm_rawlock_gc_generation_count", po::value<int>(),
        "Garbage collection generation count")
    ("sm_rawlock_gc_init_generation_count", po::value<int>(),
        "Garbage collection initial generation count")
    ("sm_rawlock_lockpool_initseg", po::value<int>(),
        "Lock pool init segment")
    ("sm_rawlock_xctpool_segsize", po::value<int>(),
        "Transaction Pool Segment Size")
    ("sm_rawlock_gc_free_segment_count", po::value<int>(),
        "Garbage Collection Free Segment Count")
    ("sm_rawlock_gc_max_segment_count", po::value<int>(),
        "Garbage Collection Maximum Segment Count")
    ("sm_locktablesize", po::value<int>(),
        "Lock table size")
    ("sm_rawlock_xctpool_initseg", po::value<int>(),
        "Transaction Pool Initialization Segment")
    ("sm_cleaner_decoupled", po::value<bool>(),
        "Enable/Disable decoupled cleaner")
    ("sm_cleaner_interval_millisec", po::value<int>(),
        "Cleaner sleep interval in ms")
    ("sm_cleaner_workspace_size", po::value<int>(),
        "Size of cleaner write buffer")
    ("sm_cleaner_num_candidates", po::value<int>(),
        "Number of candidate frames considered by each cleaner round")
    ("sm_cleaner_policy", po::value<string>(),
        "Policy used by cleaner to select candidates")
    ("sm_cleaner_min_write_size", po::value<int>(),
        "Page cleaner only writes clusters of pages with this minimum size")
    ("sm_cleaner_min_write_ignore_freq", po::value<int>(),
        "Ignore min_write_size every N rounds of cleaning")
    ("sm_cleaner_ignore_metadata", po::value<bool>(),
        "Do not write metadata pages (stnode and alloc caches) in cleaner")
    ("sm_cleaner_async_candidate_collection", po::value<bool>(),
        "Collect candidate frames to be cleaned in an asynchronous thread")
    ("sm_archiver_workspace_size", po::value<int>(),
        "Workspace size archiver")
    // CS TODO: archiver currently only works with 1MB blocks
    // ("sm_archiver_block_size", po::value<int>()->default_value(1024*1024),
    //     "Archiver Block size")
    ("sm_archiver_bucket_size", po::value<int>()->default_value(128),
        "Archiver bucket size")
    ("sm_merge_factor", po::value<int>(),
        "Merging factor")
    ("sm_archiving_blocksize", po::value<int>(),
        "Archiving block size")
    ("sm_reformat_log", po::value<bool>(),
        "Enable/Disable reformat log")
    ("sm_logging", po::value<bool>()->default_value(true),
        "Enable/Disable logging")
    ("sm_decoupled_cleaner", po::value<bool>(),
        "Use log-based propagation to clean pages")
    ("sm_shutdown_clean", po::value<bool>(),
        "Force buffer before shutting down SM")
    ("sm_archiving", po::value<bool>(),
        "Enable/Disable archiving")
    ("sm_async_merging", po::value<bool>(),
        "Enable/Disable Asynchronous merging")
    ("sm_statistics", po::value<bool>(),
        "Enable/Disable display of statistics")
    ("sm_ticker_enable", po::value<bool>(),
        "Enable/Disable ticker (currently always enabled)")
    ("sm_ticker_msec", po::value<int>(),
        "Ticker interval in millisec")
    ("sm_prefetch", po::value<bool>(),
        "Enable/Disable prefetching")
    ("sm_backup_prefetcher_segments", po::value<int>(),
        "Segment size restore")
    ("sm_restore_segsize", po::value<int>(),
        "Segment size restore")
    ("sm_restore_prefetcher_window", po::value<int>(),
        "Segment size restore")
    ("sm_restore_instant", po::value<bool>(),
        "Enable/Disable instant restore")
    ("sm_restore_reuse_buffer", po::value<bool>(),
        "Enable/Disable reusage of buffer")
    ("sm_restore_multiple_segments", po::value<int>(),
        "Number of segments to attempt restore at once")
    ("sm_restore_min_read_size", po::value<int>(),
        "Attempt to read at least this many bytes when scanning log archive")
    ("sm_restore_max_read_size", po::value<int>(),
        "Attempt to read at most this many bytes when scanning log archive")
    ("sm_restore_preemptive", po::value<bool>(),
        "Use preemptive scheduling during restore")
    ("sm_restore_sched_singlepass", po::value<bool>(),
        "Use single-pass scheduling in restore")
    ("sm_restore_threads", po::value<int>(),
        "Number of restore threads to use")
    ("sm_restore_sched_ondemand", po::value<bool>(),
        "Support on-demand restore")
    ("sm_restore_sched_random", po::value<bool>(),
        "Use random page order in restore scheduler")
    ("sm_bufferpool_swizzle", po::value<bool>(),
        "Enable/Disable bufferpool swizzle")
    ("sm_archiver_eager", po::value<bool>(),
        "Enable/Disable eager archiving")
    ("sm_archiver_read_whole_blocks", po::value<bool>(),
        "Enable/Disable reading whole blocks in the archiver")
    ("sm_archiver_slow_log_grace_period", po::value<int>(),
        "Enable/Disable slow log grace period")
    ("sm_errlog_level", po::value<string>(),
        "Specify a errorlog level. Options:")
        //TODO Stefan Find levels and insert them
    ("sm_log_impl", po::value<string>(),
        "Choose log implementation. Options")
        //TODO Stefan Find Implementations
    ("sm_backup_dir", po::value<string>(),
        "Path to a backup directory")
    ("sm_bufferpool_replacement_policy", po::value<string>(),
        "Replacement Policy")
    ("sm_evict_policy", po::value<string>()->default_value("latched"),
        "Specify a eviction policy. Options: latched, gclock")
    ("sm_bufferpool_gclock_k", po::value<int>()->default_value(10),
        "Specify the k-parameter for eviction policy glock")
    ("sm_archdir", po::value<string>()->default_value("archive"),
        "Path to archive directory")
    ("sm_fix_stats", po::value<bool>()->default_value(false),
        "Enable/Disable a log about page fix/unfix/refix/pin/unpin \
         in the buffer pool")
    ("sm_evict_stats", po::value<bool>()->default_value(false),
        "Enable/Disable a log about page evictions")
    ("sm_stats_file", po::value<string>()
        ->default_value("buffer.log"),
        "Path to the file where to write the log about the buffer \
         pool");
    options.add(smoptions);
}