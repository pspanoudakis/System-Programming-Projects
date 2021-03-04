#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include "../include/skip_list.hpp"

SkipList::SkipList(int layers, CompareFunc comp, DestroyFunc dest):
max_layer(( (layers < MAXLAYERS) ? layers : MAXLAYERS )),
compare(comp), layer_heads(new SkipListNode* [max_layer]), curr_layer(0), destroyElement(dest)
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
        destroyElement(prev->data);
        delete prev;
    }
    delete [] layer_heads;
}

SkipList::SkipListNode::SkipListNode(int total_layers): next_nodes(new SkipListNode* [layers]), layers(total_layers)
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
    SkipListNode *current;
    SkipListNode *prev = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    {
        if ( prev == NULL )
        {
            current = layer_heads[i];
        }
        else
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            {
                return current->data;
            }
            if ( cmp > 0 )
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            break;
        }
    }
    return NULL;
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

    SkipListNode *current;
    SkipListNode *prev = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    {
        if ( prev == NULL )
        {
            current = layer_heads[i];
        }
        else
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            { 
                delete [] layer_positions;
                return 0;
            }
            if ( cmp > 0 )
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            break;
        }
        layer_positions[i] = prev;
    }
    SkipListNode *new_node = new SkipListNode(max_layer);
    new_node->data = element;
    
    int new_node_layer = getRandomLayer();
    for ( int i = 0; i <= new_node_layer; i++ )
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
    if ( curr_layer < new_node_layer )
    {
        curr_layer = new_node_layer;
    }
    delete [] layer_positions;
    return 1;
}

void SkipList::remove(void *element)
{
    // The nodes in each layer that are before the element node
    SkipListNode **layer_prevs;

    layer_prevs = new SkipListNode*[max_layer];
    for ( int i = 0; i < max_layer; i++ )
    {
        layer_prevs[i] = NULL;
    }

    SkipListNode *current;
    SkipListNode *prev = NULL;
    SkipListNode *target = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    {
        if ( prev == NULL )
        {
            current = layer_heads[i];
        }
        else
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            { 
                target = current;
                break;
            }
            if ( cmp > 0 )
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            break;
        }
        layer_prevs[i] = prev;
    }
    if ( target == NULL ) { return; }

    for ( int i = 0; i < target->layers; i++ )
    {
        if ( layer_prevs[i] == NULL )
        {
            layer_heads[i] = target->next_nodes[i];
        }
        else
        {
            layer_prevs[i]->next_nodes[i] = target->next_nodes[i];
        }
    }
    destroyElement(target->data);
    delete target;
    delete [] layer_prevs;
}

int SkipList::getRandomLayer(void)
{
    int limit = ( ( (curr_layer + 1) < max_layer) ? (curr_layer + 1) : max_layer );
    for ( int i = 0; i <= limit; i++ )
    {
        if ( rand() % 2 == 1 ) { return i; }
    }
    return limit;
}

void SkipList::display(DisplayFunc f)
{
    SkipListNode *current;
    for (int i = curr_layer; i >= 0; i--)
    {
        current = layer_heads[i];
        while (current != NULL)
        {
            f(current->data);
            printf(" -> ");
            current = current->next_nodes[i];
        }
        printf("\n");
    }
}