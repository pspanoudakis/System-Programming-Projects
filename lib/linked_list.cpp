#include <cstddef>
#include "../include/linked_list.hpp"

LinkedList::ListNode::ListNode(): next(NULL), data(NULL) {}

LinkedList::LinkedList(DestroyFunc dest):
 head(NULL), last(NULL), destroy(dest) { }

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

void* LinkedList::getElement(void *element, CompareFunc compare)
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

void* LinkedList::getLast()
{
    return last->data;
}

/**
 * List Iterator methods --------------------------------------------------------------------------
 */

LinkedList::ListIterator LinkedList::listHead()
{
    return ListIterator(this->head);
}

LinkedList::ListIterator::ListIterator(LinkedList::ListNode *list_node):
node(list_node) { }

LinkedList::ListIterator::ListIterator(const ListIterator &itr):
node(itr.node) { }

void* LinkedList::ListIterator::getData()
{
    if (this->node == NULL)
    {
        return NULL;
    }
    return this->node->data;
}

void LinkedList::ListIterator::forward()
{
    if (this->node != NULL)
    {
        this->node = this->node->next;
    }
}

bool LinkedList::ListIterator::isNull()
{
    return (this->node == NULL);
}
