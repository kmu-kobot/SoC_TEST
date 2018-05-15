#include "stdafx.h"
#include "KDTree.h"

Node::Node(feature_t* key, int axis)
	:axis(axis), value(key->vec[axis]), left(NULL), right(NULL), key(key)
{
}

Node::~Node()
{
	if (left != NULL)
		delete left;
	if (right != NULL)
		delete right;
}

CKDTree::CKDTree(int d)
	:dimension(d)
{
}

CKDTree::CKDTree(Node root, int d)
	:root(&root), dimension(d)
{
}

CKDTree::~CKDTree()
{
	delete root;
}

void sort(std::vector<feature_t>& feature, int left, int right, int axis)
{
	int i = left, j = right;
	float pivot = feature[(left + right) >> 1].vec[axis];
	feature_t temp;

	do
	{
		while (feature[i].vec[axis] <= pivot)
			++i;
		while (feature[j].vec[axis] > pivot)
			--j;
		if (i <= j)
		{
			std::iter_swap(feature.begin() + i++, feature.begin() + j--);
		}
	} while (i <= j);

	if (left < j)
		sort(feature, left, j, axis);

	if (i < right)
		sort(feature, i, right, axis);
}

void CKDTree::buildKDTree(std::vector<feature_t>& feature, Node*& n, int left, int right, int axis)
{
	sort(feature, left, right, axis);
	int pivot = (left + right) >> 1;
	float pivotValue = feature[pivot].vec[axis];

	if (pivotValue == feature[right].vec[axis])
	{
		if (pivotValue != feature[left].vec[axis])
		{
			while (feature[--pivot].vec[axis] == pivotValue);
		}
	}
	else
	{
		while (feature[++pivot].vec[axis] == pivotValue);
	}

	n = new Node(&feature[pivot], (++axis) & 127);

	if (left < pivot)
		buildKDTree(feature, n->left, left, pivot - 1, axis - 1);
	if (pivot < right)
		buildKDTree(feature, n->right, pivot + 1, right, axis - 1);
}

void CKDTree::NNSearch(const feature_t& q, Node* n, feature_t*& p, float& d1, float& d2)
{
	bool search_first;
	if (n->left == NULL && n->right == NULL)
	{
		float w = _CalcSIFTSqDist(q, *p);
		if (w < d1)
		{
			d2 = d1;
			d1 = w;
			p = n->key;
		}
		else if (w < d2)
		{
			d2 = w;
		}
	}
	else
	{
		if (q.vec[n->axis] < n->value)
		{
			if (q.vec[n->axis] - d2 <= n->value)
				NNSearch(q, n->left, p, d1, d2);
			if (q.vec[n->axis] + d2 > n->value)
				NNSearch(q, n->right, p, d1, d2);
		}
		else
		{
			if (q.vec[n->axis] + d2 > n->value)
				NNSearch(q, n->right, p, d1, d2);
			if (q.vec[n->axis] - d2 <= n->value)
				NNSearch(q, n->left, p, d1, d2);
		}
	}
}