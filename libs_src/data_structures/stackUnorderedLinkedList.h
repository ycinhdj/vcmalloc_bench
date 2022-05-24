#pragma	once
#include <iostream>
#include "unorderedLinkedList.h"
using namespace std;
template <class Type>
class stackUnorderedLinkedList : public unorderedLinkedList<Type>
{
public:
	void initializeStack();
	bool isEmptyStack() const;
	bool isFullStack() const;
	void push(const Type& newItem);
	void push(const Type& newItem, PVOID addr);
	void perTypeAppend(const Type& newItem, PVOID addr);
	Type top() const;
	void pop();
};
template <class Type>

void stackUnorderedLinkedList<Type>::initializeStack()
{
	unorderedLinkedList<Type>::initializeList();
}
template <class Type>
bool stackUnorderedLinkedList<Type>::isEmptyStack() const
{
	return unorderedLinkedList<Type>::isEmptyList();
}
template <class Type>
bool stackUnorderedLinkedList<Type>::isFullStack() const
{
	return false;
}
template <class Type>
void stackUnorderedLinkedList<Type>::push(const Type& newElement)
{
	unorderedLinkedList<Type>::insertFirst(newElement);
}

template <class Type>
void stackUnorderedLinkedList<Type>::push(const Type& newElement, PVOID addr)
{
	unorderedLinkedList<Type>::insertFirst(newElement, addr);
}

template<class Type>
inline void stackUnorderedLinkedList<Type>::perTypeAppend(const Type& newItem, PVOID addr)
{
	this->push(newItem, addr);
}

template <class Type>
Type stackUnorderedLinkedList<Type>::top() const
{
	return unorderedLinkedList<Type>::front();
}
template <class Type>
void stackUnorderedLinkedList<Type>::pop()
{
	linkedListNodeType<Type>* temp;
	temp = this->first;
	this->first = this->first->link;
	delete temp;
}

