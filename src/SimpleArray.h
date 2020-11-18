/**
 *  SimpleVario!!
 *  Copyright Pedro Enrique
 */

#ifndef SIMPLE_ARRAY_H
#define SIMPLE_ARRAY_H

template <typename _T_>
class array_struct {
public:
	array_struct() {}
	array_struct<_T_>* prev = NULL;
	_T_ data;
	array_struct<_T_>* next = NULL;
};

template <typename _T_>
class SimpleArray
{
private:
	array_struct<_T_>* nodeAt(int index) {
		if (index > m_size-1) {
			return NULL;
		}
		while (m_array->prev != NULL) {
			m_array = m_array->prev;
		}
		for (int i = 0; i < index; i++) {
			m_array = m_array->next;
		}
		return m_array;
	}
	
	array_struct<_T_>* m_array { NULL };
	int m_size {0};
	
public:
	SimpleArray() {
		m_size = 0;
	}
	~SimpleArray() {
		empty();
	}
	void empty() {
		while (m_size > 0) {
			pop();
		}
	}
	
	void shift() {
		if (m_size == 0) return;
		if (m_size == 1) {
			delete m_array;
			m_array = NULL;
			m_size = 0;
			return;
		}
		
		while (m_array->prev != NULL) {
			m_array = m_array->prev;
		}
		m_array = m_array->next;
		delete m_array->prev;
		m_array->prev = NULL;
		m_size --;
	}
	
	void pop() {
		if (m_size == 0) return;
		if (m_size == 1) {
			delete m_array;
			m_array = NULL;
			m_size = 0;
			return;
		}
		while (m_array->next != NULL) {
			m_array = m_array->next;
		}
		m_array = m_array->prev;
		delete m_array->next;
		m_array->next = NULL;
		m_size --;
	}
	
	_T_ at(int index) {
		return nodeAt(index)->data;
	}
	
	_T_ operator[] (int i) {
		return at(i);
	}
	
	_T_ last() {
		return nodeAt(m_size-1)->data;
	}
	
	_T_ first() {
		return nodeAt(0)->data;
	}
	   
	void push(_T_ str) {
		if (m_array == NULL) {
			m_array = new array_struct<_T_>();
			m_array->prev = NULL;
			m_array->data = str;
			m_array->next = NULL;
		} else {
			while (m_array->next != NULL) {
				m_array = m_array->next;
			}
			auto new_array = new array_struct<_T_>();
			new_array->prev = NULL;
			new_array->data = str;
			new_array->next = NULL;
			m_array->next = new_array;
			new_array->prev = m_array;
			m_array = new_array;
		}
		m_size++;
	}
	void removeAt(int index) {
		if (index == m_size - 1) {
			pop();
			return;
		}
		
		if (index == 0) {
			shift();
			return;
		}
		array_struct<_T_>* node = nodeAt(index);
		node->prev->next = node->next;
		node->next->prev = node->prev;
		delete node;
		node = NULL;
		if (m_array->next != NULL) {
			m_array = m_array->next;
		} else {
			m_array = m_array->prev;
		}
		m_size--;
	}
	
	void insert(_T_ str, int index) {
		auto node = nodeAt(index);
		auto new_array = new array_struct<_T_>();
		new_array->prev = node->prev;
		new_array->next = node;
		node->next = new_array;
		m_size++;
	}
	
	int size() {
		return m_size;
	}
	
};

#endif
