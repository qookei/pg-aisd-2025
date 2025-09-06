#include <cstdio>

struct trie_node {
	explicit trie_node(int value)
	: value{value}, children{nullptr} { }

	trie_node(const trie_node &other) = delete;
	trie_node(trie_node &&other) = delete;
	trie_node &operator=(const trie_node &other) = delete;
	trie_node &operator=(trie_node &&other) = delete;

	~trie_node() {
		delete[] children;
	}

	void delete_children(int width, int next_width) {
		if (!children) return;
		for (int i = 0; i < width; i++) {
			if (children[i]) {
				children[i]->delete_children(next_width, next_width);
				delete children[i];
			}
		}
	}

	void force_children(int width) {
		if (!children)
			children = new trie_node *[width]{};
	}

	void print_inorder(int width, int next_width) {
		printf("%d ", value);
		if (!children) return;

		for (int i = 0; i < width; i++) {
			if (children[i]) children[i]->print_inorder(next_width, next_width);
		}
	}

	bool has_children(int width) const {
		if (!children) return false;

		for (int i = 0; i < width; i++) {
			if (children[i]) return true;
		}

		return false;
	}

	int value;
	trie_node **children;
};

struct trie {
	trie(int n, int k)
	: n{n}, k{k}, root{nullptr} { }

	trie(const trie &other) = delete;
	trie(trie &&other) = delete;
	trie &operator=(const trie &other) = delete;
	trie &operator=(trie &&other) = delete;

	~trie() {
		if (root) {
			root->delete_children(n, k);
			delete root;
		}
	}

	bool insert(int value) {
		auto at = find_slot_(value);
		if (*at) return false;

		*at = new trie_node{value};
		return true;
	}

	bool find(int value) {
		return (*find_slot_(value)) != nullptr;
	}

	bool remove(int value) {
		trie_node **at = find_slot_(value);
		if (!*at) return false;
		int at_width = at == &root ? n : k;

		if (!(*at)->has_children(at_width)) {
			delete *at;
			*at = nullptr;
			return true;
		}

		trie_node **leftmost = at;
		int leftmost_width = at_width;
		while ((*leftmost)->has_children(leftmost_width)) {
			for (int i = 0; i < leftmost_width; i++) {
				if ((*leftmost)->children[i]) {
					leftmost = &(*leftmost)->children[i];
					break;
				}
			}
			leftmost_width = k;
		}

		(*at)->value = (*leftmost)->value;
		delete *leftmost;
		*leftmost = nullptr;

		return true;
	}

	void print_inorder() {
		if (root) root->print_inorder(n, k);
		printf("\n");
	}

private:
	// Returns the pointer to the slot which is supposed to hold
	// the pointer to node of the given value.
	trie_node **find_slot_(int value) {
		trie_node **cur = &root;

		int key = value;
		while (*cur && (*cur)->value != value) {
			int width = cur == &root ? n : k;
			// Force the children array to be allocated
			// since we'll be taking a pointer into it.
			(*cur)->force_children(width);
			cur = &((*cur)->children[key % width]);
			key /= width;
		}

		return cur;
	}

	int n, k;
	trie_node *root;
};

int main() {
	int n_cmds;
	scanf("%d", &n_cmds);

	int min, max;
	scanf("%d%d", &min, &max);

	int n, k;
	scanf("%d%d", &n, &k);

	trie t{n, k};

	while (n_cmds--) {
		char cmd[2];
		int v = -1;
		scanf("%1s", cmd);
		if (cmd[0] == 'I' || cmd[0] == 'L' || cmd[0] == 'D')
			scanf("%d", &v);

		switch (cmd[0]) {
			case 'I':
				if (!t.insert(v))
					printf("%d exist\n", v);
				break;
			case 'D':
				if (!t.remove(v))
					printf("%d not exist\n", v);
				break;
			case 'L': printf("%d %s\n", v,
					t.find(v)
					? "exist"
					: "not exist");
				break;
			case 'P': t.print_inorder(); break;
		}
	}
}
