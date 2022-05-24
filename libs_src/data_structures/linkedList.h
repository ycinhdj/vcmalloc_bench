#pragma once
#include <assert.h>
using namespace std;

template <class Type>
struct linkedListNodeType
{
	Type info;
	linkedListNodeType<Type>* link;
};


template <class Type>
class linkedListIterator
{
public:
	linkedListIterator();
	//Default constructor
	//Postcondition: current = nullptr;
	linkedListIterator(linkedListNodeType<Type>* ptr);
	//Constructor with a parameter.
	//Postcondition: current = ptr;
	Type operator*();
	//Function to overload the dereferencing operator *.
	//Postcondition: Returns the info contained in the node.
	linkedListIterator<Type> operator++();
	//Overload the pre-increment operator.
	//Postcondition: The iterator is advanced to the next
	// node.
	bool operator==(const linkedListIterator<Type>& right) const;
	//Overload the equality operator.
	//Postcondition: Returns true if this iterator is equal to
	// the iterator specified by right,
	// otherwise it returns false.
	bool operator!=(const linkedListIterator<Type>& right) const;
	//Overload the not equal to operator.
	//Postcondition: Returns true if this iterator is not equal
	// to the iterator specified by right,
	// otherwise it returns false.
private:
	linkedListNodeType<Type>* current; //pointer to point to the current
	//node in the linked list
};

template <class Type>
linkedListIterator<Type>::linkedListIterator()
{
	current = nullptr;
}
template <class Type>
linkedListIterator<Type>::linkedListIterator(linkedListNodeType<Type>* ptr)
{
	current = ptr;
}
template <class Type>
Type linkedListIterator<Type>::operator*()
{
	return current->info;
}
template <class Type>
linkedListIterator<Type> linkedListIterator<Type>::operator++()
{
	current = current->link;
	return *this;
}
template <class Type>
bool linkedListIterator<Type>::operator==(const linkedListIterator<Type>& right) const
{
	return (current == right.current);
}

template <class Type>
bool linkedListIterator<Type>::operator!=(const linkedListIterator<Type>& right) const
{
	return (current != right.current);
}

template <class Type>
class linkedListType
{
public:
	//Overload the assignment operator.
	const linkedListType<Type>& operator=(const linkedListType<Type>&);
	//Initialize the list to an empty state.
	//Postcondition: first = nullptr, last = nullptr,
	// _count = 0;
	void initializeList();
	//Function to determine whether the list is empty.
	//Postcondition: Returns true if the list is empty,
	// otherwise it returns false.
	bool isEmptyList() const;
	//Function to output the data contained in each node.
	//Postcondition: none
	void print() const;
	//Function to return the number of nodes in the list.
	//Postcondition: The value of _count is returned.
	int length() const;
	//Function to delete all the nodes from the list.
	//Postcondition: first = nullptr, last = nullptr,
	// _count = 0;
	void destroyList();
	//Function to return the first element of the list.
	//Precondition: The list must exist and must not be
	// empty.
	//Postcondition: If the list is empty, the program
	// terminates; otherwise, the first
	// element of the list is returned.
	Type front() const;
	//Function to return the last element of the list.
	//Precondition: The list must exist and must not be
	// empty.
	//Postcondition: If the list is empty, the program
	// terminates; otherwise, the last
	// element of the list is returned.
	Type back() const;
	//Function to determine whether searchItem is in the list.
	//Postcondition: Returns true if searchItem is in the
	// list, otherwise the value false is
	// returned.
	virtual bool search(const Type& searchItem) const = 0;
	//Function to insert newItem at the beginning of the list.
	//Postcondition: first points to the new list, newItem is
	// inserted at the beginning of the list,
	// last points to the last node in the list,
	// and _count is incremented by 1.
	virtual void insertFirst(const Type& newItem) = 0;
	//Function to insert newItem at the end of the list.
	//Postcondition: first points to the new list, newItem
	// is inserted at the end of the list,
	// last points to the last node in the
	// list, and _count is incremented by 1.
	virtual void insertLast(const Type& newItem) = 0;
	//Function to delete deleteItem from the list.
	//Postcondition: If found, the node containing
	// deleteItem is deleted from the list.
	// first points to the first node, last
	// points to the last node of the updated
	// list, and _count is decremented by 1.
	virtual void deleteNode(const Type& deleteItem) = 0;
	//Function to return an iterator at the begining of
	//the linked list.
	//Postcondition: Returns an iterator such that current
	// is set to first.
	linkedListIterator<Type> begin();
	//Function to return an iterator one element past the
	//last element of the linked list.
	//Postcondition: Returns an iterator such that current
	// is set to nullptr.
	linkedListIterator<Type> end();
	//Default constructor
	//Initializes the list to an empty state.
	//Postcondition: first = nullptr, last = nullptr,
	// _count = 0;
	linkedListType();
	//copy constructor
	linkedListType(const linkedListType<Type>& otherList);
	//Destructor
	//Deletes all the nodes from the list.
	//Postcondition: The list object is destroyed.
	//~linkedListType();
	void* returnFirst() {
		return (void*)(this->first);
	}
protected:
	linkedListNodeType<Type>* first; //pointer to the first node of the list
	linkedListNodeType<Type>* last; //pointer to the last node of the list
	int _count; //variable to store the number of elements in the list
private:
	void copyList(const linkedListType<Type>& otherList);
	//Function to make a copy of otherList.
	//Postcondition: A copy of otherList is created and
	// assigned to this list.
};

