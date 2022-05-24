#pragma once
#include <data_structures/queue.h>
#include <assert.h>
//Definition of the node


template <class Type> struct linkedQueuesNodeType
{
	Type info;
	linkedQueuesNodeType<Type>* link;
};


template <class Type>
class linkedQueueType : public queueADT<Type>
{
public:
	//const linkedQueueType<Type>& operator=(const linkedQueueType<Type>&);
	//Overload the assignment operator.
	bool isEmptyQueue() const;
	//Function to determine whether the queue is empty.
	//Postcondition: Returns true if the queue is empty,
	// otherwise returns false.
	bool isFullQueue() const;
	//Function to determine whether the queue is full.
	//Postcondition: Returns true if the queue is full,
	// otherwise returns false.
	void initializeQueue();
	//Function to initialize the queue to an empty state.
	//Postcondition: queueFront = nullptr; queueRear = nullptr
	Type front() const;
	//Function to return the first element of the queue.
	//Precondition: The queue exists and is not empty.
	//Postcondition: If the queue is empty, the program
	// terminates; otherwise, the first
	// element of the queue is returned.
	Type back() const;
	//Function to return the last element of the queue.
	//Precondition: The queue exists and is not empty.
	//Postcondition: If the queue is empty, the program
	// terminates; otherwise, the last
	// element of the queue is returned.
	void addQueue(const Type& queueElement);
	void addQueue(const Type& queueElement, PVOID addr);
	void perTypeAppend(const Type& queueElement, PVOID addr);
	bool search(const Type& searchItem) const;
	//Function to add queueElement to the queue.
	//Precondition: The queue exists and is not full.
	//Postcondition: The queue is changed and queueElement
	// is added to the queue.
	void deleteQueue();
	//Function to remove the first element of the queue.
	//Precondition: The queue exists and is not empty.
	//Postcondition: The queue is changed and the first
	// element is removed from the queue.
	linkedQueueType();
	//Default constructor
	//linkedQueueType(const linkedQueueType<Type>& otherQueue);
	//Copy constructor
	//~linkedQueueType();
	//Destructor

	void* returnFirst() {
		return (void*)(this->queueFront);
	}
private:
	linkedQueuesNodeType<Type>* queueFront; //pointer to the front of
	//the queue
	linkedQueuesNodeType<Type>* queueRear; //pointer to the rear of
	//the queue
};

template <class Type>
bool linkedQueueType<Type>::search(const Type& searchItem) const
{
	linkedQueuesNodeType<Type>* current; //pointer to traverse the list
	bool found = false;
	current = this->queueFront; //set current to point to the first
	//node in the list
	while (current != nullptr && !found) //search the list
		if (current->info == searchItem) //searchItem is found
			found = true;
		else
			current = current->link; //make current point to
			//the next node
	return found;
}//end search

template <class Type>
bool linkedQueueType<Type>::isEmptyQueue() const
{
	return (queueFront == nullptr);
} //end isEmptyQueue
template <class Type>
bool linkedQueueType<Type>::isFullQueue() const
{
	return false;
} //end isFullQueue

template <class Type>
void linkedQueueType<Type>::initializeQueue()
{
	linkedQueuesNodeType<Type>* temp;
	while (queueFront != nullptr) //while there are elements
	//left in the queue
	{
		temp = queueFront; //set temp to point to the
		//current node
		queueFront = queueFront->link; //advance first to
		//the next node
		delete temp; //deallocate memory occupied by temp
	}
	queueRear = nullptr; //set rear to nullptr
} //end initializeQueue

template <class Type>
void linkedQueueType<Type>::addQueue(const Type& newElement)
{
	linkedQueuesNodeType<Type>* newNode;
	newNode = new linkedQueuesNodeType<Type>; //create the node
	newNode->info = newElement; //store the info
	newNode->link = nullptr; //initialize the link
	//field to nullptr
	if (queueFront == nullptr) //if initially the queue is empty
	{
		queueFront = newNode;
		queueRear = newNode;
	}
	else //add newNode at the end
	{
		queueRear->link = newNode;
		queueRear = queueRear->link;
	}
}//end addQueue

template <class Type>
void linkedQueueType<Type>::addQueue(const Type& newElement, PVOID addr)
{
	linkedQueuesNodeType<Type>* newNode;
	newNode = new(addr) linkedQueuesNodeType<Type>; //create the node
	newNode->info = newElement; //store the info
	newNode->link = nullptr; //initialize the link
	//field to nullptr
	if (queueFront == nullptr) //if initially the queue is empty
	{
		queueFront = newNode;
		queueRear = newNode;
	}
	else //add newNode at the end
	{
		queueRear->link = newNode;
		queueRear = queueRear->link;
	}
}
template<class Type>
inline void linkedQueueType<Type>::perTypeAppend(const Type& queueElement, PVOID addr)
{
	this->addQueue(queueElement, addr);
}
//end addQueue

template <class Type>
Type linkedQueueType<Type>::front() const
{
	assert(queueFront != nullptr);
	return queueFront->info;
} //end front
template <class Type>
Type linkedQueueType<Type>::back() const
{
	assert(queueRear != nullptr);
	return queueRear->info;
} //end back

template <class Type>
void linkedQueueType<Type>::deleteQueue()
{
	linkedQueuesNodeType<Type>* temp;
	if (!isEmptyQueue())
	{
		temp = queueFront; //make temp point to the
		//first node
		queueFront = queueFront->link; //advance queueFront
		delete temp; //delete the first node
		if (queueFront == nullptr) //if after deletion the
		//queue is empty
			queueRear = nullptr; //set queueRear to nullptr
	}
	else
		cout << "Cannot remove from an empty queue" << endl;
}//end deleteQueue

template <class Type>
linkedQueueType<Type>::linkedQueueType()
{
	queueFront = nullptr; //set front to nullptr
	queueRear = nullptr; //set rear to nullptr
} //end default constructor

