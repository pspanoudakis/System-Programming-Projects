/**
 * File: rb_tree.cpp
 * Red-Black Tree implementation.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstddef>
#include "../include/rb_tree.hpp"

/**
 * @brief Creates a new Red-Black Tree Node with the specified information.
 *
 * @param c The color of the new Node.
 * @param element The data to be stored in the new Node.
 * @param parent_node The parent Node of the new Node.
 */
RBTreeNode::RBTreeNode(Color c, void *element, RBTreeNode *parent_node):
color(c), data(element), left(NULL), right(NULL), parent(parent_node) { }

/**
 * Returns the sibling of the Node. If there is no sibling, NULL is returned.
 */
RBTreeNode* RBTreeNode::sibling()
{
    if (this->parent == NULL) { return NULL; }
    if (this->isLeftChild())
    {
        return this->parent->right;
    }
    return this->parent->left;
}

/**
 * Deletes all the descendants of the Node recursively.
 */
void RBTreeNode::destroyDescendants()
{
    if (this->left != NULL)
    // If there is a Left Child, destroy its descendants and itself
    {
        this->left->destroyDescendants();
        delete left;
    }
    if (this->right != NULL)
    // If there is a Right Child, destroy its descendants and itself
    {
        this->right->destroyDescendants();
        delete right;
    }
}

/**
 * Deletes all the descendants of the Node recursively.
 * The Nodes data will also be deleted using the specified destroy function.
 */
void RBTreeNode::destroyDescendants(DestroyFunc destroy)
{
    if (this->left != NULL)
    // If there is a Left Child, destroy its descendants and itself
    {
        this->left->destroyDescendants(destroy);
        destroy(this->left->data);
        delete left;
        this->left = NULL;
    }
    if (this->right != NULL)
    // If there is a Right Child, destroy its descendants and itself
    {
        this->right->destroyDescendants(destroy);
        destroy(this->right->data);
        delete right;
        this->right = NULL;
    }
}

/**
 * Indicates whether the Node is a Left Child.
 */
bool RBTreeNode::isLeftChild()
{
    if (this->parent == NULL) { return false; }

    return (this->parent->left == this);
}

/**
 * Swaps the colors between the Node and other.
 */
void RBTreeNode::swapColor(RBTreeNode *other)
{
    Color temp = this->color;
    this->color = other->color;
    other->color = temp;
}

/**
 * Replaces the Node with new_parent, which becomes parent of the Node.
 * Whether the Node will be Left or Right child must be set externally.
 */
void RBTreeNode::replaceWithNewParent(RBTreeNode *new_parent)
{
    if (this->parent != NULL)
    {
        if ( this->isLeftChild() )
        // Replace Node with new_parent
        {
            this->parent->left = new_parent;
        }
        else
        {
            this->parent->right = new_parent;
        }
    }
    new_parent->parent = this->parent;
    // make new_parent the parent of the Node
    this->parent = new_parent;
    
}

/**
 * Creates a Red-Black Tree, which will use comp function for comparisons.
 */
RedBlackTree::RedBlackTree(CompareFunc comp):
num_elements(0), compare(comp), root(NULL) { }

RedBlackTree::~RedBlackTree()
{
    if (root != NULL)
    {
        root->destroyDescendants();
        delete root;
    }
}

/**
 * Performs Left Rotation operation.
 * The target node becomes left child of its right child.
 */
void RedBlackTree::leftRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->right;

    if (this->root == target)
    // If target is the root, change tree root to new_parent
    {
        this->root = new_parent;
    }
    // Properly replace and push target a level down
    target->replaceWithNewParent(new_parent);
    target->right = new_parent->left;
    // new_parent left child will become target's right child.
    if (target->right != NULL)
    {
        target->right->parent = target;
    }
    new_parent->left = target;
}

/**
 * Performs Right Rotation operation.
 * The target node becomes right child of its left child.
 */
void RedBlackTree::rightRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->left;

    if (this->root == target)
    // If target is the root, change tree root to new_parent
    {
        this->root = new_parent;
    }
    // Properly replace and push target a level down
    target->replaceWithNewParent(new_parent);
    target->left = new_parent->right;
    // new_parent right child will become target's left child.
    if (target->left != NULL)
    {
        target->left->parent = target;
    }
    new_parent->right = target;
}

/**
 * Takes care of a potential double Red Node violation
 * (both target and its parent are RED).
 */
