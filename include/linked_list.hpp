/**
 * File: linked_list.hpp
 * Author: Pavlos Spanoudakis (sdi1800184)
 */

#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

#include "utils.hpp"

class LinkedList {
    private:
        struct ListNode {
            ListNode* next;
            void* data;
            ListNode();
        };
        ListNode* head;
        ListNode* last;
        DestroyFunc destroy;

    public:
        class ListIterator
        {
            private:
                ListNode *node;
            public:
                ListIterator(ListNode *list_node);
                void* getData();
                void forward();
                bool isNull();
        };
        
        LinkedList(DestroyFunc dest);
        ~LinkedList();
        void append(void *element);
        void* getElement(void *element, CompareFunc compare);
        void *getLast();
        ListIterator listHead();
};

#endif
