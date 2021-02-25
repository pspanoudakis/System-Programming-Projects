/**
 * File: linked_list.hpp
 * Author: Pavlos Spanoudakis (sdi1800184)
 */

class LinkedList {
    protected:
        struct ListNode {
            ListNode* next;
            void* data;
            ListNode();
        };
        ListNode* head;

    public:
        LinkedList();
        ~LinkedList();
    
};