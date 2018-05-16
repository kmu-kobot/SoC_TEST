#pragma once
#include <stack>
#include <vector>
#include <string>
#include "Feature.h"

class Node
{
public:
	Node();
	Node(feature_t *key, int axis);
	~Node();

	int axis;
	Node *left, *right;
	float value;
	feature_t *key;
};

class CKDTree
{
public:
	CKDTree();
	CKDTree(int d);
	CKDTree(Node* root, int d);
	~CKDTree();

	void buildKDTree(std::vector<feature_t>& feature, Node*& n, int left, int right, int axis);
	void NNSearch(feature_t& q, Node* n, feature_t*& p, float& d1, float& d2);

	int dimension;
	Node *root;
};