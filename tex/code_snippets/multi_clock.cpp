template<class key, class value>
multi_clock<key, value>::multi_clock(key clocksize, 
         clk_idx clocknumber, key invalid_index) {
    _clocksize = clocksize;
    _values = new value[_clocksize]();
    _clocks = new index_pair[_clocksize]();
    _invalid_index = invalid_index;
    
    _clocknumber = clocknumber;
    _hands = new key[_clocknumber]();
    _sizes = new key[_clocknumber]();
    for (int i = 0; i <= _clocknumber - 1; i++) {
        _hands[i] = _invalid_index;
    }
    _invalid_clock_index = _clocknumber;
    _clock_membership = new clk_idx[_clocksize]();
    for (int i = 0; i <= _clocksize - 1; i++) {
        _clock_membership[i] = _invalid_clock_index;
    }
}

template<class key, class value>
multi_clock<key, value>::~multi_clock() {
    _clocksize = 0;
    delete[](_values);
    delete[](_clocks);
    delete[](_clock_membership);
    
    _clocknumber = 0;
    delete[](_hands);
    delete[](_sizes);
}

template<class key, class value>
bool multi_clock<key, value>::get_head(clk_idx clock, 
                                       value &head_value) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        head_value = _values[_hands[clock]];
        if (_sizes[clock] >= 1) {
            w_assert1(_clock_membership[_hands[clock]] == clock);
            return true;
        } else {
            w_assert1(_hands[clock] == _invalid_index);
            return false;
        }
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::set_head(clk_idx clock, 
                                       value new_value) {
    if (clock >= 0 && clock <= _clocknumber - 1
                   && _sizes[clock] >= 1) {
        _values[_hands[clock]] = new_value;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::get_head_index(clk_idx clock, 
                                             key &head_index) {
    if (clock >= 0 && clock <= _clocknumber - 1) {
        head_index = _hands[clock];
        if (_sizes[clock] >= 1) {
            w_assert1(_clock_membership[_hands[clock]] == clock);
            return true;
        } else {
            w_assert1(head_index == _invalid_index);
            return false;
        }
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::move_head(clk_idx clock) {
    if (clock >= 0 && clock <= _clocknumber - 1
                   && _sizes[clock] >= 1) {
        _hands[clock] = _clocks[_hands[clock]]._after;
        w_assert1(_clock_membership[_hands[clock]] == clock);
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::add_tail(clk_idx clock, key index) {
    if (index >= 0 && index <= _clocksize - 1
     && index != _invalid_index && clock >= 0
     && clock <= _clocknumber - 1
     && _clock_membership[index] == _invalid_clock_index) {
        if (_sizes[clock] == 0) {
            _hands[clock] = index;
            _clocks[index]._before = index;
            _clocks[index]._after = index;
        } else {
            _clocks[index]._before = _clocks[_hands[clock]]._before;
            _clocks[index]._after = _hands[clock];
            _clocks[_clocks[_hands[clock]]._before]._after = index;
            _clocks[_hands[clock]]._before = index;
        }
        _sizes[clock]++;
        _clock_membership[index] = clock;
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::remove_head(clk_idx clock, 
                                          key &removed_index) {
    removed_index = _invalid_index;
    if (clock >= 0 && clock <= _clocknumber - 1) {
        removed_index = _hands[clock];
        if (_sizes[clock] == 0) {
            w_assert1(_hands[clock] == _invalid_index);
            return false;
        } else if (_clock_membership[removed_index] != clock) {
            return false;
        } else {
            w_assert0(remove(removed_index));
        }
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::remove(key &index) {
    if (index >= 0 && index <= _clocksize - 1
                   && index != _invalid_index
     && _clock_membership[index] != _invalid_clock_index) {
        clk_idx clock = _clock_membership[index];
        if (_sizes[clock] == 1) {
            w_assert1(_hands[clock] >= 0
                   && _hands[clock] <= _clocksize - 1
                   && _hands[clock] != _invalid_index);
            w_assert1(_clocks[_hands[clock]]._before
                                   == _hands[clock]);
            w_assert1(_clocks[_hands[clock]]._after
                                   == _hands[clock]);
            
            _clocks[index]._before = _invalid_index;
            _clocks[index]._after = _invalid_index;
            _hands[clock] = _invalid_index;
            _clock_membership[index]
                           = _invalid_clock_index;
            _sizes[clock]--;
            return true;
        } else {
            _clocks[_clocks[index]._before]._after
                            = _clocks[index]._after;
            _clocks[_clocks[index]._after]._before
                            = _clocks[index]._before;
            _hands[clock] = _clocks[index]._after;
            _clocks[index]._before = _invalid_index;
            _clocks[index]._after = _invalid_index;
            _clock_membership[index]
                            = _invalid_clock_index;
            _sizes[clock]--;
            
            w_assert1(_hands[clock] != _invalid_index);
            return true;
        }
    } else {
        return false;
    }
}

template<class key, class value>
bool multi_clock<key, value>::switch_head_to_tail(clk_idx source, 
                       clk_idx destination, key &moved_index) {
    moved_index = _invalid_index;
    if (_sizes[source] > 0
     && source >= 0 && source <= _clocknumber - 1
     && destination >= 0 && destination <= _clocknumber - 1) {
        w_assert0(remove_head(source, moved_index));
        w_assert0(add_tail(destination, moved_index));
        
        return true;
    } else {
        return false;
    }
}

template<class key, class value>
key multi_clock<key, value>::size_of(clk_idx clock) {
    return _sizes[clock];
}
