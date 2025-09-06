#include <climits>
#include <cstdio>

struct vertex {
	int x, y;

	bool operator==(const vertex &other) const {
		return other.x == x && other.y == y;
	}
};

struct neighbor {
	vertex at;
	int time;
};

struct lift {
	vertex from;
	vertex to;

	int leaves_every;
	int travel_time;
};

// --------------------------------------------------------------------

// Based on the description and pseudo-code from Wikipedia:
// https://en.wikipedia.org/wiki/Pairing_heap
struct pairing_heap {
	pairing_heap() = default;

	pairing_heap(const pairing_heap &other) = delete;
	pairing_heap(pairing_heap &&other) = delete;
	pairing_heap &operator=(const pairing_heap &other) = delete;
	pairing_heap &operator=(pairing_heap &&other) = delete;

	~pairing_heap() {
		delete_tree_(root_);
	}

private:
	struct node {
		node(int priority, vertex vtx)
		: priority{priority}, vtx{vtx} { }

		int priority;
		vertex vtx;

		node *child = nullptr;
		node *sibling = nullptr;
	};

public:
	void insert(int priority, vertex vtx) {
		root_ = meld_(root_, new node{priority, vtx});
	}

	vertex extract_min() {
		vertex vtx = root_->vtx;

		auto old_root = root_;
		root_ = merge_pairs_(root_->child);
		delete old_root;

		return vtx;
	}

	bool empty() const {
		return !root_;
	}

private:
	static node *meld_(node *a, node *b) {
		if (!a) return b;
		else if (!b) return a;

		if (a->priority < b->priority) {
			b->sibling = a->child;
			a->child = b;

			return a;
		} else {
			a->sibling = b->child;
			b->child = a;

			return b;
		}
	}

	static node *merge_pairs_(node *list) {
		if (!list) return nullptr;
		if (!list->sibling) return list;

		return meld_(meld_(list, list->sibling), merge_pairs_(list->sibling->sibling));
	}

	static void delete_tree_(node *at) {
		if (!at) return;

		auto child = at->child;
		delete at;

		while (child) {
			auto old = child;
			child = child->sibling;

			delete_tree_(old);
		}
	}

	node *root_ = nullptr;
};

template <typename T>
struct array2d {
	array2d(int width, int height)
	: width{width}, height{height}
	, data{new T[width * height]{}} { }

	array2d(const array2d &other) = delete;
	array2d(array2d &&other) = delete;
	array2d &operator=(const array2d &other) = delete;
	array2d &operator=(array2d &&other) = delete;

	~array2d() {
		delete[] data;
	}

	T &operator[](vertex vtx) const {
		return data[vtx.x + width * vtx.y];
	}

	const int width, height;
	T *const data;
};

template <typename T>
struct fixed_vector {
	explicit fixed_vector(int capacity)
	: capacity_{capacity}, data_{new T[capacity]} { }

	fixed_vector(const fixed_vector &other) = delete;
	fixed_vector(fixed_vector &&other) = delete;
	fixed_vector &operator=(const fixed_vector &other) = delete;
	fixed_vector &operator=(fixed_vector &&other) = delete;

	~fixed_vector() {
		delete[] data_;
	}

	int size() const {
		return size_;
	}

	void push(const T &t) {
		data_[size_++] = t;
	}

	void clear() {
		size_ = 0;
	}

	T *begin() const {
		return data_;
	}

	T *end() const {
		return data_ + size_;
	}

private:
	const int capacity_;
	int size_ = 0;
	T *const data_;
};

// --------------------------------------------------------------------

struct map {
	map(int width, int height, int n_lifts)
	: width{width}, height{height}, n_lifts{n_lifts}
	, heights{width, height}, lifts{width, height}
	, neighbors_{4 + n_lifts} { }

	map(const map &other) = delete;
	map(map &&other) = delete;

	map &operator=(const map &other) = delete;
	map &operator=(map &&other) = delete;

	~map() {
		for (int i = 0; i < width * height; i++)
			delete lifts.data[i];
	}

	void read_lifts() {
		for (int i = 0; i < n_lifts; i++) {
			lift l;
			scanf("%d%d%d%d%d%d",
					&l.from.x, &l.from.y,
					&l.to.x, &l.to.y,
					&l.travel_time,
					&l.leaves_every);

			auto &vec = lifts[l.from];
			if (!vec) vec = new fixed_vector<lift>{n_lifts};

			vec->push(l);
		}
	}

	void read_heights() {
		for (int i = 0; i < width * height; i++)
			scanf("%d", heights.data + i);
	}

	auto &compute_neighbors(int time, vertex from)  {
		int B = heights[from];
		neighbors_.clear();

		auto insert_grid_edge = [&] (vertex to) {
			if (to.x < 0 || to.x >= width) return;
			if (to.y < 0 || to.y >= height) return;
			int A = heights[to];

			neighbors_.push({to, A > B ? A - B + 1 : 1});
		};

		auto insert_lift_edge = [&] (lift l) {
			int last_departure = time % l.leaves_every;
			int next_departure = last_departure ? l.leaves_every - last_departure : 0;

			neighbors_.push({l.to, next_departure + l.travel_time});
		};

		insert_grid_edge({from.x - 1, from.y});
		insert_grid_edge({from.x + 1, from.y});
		insert_grid_edge({from.x, from.y - 1});
		insert_grid_edge({from.x, from.y + 1});

		if (lifts[from]) {
			for (auto lift : *lifts[from]) insert_lift_edge(lift);
		}

		return neighbors_;
	}

	const int width, height;
	const int n_lifts;
	array2d<int> heights;
	array2d<fixed_vector<lift> *> lifts;

private:
	fixed_vector<neighbor> neighbors_;
};

// --------------------------------------------------------------------

// Based on the description and pseudo-code from Wikipedia:
// https://en.wikipedia.org/wiki/Dijkstra's_algorithm#Using_a_priority_queue
// Implements the variant where only source is added to the queue, and
// new elements are added in place of Q.decrease_priority.
int dijkstra(map &m, vertex source, vertex target) {
	array2d<int> dist{m.width, m.height};
	pairing_heap Q;

	Q.insert(0, source);

	for (int y = 0; y < m.height; y++) {
		for (int x = 0; x < m.width; x++) {
			vertex v{x, y};
			dist[v] = v == source ? 0 : INT_MAX;
		}
	}

	while (!Q.empty()) {
		auto u = Q.extract_min();
		if (u == target) break;

		for (auto [v, edge] : m.compute_neighbors(dist[u], u)) {
			auto alt = dist[u] + edge;

			if (alt < dist[v]) {
				dist[v] = alt;
				Q.insert(alt, v);
			}
		}
	}

	return dist[target];
}

// --------------------------------------------------------------------

int main() {
	int width, height;
	vertex start, end;
	int n_lifts;

	scanf("%d%d%d%d%d%d%d",
			&width, &height,
			&start.x, &start.y,
			&end.x, &end.y,
			&n_lifts);

	map m{width, height, n_lifts};
	m.read_lifts();
	m.read_heights();

	printf("%d\n", dijkstra(m, start, end));
}
