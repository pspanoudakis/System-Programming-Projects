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
        DestroyFunc destroy;

    public:
        LinkedList(DestroyFunc dest);
        ~LinkedList();
        void append(void *element);
        void* getElement(void *element, CompareFunc compare);
        void *getLast();
};

#endif
