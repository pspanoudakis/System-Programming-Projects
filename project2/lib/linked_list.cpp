/**
 * File: linked_list.cpp
 * Linked List implementation.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstddef>
#include "../include/linked_list.hpp"

/**
 * Creates a null-initialized List Node.
 */
LinkedList::ListNode::ListNode(): next(NULL), data(NULL) {}

/**
 * Linked List methods ----------------------------------------------------------------------------
 */

/**
 * Create a Linked List.
 * @param dest The function to be used for destroying elements when the list is being destroyed.
 */
LinkedList::LinkedList(DestroyFunc dest):
 head(NULL), last(NULL), destroy(dest), numElements(0) { }

LinkedList::~LinkedList()
{
    ListNode *temp;
    while (this->head != NULL)
    // Iterate over the list until no elements are left
    {
        temp = head;
        // Skip the head node
        head = head->next;
        // Delete the previous node data, and the node itself
        destroy(temp->data);
        delete temp;
    }    
}

/**
 * Returns the first list element which is equal to the specified one,
 * based on the provided comparison function. Returns NULL if such element is not found.
 */
void* LinkedList::getElement(void *element, CompareFunc compare) const
{
    ListNode* current = this->head;
    while (current != NULL)
    // Iterate over the list
    {
        if ( compare(element, current->data) == 0 )
        // If an equal element is found, return it.
        {
            return current->data;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Add the specified element at the end of the list.
 */
void LinkedList::append(void *element)
{
    if (last != NULL)
    // Create a new node and place it after the currently last node.
    {
        last->next = new ListNode();
        last->next->data = element;
        last = last->next;
        this->numElements++;
        return;
    }

    // List has no elements
    head = new ListNode();
    head->data = element;
    last = head;
    this->numElements++;    
}

/**
 * Returns the last element in the list.
 */
void* LinkedList::getLast() const
{
    return last->data;
}

bool LinkedList::isEmpty() const
{
    return this->head == NULL;
}

/**
 * Returns an iterator to the head of the list.
 */
LinkedList::ListIterator LinkedList::listHead() const
{
    return ListIterator(this->head);
}

/**
 * Returns an iterator to the last element of the list.
 */
LinkedList::ListIterator LinkedList::listLast() const
{
    return ListIterator(this->last);
}

unsigned int LinkedList::getNumElements() const
{
    return this->numElements;
}

/**
 * List Iterator methods --------------------------------------------------------------------------
 */

/**
 * Creates an iterator to the specified node.
 */
LinkedList::ListIterator::ListIterator(LinkedList::ListNode *list_node):
node(list_node) { }

LinkedList::ListIterator::ListIterator(const ListIterator &itr):
node(itr.node) { }

/**
 * Returns the data of the node pointed by the iterator.
 */
void* LinkedList::ListIterator::getData() const
{
    if (this->node == NULL)
    {
        return NULL;
    }
    return this->node->data;
}

/**
 * Make the iterator point to the next list node.
 * If the iterator does not point to a node, do nothing.
 */
void LinkedList::ListIterator::forward()
{
    if (this->node != NULL)
    {
        this->node = this->node->next;
    }
}

/**
 * Returns TRUE if the iterator points to null, FALSE otherwise.
 */
bool LinkedList::ListIterator::isNull() const
{
    return (this->node == NULL);
}
