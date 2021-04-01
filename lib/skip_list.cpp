/**
 * File: skip_list.cpp
 * Skip List implementation.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include "../include/skip_list.hpp"

/**
 * @brief Creates a Skip List with the given parameters.
 * @param layers The maximum number of levels for the Skip List.
 * @param dest The function to be used for destroying elements when the Skip List is being destroyed.
 */
SkipList::SkipList(int layers, DestroyFunc dest):
max_layer(( (layers < SKIP_LIST_MAX_LAYERS) ? layers : SKIP_LIST_MAX_LAYERS )), curr_layer(0), 
layer_heads(new SkipListNode* [max_layer]), destroyElement(dest)
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

    // All nodes are present in the lower level, so we can delete them with a simple iteration.
    while ( current != NULL )
    {
        prev = current;
        current = current->next_nodes[0];
        destroyElement(prev->data);
        delete prev;
    }
    delete [] layer_heads;
}

/**
 * @brief Creates a Skip List Node.
 * @param total_layers The number of levels that this Node can be connected to.
 */
SkipList::SkipListNode::SkipListNode(int total_layers): 
layers(total_layers), next_nodes(new SkipListNode* [layers]), data(NULL)
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

/**
 * Searches the specified element in the Skip List.
 * 
 * @param element The element to be searched.
 * @param compare The function used for comparing the specified element
 * and the existing elements.
 * 
 * @return The element that was found if the search was successful, otherwise NULL.
 */
void* SkipList::find(void *element, CompareFunc compare)
{
    // The current node.
    SkipListNode *current;
    // The last found node with an element smaller than the specified one.
    SkipListNode *prev = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    // Search all layers, starting from the higher.
    {
        if ( prev == NULL )
        // No element smaller than the specified one has been found,
        {
            // So start searching from the beginning of the level.
            current = layer_heads[i];
        }
        else
        // Such element has been found, so start searching after its node.
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        // Iterate over the layer nodes
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            // Element found
            {
                return current->data;
            }
            if ( cmp > 0 )
            // The current element is smaller, so store it and keep searching in this layer.
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            // The current element is greater, so go to the next layer.
            break;
        }
    }
    // The element was not found.
    return NULL;
}

/**
 * Inserts the specified element in the Skip List.
 * 
 * @param element The element to be inserted.
 * @param present If the element was already present in the Skip List,
 * this will point to the existing data after the end of the execution.
 * @param compare The function used for comparing the specified element
 * and the existing elements.
 * 
 * @return TRUE if the element was inserted, FALSE otherwise.
 */
bool SkipList::insert(void *element, void **present, CompareFunc compare)
{
    // The nodes in each layer to place the new element after
    SkipListNode **layer_positions;

    // Initialize all to NULL at first
    layer_positions = new SkipListNode*[max_layer];
    for ( int i = 0; i < max_layer; i++ )
    {
        layer_positions[i] = NULL;
    }

    // The current node.
    SkipListNode *current;
    // The last found node to place the element after.
    SkipListNode *prev = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    // Iterate over the layers, starting from the higher.
    {
        if ( prev == NULL )
        // No element smaller than the specified one has been found,
        {
            // So start from the beginning of the level.
            current = layer_heads[i];
        }
        else
        // Such element has been found, so start searching after its node.
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        // Iterate over the layer nodes
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            // The specified element is already present, so store and return.
            {
                *present = current->data; 
                delete [] layer_positions;
                return false;
            }
            if ( cmp > 0 )
            // The current element is smaller, so store it and continue in this layer.
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            // The current element is greater, so go to the next layer.
            break;
        }
        // Store the last found node to place the element after (in this layer).
        layer_positions[i] = prev;
    }
    // Create the new node
    int new_node_layer = getRandomLayer();
    SkipListNode *new_node = new SkipListNode(new_node_layer + 1);
    new_node->data = element;
    
    // Inserting the target node to all the layers where it will be present.
    for ( int i = 0; i <= new_node_layer; i++ )
    {
        if ( layer_positions[i] == NULL )
        // No element smaller than the specified was found,
        {
            // So place the node at the beginning of the layer.        
            new_node->next_nodes[i] = layer_heads[i];
            layer_heads[i] = new_node;
        }
        else
        // A least smaller element was found,
        {
            // So insert the node right after it.
            new_node->next_nodes[i] = layer_positions[i]->next_nodes[i];
            layer_positions[i]->next_nodes[i] = new_node;
        }
    }
    // Update Current Skip List layer, in case it is needed.
    if ( curr_layer < new_node_layer )
    {
        curr_layer = new_node_layer;
    }
    delete [] layer_positions;
    *present = NULL;
    return true;
}

