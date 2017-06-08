template<class key, class value>
class multi_clock {
public:
    typedef clk_idx u_int32_t;

private:
    class index_pair {
    public:
        index_pair() {};
        index_pair(key before, key after) {
            this->_before = before;
            this->_after = after;
        };

        // visited before
        key     _before;
        // visited after
        key     _after;
    };

    // number of elements in the multi clock:
    key                             _clocksize;
    // stored values of the elements:
    value*                          _values;
    // .first == before, .second == after:
    index_pair*                     _clocks;
    // index value with NULL semantics:
    key                             _invalid_index;
    // to which clock does an element belong?:
    clk_idx*                        _clock_membership;

    // number of clocks in the multi clock:
    clk_idx                         _clocknumber;
    // always points to the clocks head:
    key*                            _hands;
    // number of elements within a clock:
    key*                            _sizes;
    // index of a clock value with NULL semantics:
    clk_idx                         _invalid_clock_index;

public:
    multi_clock(key clocksize, u_int32_t clocknumber,
                               key invalid_index);
    virtual         ~multi_clock();
    
    bool            get_head(clk_idx clock, value &head_value);
    bool            set_head(clk_idx clock, value new_value);
    bool            get_head_index(clk_idx clock, key &head_index);
    bool            move_head(clk_idx clock);
    bool            add_tail(clk_idx clock, key index);
    bool            remove_head(clk_idx clock, key &removed_index);
    bool            remove(key &index);
    bool            switch_head_to_tail(clk_idx source, 
                        clk_idx destination, key &moved_index);
    inline key      size_of(clk_idx clock);
    
    inline value&   get(key index) {
        return _values[index];
    }
    inline void     set(key index, value new_value) {
        _values[index] = new_value;
    }
    inline value&   operator[](key index) {
        return _values[index];
    }
};
