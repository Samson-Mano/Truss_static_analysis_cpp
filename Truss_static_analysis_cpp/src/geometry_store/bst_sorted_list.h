#pragma once
#include <unordered_map>
#include <iostream>
#include <string>

struct bst_Node 
{
	int id;
	long int x, y;
	bst_Node* left;
	bst_Node* right;
};


class bst_sorted_list
{
public:
	const int precision = 1000000;
	int count;
	bst_Node* root_ordered;
	bst_Node* root;
	bst_sorted_list();
	~bst_sorted_list();
	void init();
	bst_Node* newNode(const int& id,const long int& x, const long int& y);
	bst_Node* insert(bst_Node* root, const int& id, const long int& x, const long int& y);
	void add_Node(const int& id, const double& dbl_x, const double dbl_y);
	void finalize_list(unsigned int node_count);
	void inorder(bst_Node* root,int& i);
};