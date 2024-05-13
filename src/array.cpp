template <typename T>
struct DynArr {
	T* buf;
	int len;
	int cap;

	DynArr<T>(): buf(NULL), len(0), cap(0) {}

	T& operator [] (int i) {
		return buf[i];
	}

	static DynArr<T> new_with_cap(int cap) {
		DynArr<T> arr;
		arr.len = 0;
		arr.cap = cap;
		arr.buf = (T*)malloc(sizeof(T) * cap);
		if (arr.buf == NULL) {
			printf("Failed to allocate a dynamic array for %i elements", cap);
			exit(1);
		}
		return arr;
	}

	void push(T value) {
		if (len == cap) {
			cap *= 2;
			buf = (T*)realloc(buf, sizeof(T) * cap);
			if (buf == NULL) {
				printf("Failed to reallocate a dynamic array for %i elements", cap);
				exit(1);
			}
		}
		buf[len] = value;
		++len;
	}

	T pop() {
		if (len == 0) {
			printf("Failed to pop element from array");
			exit(1);
		}
		len -= 1;
		return buf[len];
	}

	T pop_front() {
		if (len == 0) {
			printf("Failed to pop front element from array");
			exit(1);
		}
		T val = buf[0];
		buf += 1;
		len -= 1;
		return val;
	}

	T remove_at(int index) {
		if (index < 0 || index >= len) {
			printf("Failed to remove element at index %i from array", index);
			exit(1);
		}
		T val = buf[index];
		memmove(&buf[index], &buf[index + 1], sizeof(T) * (len - index - 1));
		len -= 1;
		return val;
	}

	void join(DynArr<T> other) {
		while (len + other.len >= cap) {
			cap *= 2;
			buf = (T*)realloc(buf, sizeof(T) * cap);
			if (buf == NULL) {
				printf("Failed to reallocate a dynamic array for %i elements", cap);
				exit(1);
			}
		}
		memcpy(&buf[len], other.buf, sizeof(T) * other.len);
		len += other.len;
	}

	DynArr<T> clone() {
		DynArr<T> new_array = DynArr<T>::new_with_cap(cap);
		memcpy(new_array.buf, buf, sizeof(T) * len);
		new_array.len = len;
		return new_array;
	}

	DynArr<DynArr<T>> split(T delimiter) {
		DynArr<DynArr<T>> new_array = DynArr<DynArr<T>>::new_with_cap(4);
		int last_index = 0;
		for (int i = 0; i <= len; ++i) {
			if (i == len || buf[i] == delimiter) {
				int size = i - last_index;
				DynArr<T> sub_array = DynArr<T>::new_with_cap(size);
				memcpy(sub_array.buf, &buf[last_index], sizeof(T) * size);
				sub_array.len = size;
				new_array.push(sub_array);
				last_index = ++i;
			}
		}
		return new_array;
	}
};