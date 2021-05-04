/**
 * File: linked_list.hpp
 * Linked List definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

#include "utils.hpp"

/**
 * A typical single-linked List, with an extra pointer 
 * to the last element to allow O(1) insertion.  
 */
class LinkedList {
    private:
        /**
         * A typical single-linked List Node.
         */
        struct ListNode {
            ListNode* next;
            void* data;
            ListNode();
        };
        ListNode* head;
        ListNode* last;
        DestroyFunc destroy;    // Used for destroying elements when the list is being destroyed.
        int num_elements;

    public:
        /**
         * Used for iterating over list elements.
         */
        class ListIterator
        {
            private:
                ListNode *node;     // The actual list node.
            public:
                ListIterator(ListNode *list_node);
                ListIterator(const ListIterator &itr);
                void* getData();
                void forward();
                bool isNull();
        };
        
        LinkedList(DestroyFunc dest);
        ~LinkedList();
        void append(void *element);
        void* getElement(void *element, CompareFunc compare);
        void *getLast();
        bool isEmpty();
        ListIterator listHead();
        ListIterator listLast();
};

#endif
