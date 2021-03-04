#include <cstddef>
#include "../include/skip_list.hpp"

SkipList::SkipList(int layers, CompareFunc function):
max_layer(( (layers < MAXLAYERS) ? layers : MAXLAYERS )),
compare(function), layer_heads(new SkipListNode* [max_layer]), curr_layer(0)
{
    for ( int i = 0; i < max_layer; i++ )
    {
        layer_heads[i] = NULL;
    }
}

SkipList::~SkipList()
{
    SkipListNode* current = layer_heads[0];
    SkipListNode* prev;

    while ( current != NULL )
    {
        prev = current;
        current = current->next_nodes[0];
        delete prev;
    }
    delete [] layer_heads;
}

SkipList::SkipListNode::SkipListNode(int layers): next_nodes(new SkipListNode* [layers])
{
    for ( int i = 0; i < layers; i++ )
    {
        next_nodes[i] = NULL;
    }
}

SkipList::SkipListNode::~SkipListNode()
{
    delete [] next_nodes;
}

void* SkipList::find(void *element)
{
    return search(element, layer_heads[curr_layer], curr_layer);
}

void* SkipList::search(void *element, SkipListNode *start, int search_layer)
{
    if (start == NULL) { return NULL; }

    SkipListNode *prev = NULL;
    SkipListNode *curr = start;
    int cmp;

    do {
        cmp = compare(element, curr->data);
        if ( cmp > 0 ) { break; }
        if ( cmp == 0 ) { return curr->data; }

        prev = curr;
        curr = curr->next_nodes[search_layer];
    } while (curr != NULL);
    
    if (prev == NULL )
    // Start Node has value greater than target element
    {
        // So the element does not exist
        return NULL;
    }
    // Continue search one layer down
    if ( search_layer == 0 ) { return NULL; }
    return search(element, prev, search_layer - 1);
}

int SkipList::insert(void *element)
{
    // The nodes in each layer to place the new element after
    SkipListNode **layer_positions;

    layer_positions = new SkipListNode*[max_layer];
    for ( int i = 0; i < max_layer; i++ )
    {
        layer_positions[i] = NULL;
    }

    SkipListNode *current = layer_heads[curr_layer];
    SkipListNode *prev = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    {
        while (current != NULL)
        {
            prev = current;
            cmp = compare(element, current->data);

            if ( cmp == 0 ) { return 0; }
            if ( cmp > 0 ) { break; }
            current = current->next_nodes[curr_layer];
        }
        layer_positions[i] = prev;
    }
    SkipListNode *new_node = new SkipListNode(max_layer);
    new_node->data = element;
    
    int new_node_layer = getRandomLayer();
    for ( int i = 0; i < new_node_layer; i++ )
    {
        if ( layer_positions[i] == NULL )
        {        
            new_node->next_nodes[i] = layer_heads[i];
            layer_heads[i] = new_node;
        }
        else
        {
            new_node->next_nodes[i] = layer_positions[i]->next_nodes[i];
            layer_positions[i]->next_nodes[i] = new_node;
        }
    }
    return 1;
}

void SkipList::remove(void *element)
{

}