void RedBlackTree::fixRedRedViolation(RBTreeNode *target)
{
    if ( this->root == target )
    // target is the root, so just make it BLACK
    {
        target->color = BLACK;
        return;
    }
    RBTreeNode *parent = target->parent;
    // The parent node is BLACK, so there is no violation
    if (parent->color == BLACK) { return; }

    RBTreeNode *uncle = parent->sibling();

    // We know that parent is not BLACK 
    // (so it's not the root), so grandparent exists
    RBTreeNode *grandparent = parent->parent;

    if (uncle == NULL || (uncle->color == BLACK))
    {
        if (parent->isLeftChild())
        {
            if (target->isLeftChild())
            // Left-Left case
            {
                rightRotation(grandparent);
                parent->swapColor(grandparent);
            }
            else
            // Left-Right case
            {
                leftRotation(parent);
                rightRotation(grandparent);
                target->swapColor(grandparent);
            }            
        }
        else
        {
            if (target->isLeftChild())
            // Right-Left case
            {
                rightRotation(parent);
                leftRotation(grandparent);
                target->swapColor(grandparent);
            }
            else
            // Right-Right case
            {
                leftRotation(grandparent);
                parent->swapColor(grandparent);
            }            
        }        
    }
    else
    // uncle is RED
    {
        // do proper recoloring
        parent->color = BLACK;
        uncle->color = BLACK;
        grandparent->color = RED;
        // Now the violation has been resolved in this level,
        // but it might have occured in grandparent's level, so take care of it.
        fixRedRedViolation(grandparent);
    }
}

/**
 * Inserts the element recursively in the subtree with start as root.
 * If an equal element is already present, the Tree will remain untouched.
 */
void RedBlackTree::recursiveInsert(RBTreeNode *start, void *element)
{
    int cmp = compare(element, start->data);

    // If an equal element is present, just return.
    if (cmp == 0) { return; }
    
    if (cmp > 0)
    // The element is greater than the subtree root,
    {
        if (start->right == NULL)
        // If there is no right child, create one and store the element there.
        {
            start->right = new RBTreeNode(RED, element, start);
            fixRedRedViolation(start->right);       // There might be a color violation, so take care of it.
            this->num_elements++;
            return;
        }
        // Otherwise insert in Right Child subtree.
        recursiveInsert(start->right, element);
    }
    else
    // The element is smaller than the subtree root,
    {
        if (start->left == NULL)
        // If there is no left child, create one and store the element there.
        {
            start->left = new RBTreeNode(RED, element, start);
            fixRedRedViolation(start->left);        // There might be a color violation, so take care of it.
            this->num_elements++;
            return;
        }
        // Otherwise insert in Right Child subtree.
        recursiveInsert(start->left, element);
    }
}

/**
 * Inserts the specified element in the Tree. If an equal element is already present,
 * the Tree will remain untouched.
 */
void RedBlackTree::insert(void *element)
{
    if (root == NULL)
    // If the Tree is empty just insert in root and return
    {
        root = new RBTreeNode(BLACK, element, NULL);
        this->num_elements++;
        return;
    }
    // Otherwise, insert in recursive fashion, starting from the root.
    recursiveInsert(root, element);
}

/**
 * Search for an element equal to the specified one in the Tree,
 * based on the Tree comparison function.
 * @returns The found element, or NULL if it was not found. 
 */
void* RedBlackTree::search(void *element)
{
    int cmp;
    RBTreeNode *current;
    current = this->root;

    while (current != NULL)
    // Start searching from the root
    {
        cmp = compare(element, current->data);
        if (cmp == 0)
        // Return when found
        {
            return current->data;
        }
        // Search either left or right based on the comparison result
        else if(cmp > 0)
        {
            current = current->right;
        }
        else
        {
            current = current->left;   
        }
    }
    return NULL;
}

/**
 * Returns the number of elements in the Tree.
 */
unsigned int RedBlackTree::getNumElements()
{
    return this->num_elements;
}

/**
 * Deletes the Tree structure, but also destroys the node data using
 * the specified destroy function.
 */
void RedBlackTree::clear(DestroyFunc destroy)
{
    if (this->root != NULL)
    {
        this->root->destroyDescendants(destroy);
        destroy(this->root->data);
        delete this->root;
        this->root = NULL;
    }
}
