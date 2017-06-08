bf_tree_cb_t* bf_tree_m::get_cbp(bf_idx idx) const {
    bf_idx real_idx;
    real_idx = (idx << 1) + (idx & 0x1);
    return &_control_blocks[real_idx];
}

bf_tree_cb_t& bf_tree_m::get_cb(bf_idx idx) const {
    return *get_cbp(idx);
}
