/**
 * File: linked_list.hpp
 * Author: Pavlos Spanoudakis (sdi1800184)
 */

#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

#include "utils.hpp"

class LinkedList {
    protected:
        struct ListNode {
            ListNode* next;
            void* data;
            ListNode();
        };
        ListNode* head;
        ListNode* last;
        CompareFunc compare;
        DestroyFunc destroy;

    public:
        LinkedList(CompareFunc comp, DestroyFunc dest);
        ~LinkedList();
        void append(void *element);
        void* getElement(void *element);
};

#endif