template <class Type>
bool linkedListType<Type>::isEmptyList() const
{
	return (first == nullptr);
}

template <class Type>
linkedListType<Type>::linkedListType() //default constructor
{
	first = nullptr;
	last = nullptr;
	_count = 0;
}

template <class Type>
void linkedListType<Type>::destroyList()
{
	linkedListNodeType<Type>* temp; //pointer to deallocate the memory
	//occupied by the node
	while (first != nullptr) //while there are nodes in
	{ //the list
		temp = first; //set temp to the current node
		first = first->link; //advance first to the next node
		delete temp; //deallocate the memory occupied by temp
	}
	last = nullptr; //initialize last to nullptr; first has
	//already been set to nullptr by the while loop
	_count = 0;
}

template <class Type>
void linkedListType<Type>::initializeList()
{
	destroyList(); //if the list has any nodes, delete them
}

template <class Type>
void linkedListType<Type>::print() const
{
	linkedListNodeType<Type>* current; //pointer to traverse the list
	current = first; //set current so that it points to
	//the first node
	for (int i = 0; i < this->_count; i++) {
		cout << current->info << " ";
		current = current->link;
	}
}//end print

template <class Type>
int linkedListType<Type>::length() const
{
	return _count;
} //end length

template <class Type>
Type linkedListType<Type>::front() const
{
	assert(first != nullptr);
	return first->info; //return the info of the first node
}//end front

template <class Type>
Type linkedListType<Type>::back() const
{
	assert(last != nullptr);
	return last->info; //return the info of the last node
}//end back

template <class Type>
linkedListIterator<Type> linkedListType<Type>::begin()
{
	linkedListIterator<Type> temp(first);
	return temp;
}
template <class Type>
linkedListIterator<Type> linkedListType<Type>::end()
{
	linkedListIterator<Type> temp(nullptr);
	return temp;
}

template <class Type>
void linkedListType<Type>::copyList(const linkedListType<Type>& otherList)
{
	linkedListNodeType<Type>* newNode; //pointer to create a node
	linkedListNodeType<Type>* current; //pointer to traverse the list
	if (first != nullptr) //if the list is nonempty, make it empty
		destroyList();
	if (otherList.first == nullptr) //otherList is empty
	{
		first = nullptr;
		last = nullptr;
		_count = 0;
	}
	else
	{
		current = otherList.first; //current points to the
		//list to be copied
		_count = otherList._count;
		//copy the first node
		first = new linkedListNodeType<Type>; //create the node
		first->info = current->info; //copy the info
		first->link = nullptr; //set the link field of
		//the node to nullptr
		last = first; //make last point to the
	   //first node
		current = current->link; //make current point to
		 //the next node
		 //copy the remaining list
		while (current != nullptr)
		{
			newNode = new linkedListNodeType<Type>; //create a node
			newNode->info = current->info; //copy the info
			newNode->link = nullptr; //set the link of
			//newNode to nullptr
			last->link = newNode; //attach newNode after last
			last = newNode; //make last point to
			//the actual last node
			current = current->link; //make current point
			//to the next node
		}//end while
	}//end else
}//end copyList

/*
template <class Type>
linkedListType<Type>::~linkedListType() //destructor
{
	destroyList();
}//end destructor
*/

template <class Type>
linkedListType<Type>::linkedListType(const linkedListType<Type>& otherList)
{
	first = nullptr;
	copyList(otherList);
}//end copy constructor


//overload the assignment operator
template <class Type>
const linkedListType<Type>& linkedListType<Type>::operator=(const linkedListType<Type>& otherList)
{
	if (this != &otherList) //avoid self-copy
	{
		copyList(otherList);
	}//end else
	return *this;
}

