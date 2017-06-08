#include <unordered_map>

class hashtable_queue {
private:
    class key_pair {
    public:
        key_pair() {}
        key_pair(key previous, key next) {
            this->_previous = previous;
            this->_next = next;
        }
        virtual ~key_pair() {}
        
        key     _previous;
        key     _next;
    };
    
    // the actual queue of keys:
    std::unordered_map<key, key_pair>*      _direct_access_queue;
    // the back of the list (MRU-element); insert here:
    key                                     _back;
    // the front of the list (LRU-element); remove here:
    key                                     _front;
    
    // index value with NULL semantics:
    key                                     _invalid_key;

public:
    hashtable_queue(key invalid_key);
    virtual         ~hashtable_queue();
    
    bool            contains(key k);
    bool            insert_back(key k);
    bool            remove_front();
    bool            remove(key k);
    u_int32_t       length();
};