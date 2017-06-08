const uint32_t SWIZZLED_PID_BIT = 0x80000000;

class bf_tree_m {

public:
    static bool is_swizzled_pointer (PageID pid) {
        return (pid & SWIZZLED_PID_BIT) != 0;
    }

}
