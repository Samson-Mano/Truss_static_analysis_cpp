#include "bst_sorted_list.h"

bst_sorted_list::bst_sorted_list()
	:root(nullptr), count(0), root_ordered(nullptr)
{
	// Empty constructor
}

bst_sorted_list::~bst_sorted_list()
{
	// Destructor
}

void bst_sorted_list::init()
{
	// reinitialize the data
	root = nullptr;
	root_ordered = nullptr;
	count = 0;
}


bst_Node* bst_sorted_list::newNode(const int& id, const long int& x, const long int& y)
{
	// Add new node
	bst_Node* node = new bst_Node;
	node->id = id;
	node->x = x;
	node->y = y;
	node->left = nullptr;
	node->right = nullptr;
	return node;
}

bst_Node* bst_sorted_list::insert(bst_Node* root, const int& id, const long int& x, const long int& y)
{
	if (root == nullptr)
	{
		// Add first item
		return newNode(id, x, y);
	}

	if (x < root->x)
	{
		// x is less than the last added item. 
		// Add at the left
		root->left = insert(root->left, id, x, y);
	}
	else if (x > root->x)
	{
		// x is greater than the last added item. 
		// Add at the right
		root->right = insert(root->right, id, x, y);
	}
	else if (y < root->y)
	{
		// Sort by y because both x are same
		// y is less than the last added item. 
		// Add at the left
		root->left = insert(root->left, id, x, y);
	}
	else if (y > root->y)
	{
		// Sort by y because both x are same
		// y is greater than the last added item. 
		// Add at the right
		root->right = insert(root->right, id, x, y);
	}
	return root;
}

void bst_sorted_list::add_Node(const int& id, const double& dbl_x, const double dbl_y)
{
	// Add Node
	long int x, y;

	x = static_cast<int>(dbl_x * precision);
	y = static_cast<int>(dbl_y * precision);

	// std::cout << x << ", " << y << std::endl;
	if (count == 0)
	{
		root = insert(root, id, x, y);
	}
	else
	{
		insert(root, id, x, y);
	}
	count++;
}

void bst_sorted_list::finalize_list(unsigned int node_count)
{
	// Function which sets the zero ptr
	root_ordered = new bst_Node[node_count];
	int i = 0;
	inorder(root,i);

	std::cout << "__________________________________________________________" << std::endl;
	std::cout << "__________________________________________________________" << std::endl;

	for (int i = 0; i < count; i++)
	{
		std::cout << root_ordered[i].x << ", " << root_ordered[i].y << std::endl;
	}
}

void bst_sorted_list::inorder(bst_Node* root,int& i)
{
	if (root == nullptr)
	{
		return;
	}
	inorder(root->left,i);
	std::cout << root->x << ", " << root->y << std::endl;
	root_ordered[i] = *root;
	i++;
	inorder(root->right,i);
}