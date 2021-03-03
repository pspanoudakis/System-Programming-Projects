#include <cstddef>
#include "../include/skip_list.hpp"

SkipList::SkipList(CompareFunc function): compare(function)
{
}

SkipList::~SkipList()
{
}

SkipList::SkipListNode::SkipListNode() {}

SkipList::SkipListNode::~SkipListNode() {}

void* SkipList::find(void *element)
{
    return search(element, layers);
}

void* SkipList::search(void *element, SkipListNode *start)
{
    if (start == NULL) { return NULL; }

    SkipListNode *prev = NULL;
    SkipListNode *curr = start;
    int cmp;

    do
    {
        cmp = compare(element, curr->data);
        if ( cmp > 0 ) { break; }
        if ( cmp == 0 ) { return curr->data; }

        prev = curr;
        curr = curr->next;
    } while (curr != NULL);
    
    if (prev == NULL )
    // Start Node has value greater than target element
    {
        // So the element does not exist
        return NULL;
    }
    // Continue search one layer down
    return search(element, prev->down);
}

void SkipList::insert(void *element)
{

}