#include <cstddef>
#include "../include/linked_list.hpp"

LinkedList::ListNode::ListNode(): next(NULL), data(NULL) {}

LinkedList::LinkedList(CompareFunc comp, DestroyFunc dest):
 head(NULL), last(NULL), compare(comp), destroy(dest) { }

LinkedList::~LinkedList()
{
    ListNode *temp;
    while (this->head != NULL)
    {
        temp = head;
        head = head->next;
        destroy(temp->data);
        delete temp;
    }    
}

void* LinkedList::getElement(void *element)
{
    ListNode* current = this->head;
    while (current != NULL)
    {
        if ( compare(element, current->data) == 0 )
        {
            return current->data;
        }
        current = current->next;
    }
    return NULL;
}

void LinkedList::append(void *element)
{
    if (last != NULL)
    {
        last->next = new ListNode();
        last->next->data = element;
        last = last->next;
        return;
    }

    // List has no elements
    head = new ListNode();
    head->data = element;
    last = head;    
}

bool LinkedList::insert(void *element)
{
    if ( this->head == NULL )
    {
        this->head = new ListNode();
        this->head->data = element;
        return true;
    }
    ListNode* current = this->head;
    while (current->next != NULL)
    {
        if ( compare(element, current->data) == 0 )
        {
            return false;
        }
        current = current->next;
    }
    current->next = new ListNode();
    current->next->data = element;
    return true;