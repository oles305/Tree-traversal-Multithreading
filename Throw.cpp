//#include<bits/stdc++.h>
/*not use
void createLayerList(uint count)
{
	default_random_engine generator;
	uniform_int_distribution<int> distribution(1, INT_MAX);
	list<Node*> layerList;
	while (count--)
	{
		layerList.push_back(new Node(distribution(generator)));
	}
}*/
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <map>
#include <chrono>
#include <functional>

using namespace std;
using namespace std::placeholders;
typedef unsigned int   uint;
typedef unsigned char  uint8;
typedef unsigned long long ulong;

struct Node
{
	Node* left = nullptr;
	Node* right = nullptr;
	ulong data = 0;
	Node(ulong val) :data(val), left(nullptr), right(nullptr) {}
	~Node() {
		if (nullptr != left)
			delete left;
		if (nullptr != right)
			delete right;
	}
};


void addLayerToTree(list<Node*>prevLayer)
{
	static ulong val = 2ul;
	list<Node*> newLayer;

	if (val > 1000ul)
		return;

	for (auto n : prevLayer)
	{
		n->left = new Node(val++);
		newLayer.push_back(n->left);

		n->right = new Node(val++);
		newLayer.push_back(n->right);
	}
	prevLayer = newLayer;
	addLayerToTree(prevLayer);
}

typedef void(*funcAddLayer)(list<Node*>prevLayer);

Node* createTree(funcAddLayer addLayer)
{
	Node* head = new Node(1ul);
	list<Node*>prevList = {head};
	addLayer(prevList);

	return head;
}

void printLayer(list<Node*>layer)
{
	if (layer.size() == 0)
		return;

	list<Node*> nextLayer;
	for (auto n : layer)
	{
		if (nullptr != n->left) 
		{			
			cout << n->left->data << " ";
			nextLayer.push_back(n->left);
		}

		if (nullptr != n->right)
		{
			cout << n->right->data << " ";
			nextLayer.push_back(n->right);
		}
	}

	cout << endl;
	printLayer(nextLayer);
}

void printTree(Node* head)
{
	cout << head->data << endl;
	list<Node*> layer = { head };
	printLayer(layer);

}

void printNode(Node* head)
{
	if (nullptr == head)
		return;
	cout <<"\n\n *************** Node "<< head->data << endl << endl;
}

/*Find*/
typedef Node*(*funcFind)(Node*, ulong);

Node* findNodeByKey(Node* head, ulong key)
{
	if (nullptr == head)
		return nullptr;
	if (head->data == key)
		return head;
	if(nullptr != head->left)
		findNodeByKey(head->left, key);

	if (nullptr != head->right)
		findNodeByKey(head->right, key);

}

static Node* foundFromThread;
static bool found = false;
static int countTh = 0;

void findNodeByKeyThread(Node* head, ulong key, int thNum)
{
	if (nullptr == head)
		return;
	if (head->data == key)
	{
		cout << "\n\n*************** FOUND in " << thNum << endl << endl;
		foundFromThread= head;
		found = true;
	}

	if (nullptr != head->left && !found)
	{
		countTh++;
		thread thL(findNodeByKeyThread, head->left, key, countTh);
		cout <<"  Start thL " <</*thL.get_id()*/countTh<<" \n";
		thL.detach();
	}

	if (nullptr != head->right && !found)
	{
		countTh++;
		thread thR(findNodeByKeyThread, head->right, key, countTh);
		cout << "  Start thR " << /*thR.get_id()*/ countTh  <<" \n";
		thR.detach();
	}


}


void findNodeByKeyPromise(Node* head, ulong key, promise<Node*>& p)// promise<Node*>&
{
	if (nullptr == head)
	{
		 p.set_value(nullptr);
		 return;
	}

	if (head->data == key)
	{
		 p.set_value(head);
		found = true;

		//cout << "***********"<<this_thread::get_id() << endl;
		return;
	}
	//cout << this_thread::get_id() << endl;
	if (nullptr != head->left)
		findNodeByKeyPromise(head->left, key, p);

	if (nullptr != head->right)
		findNodeByKeyPromise(head->right, key, p);

}

/**/
#define tmaxcount 8
mutex mforlist;
static list<function<void(int n)>> lnforserch;
static uint8 tcount = 0;


void find(Node*n, ulong k)
{
	srand(time(NULL));
	this_thread::sleep_for(chrono::milliseconds(int(0.0+rand()/RAND_MAX)*9000));

	//lock_guard<mutex> gard(mforlist);
	thread::id tid = this_thread::get_id();
	cout << "start " << tid <<" "<<n->data<< endl;

	if (nullptr == n)
	{
		tcount--;

		cout << "end " << tid << " " << endl;
		return;
	}
	if (n->data == k)
	{
		foundFromThread = n;
		found = true;
		cout << "********* " << n->data << endl;
		tcount--;

		cout << "end " << tid << " " << endl;
		return;
	}
	{
		lock_guard<mutex> gard(mforlist);
		if (nullptr != n->left)
			lnforserch.push_back(bind(find, n->left, _1));

		if (nullptr != n->right)
			lnforserch.push_back(bind(find,n->right, _1));
	}
	cout << "end " << tid << " " << endl;
	tcount--;
}

void findNodeByKeyFromList(ulong key)
{
	while (!found)
	{
		if (lnforserch.size() > 0)
		{
			if (tcount < tmaxcount)
			{
				function<void(int)> n = *(lnforserch.begin());
				{
					lock_guard<mutex> gard(mforlist);
					lnforserch.pop_front();
				}
				tcount++;
				
				thread(n, key).detach();
			}
			else {
				//this_thread::sleep_for(chrono::seconds(2));
			}
		}
	}
}

//
//struct tp {
//	uint8 tpcount = 0;
//	vector<uint8> tpfree;
//	vector<uint8> tpclose;
//	vector<function<void()>> tptask;
//	
//	tp(uint8 count) : tpcount(count), tpfree(count), tptask(count)
//	{
//		std::fill(tpfree.begin(), tpfree.end(), 0);
//		std::fill(tpclose.begin(), tpclose.end(), 0);
//		std::fill(tptask.begin(), tptask.end(), nullptr);
//		
//		for (int i = 0; i < tpcount; ++i)
//		{
//			//thread(worker,tptask[i], tpfree[i], tpclose[i]);
//		}
//	}
//
//	void worker(function<void()>&task, uint8& free , uint8& close){
//		while (true)
//		{
//
//		}
//	}
//
//	void dowork()
//	{
//		while (true)
//		{
//			for (auto f : tptask)
//			{
//				if (nullptr == f)
//				{
//
//				}
//				if()
//
//
//
//
//
//
//			}
//		}
//	}
//};


int mainThrow() {

	srand(time(NULL));
	Node* head = createTree(addLayerToTree);
	printTree(head);

	Node* finded = findNodeByKey(head, 120ul);
	printNode(finded);


	lnforserch.push_back(bind(find, head, _1));
	findNodeByKeyFromList(120ul);
	printNode(foundFromThread);

	

	/*multy task
	findNodeByKeyThread(head, 120ul, countTh);
	while (!found) {}
	if (found)
	{
	printNode(foundFromThread);
	cout <<"\n\n***************countTh = "<< countTh << endl<<endl;
	}*/

	/*promise<Node*> p0;
	future<Node*> f0 = p0.get_future();
	thread t0(findNodeByKeyPromise, head, 120ul, ref(p0));

	t0.detach();
	while (!found) {}
	cout <<"&&&& "<< f0.get()->data << endl;*/


	return 0;
}