/**
 * Deletes the specified element from the Skip List.
 * 
 * @param element The element to be deleted.
 * @param present If the element was present in the Skip List,
 * this will point to the existing data after the end of the execution.
 * Otherwise, it is set to NULL.
 * @param compare The function used for comparing the specified element
 * and the existing elements.
 */
void SkipList::remove(void *element, void **present, CompareFunc compare)
{
    // The nodes in each layer that are just before
    // the target element node (in ordering)
    SkipListNode **layer_prevs;

    // Initialize all to NULL at first
    layer_prevs = new SkipListNode*[max_layer];
    for ( int i = 0; i < max_layer; i++ )
    {
        layer_prevs[i] = NULL;
    }

    // The current node.
    SkipListNode *current;
    // The last found node with an element smaller than the specified one.
    SkipListNode *prev = NULL;
    // If the element is found, its node will be stored here.
    SkipListNode *target = NULL;
    *present = NULL;
    int cmp;
    for (int i = curr_layer; i >= 0; i--)
    // Search all layers, starting from the higher.
    {
        if ( prev == NULL )
        // No element smaller than the specified one has been found,
        {
            // So start searching from the beginning of the level.
            current = layer_heads[i];
        }
        else
        // Such element has been found, so start searching after its node.
        {
            current = prev->next_nodes[i];
        }
        while (current != NULL)
        // Iterate over the layer nodes
        {
            cmp = compare(element, current->data);

            if ( cmp == 0 )
            // Element found
            { 
                target = current;
                break;
            }
            if ( cmp > 0 )
            // The current element is smaller, so store it and keep searching in this layer.
            {
                prev = current;
                current = current->next_nodes[i];
                continue;
            }
            // The current element is greater, so go to the next layer.
            break;
        }
        // Store the least smaller element found in this layer.
        layer_prevs[i] = prev;
    }
    if ( target == NULL )
    // The specified element was not found, so nothing more to do.
    { 
        delete [] layer_prevs;
        return; 
    }

    // Deleting the target node from all the layers where it is present.
    for ( int i = 0; i < target->layers; i++ )
    {
        if ( layer_prevs[i] == NULL )
        // No element smaller than the specified was found,
        {
            // So the node is the first in the layer,
            // and must be skipped by the layer head.
            layer_heads[i] = target->next_nodes[i];
        }
        else
        // A least smaller element was found,
        {
            // So skip the node properly.
            layer_prevs[i]->next_nodes[i] = target->next_nodes[i];
        }
    }
    // Store the found data and delete the node.
    *present = target->data;
    delete target;
    delete [] layer_prevs;
}

/**
 * Returns a random layer number. The number can be between 0 and current level + 1,
 * but never above the maximum layer allowed.
 */
int SkipList::getRandomLayer(void)
{
    int limit = ( ( (curr_layer + 1) < max_layer) ? (curr_layer + 1) : max_layer );
    for ( int i = 0; i < limit; i++ )
    {
        if ( rand() % 2 == 1 ) { return i; }
    }
    return limit - 1;
}

/**
 * Displays all the elements of the Skip List, by simply 
 * iterating over the nodes of layer 0.
 * @param print The function used for printing the node data.
 */
void SkipList::displayElements(DisplayFunc print)
{
    SkipListNode *current = layer_heads[0];

    while (current != NULL)
    {
        print(current->data);
        current = current->next_nodes[0];
    }    
}
