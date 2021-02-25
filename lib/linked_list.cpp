#include <cstddef>
#include "../include/linked_list.hpp"

LinkedList::ListNode::ListNode(): next(NULL), data(NULL) {}

LinkedList::~LinkedList()
{
    ListNode *temp;
    while (this->head != NULL)
    {
        temp = head;
        head = head->next;
        delete temp;
    }    
}