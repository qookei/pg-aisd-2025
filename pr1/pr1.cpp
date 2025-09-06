#include <cstddef> // size_t
#include <cassert> // assert
#include <cstring> // strlen
#include <cstdio>  // fgets, getchar, printf
#include <utility> // std::move, std::forward, std::swap

template <typename T>
struct linked_list {
	struct node {
		friend struct linked_list;

		template <typename U>
		explicit node(U &&u)
		: value{std::forward<U>(u)} { }

		node(const node &) = delete;
		node(node &&) = delete;
		node &operator=(const node &) = delete;
		node &operator=(node &&) = delete;

		T value;

		node *next() const { return next_; }
		node *prev() const { return prev_; }

	private:
		node *next_ = nullptr, *prev_ = nullptr;
	};

	friend void swap(linked_list &a, linked_list &b) {
		std::swap(a.head_, b.head_);
		std::swap(a.tail_, b.tail_);
	}

	linked_list() = default;
	linked_list(const linked_list &) = delete;
	linked_list(linked_list &&other) : linked_list{} {
		swap(*this, other);
	}

	linked_list &operator=(linked_list other) {
		swap(*this, other);
		return *this;
	}

	~linked_list() {
		auto delete_node = [] (node *cur, auto succ) {
			if (!cur)
				return;
			succ(cur->next_, succ);
			delete cur;
		};

		delete_node(head_, delete_node);
	}

	node *head() const {
		return head_;
	}

	node *tail() const {
		return tail_;
	}

	template <typename U>
	node *insert_after(node *after, U &&u) {
		if (!after) {
			assert(!head_);
			assert(!tail_);

			head_ = tail_ = new node{std::forward<U>(u)};
			return head_;
		}

		auto n = new node{std::forward<U>(u)};

		n->prev_ = after;
		n->next_ = after->next_;
		if (after->next_)
			after->next_->prev_ = n;
		n->prev_->next_ = n;
		if (after == tail_)
			tail_ = n;

		return n;
	}

	template <typename U>
	node *insert_before(node *before, U &&u) {
		if (!before) {
			assert(!head_);
			assert(!tail_);

			head_ = tail_ = new node{std::forward<U>(u)};
			return head_;
		}

		auto n = new node{std::forward<U>(u)};

		n->next_ = before;
		n->prev_ = before->prev_;
		if (before->prev_)
			before->prev_->next_ = n;
		n->next_->prev_ = n;
		if (before == head_)
			head_ = n;

		return n;
	}

	void remove(node *that) {
		if (that == head_)
			head_ = that->next_;
		if (that == tail_)
			tail_ = that->prev_;

		if (that->prev_)
			that->prev_->next_ = that->next_;
		if (that->next_)
			that->next_->prev_ = that->prev_;

		delete that;
	}

	void splice_in(linked_list &other) {
		if (!other.tail_) {
			assert(!other.head_);
			return;
		}
		if (!tail_) {
			assert(!head_);
			swap(*this, other);
			return;
		}

		tail_->next_ = other.head_;
		other.head_->prev_ = tail_;
		tail_ = other.tail_;

		other.head_ = nullptr;
		other.tail_ = nullptr;
	}

	linked_list copy() {
		linked_list out;

		auto do_copy = [&out] (node *cur, auto succ) {
			if (!cur)
				return;
			out.insert_after(out.tail(), cur->value);
			succ(cur->next_, succ);
		};

		do_copy(head_, do_copy);

		return out;
	}

private:
	node *head_ = nullptr, *tail_ = nullptr;
};

template <typename T>
using ll_node = typename linked_list<T>::node;


template <typename T>
struct ll_stack {
	ll_stack() = default;

	template <typename U>
	void push(U &&u) {
		ll_.insert_after(ll_.tail(), std::forward<U>(u));
		depth_++;
	}

	T pop() {
		auto v = std::move(ll_.tail()->value);
		ll_.remove(ll_.tail());
		depth_--;
		return v;
	}

	T &peek(size_t index = 0) & {
		auto do_peek = [] (ll_node<T> *n, size_t left, auto succ) {
			if (!left || !n)
				return n;
			return succ(n->prev(), left - 1, succ);
		};

		return do_peek(ll_.tail(), index, do_peek)->value;
	}

	size_t depth() const { return depth_; }
	auto &underlying_list() { return ll_; }

private:
	linked_list<T> ll_;
	size_t depth_ = 0;
};

struct ll_item {
	ll_item() = default;
	explicit ll_item(linked_list<char> value) : value_{std::move(value)} { }

	template <typename T>
	static ll_item from_number(T value) {
		ll_item out;

		auto do_format = [&out] (T value, auto succ) {
			out.value_.insert_after(out.value_.tail(), char((value % 10) + '0'));
			value /= 10;
			if (!value) return;
			succ(value, succ);
		};

		do_format(value, do_format);
		if (value < 0)
			out.negate();

		return out;
	}

	static ll_item from_char(char c) {
		ll_item out;
		out.prepend(c);
		return out;
	}

	template <typename T>
	T into_number() {
		T out{};

		auto do_parse = [&out] (ll_node<char> *cur, auto succ) {
			if (!cur)
				return;
			out *= 10;
			if (cur->value != '-')
				out += (cur->value - '0');
			succ(cur->prev(), succ);
		};

		do_parse(value_.tail(), do_parse);
		if (is_negative())
			out = -out;

		return out;
	}

	ll_item copy() {
		return ll_item{value_.copy()};
	}

	void print() {
		auto do_print = [] (ll_node<char> *cur, auto succ) {
			if (!cur)
				return;
			printf("%c", cur->value);
			succ(cur->next(), succ);
		};

		do_print(value_.head(), do_print);
	}

	linked_list<char> &underlying_list() {
		return value_;
	}


	bool is_truthy() const {
		return value_.head() && !(value_.head() == value_.tail() && value_.head()->value == '0');
	}

	bool is_negative() const {
		return value_.tail() && value_.tail()->value == '-';
	}


	void negate() {
		if (is_negative())
			value_.remove(value_.tail());
		else
			value_.insert_after(value_.tail(), '-');
	}

	void make_absolute() {
		if (is_negative())
			value_.remove(value_.tail());
	}

	void prepend(char c) {
		value_.insert_before(value_.head(), c);
	}

	void append(char c) {
		value_.insert_after(value_.tail(), c);
	}

	char detach_first() {
		char c = value_.head()->value;
		value_.remove(value_.head());
		return c;
	}

	void trim_zeros() {
		auto do_trim = [this] (ll_node<char> *cur, ll_node<char> *head,
				auto succ) {
			if (cur == head)
				return;
			if (cur->value != '0')
				return;

			auto prev = cur->prev();
			value_.remove(cur);
			succ(prev, head, succ);
		};

		do_trim(value_.tail(), value_.head(), do_trim);
	}


	bool numerically_zero() const {
		auto do_check = [] (ll_node<char> *cur, auto succ) {
			if (!cur)
				return true;
			if (cur->value != '0')
				return false;
			return succ(cur->next(), succ);
		};

		return do_check(value_.head(), do_check);
	}

	bool numerically_equal(ll_item &&other) {
		bool a_neg = is_negative(), b_neg = other.is_negative();

		make_absolute();
		other.make_absolute();

		if (numerically_zero())
			a_neg = false;
		if (other.numerically_zero())
			b_neg = false;

		if (a_neg != b_neg)
			return false;

		auto do_compare = [] (ll_node<char> *left, ll_node<char> *right,
				auto succ) {
			if (left && !right && left->value != '0')
				return false;
			if (!left && right && right->value != '0')
				return false;
			if (!left && !right)
				return true;
			if (left && right && left->value != right->value)
				return false;
			return succ(
				left ? left->next() : left,
				right ? right->next() : right,
				succ);
		};

		return do_compare(value_.head(), other.value_.head(), do_compare);
	}

	bool numerically_lesser(ll_item &&other) {
		bool a_neg = is_negative(), b_neg = other.is_negative();

		make_absolute();
		other.make_absolute();

		if (numerically_zero())
			a_neg = false;
		if (other.numerically_zero())
			b_neg = false;

		trim_zeros();
		other.trim_zeros();

		// a < 0, b >= 0
		if (a_neg && !b_neg)
			return true;
		// a >= 0, b < 0
		if (!a_neg && b_neg)
			return false;

		auto do_compare = [] (ll_node<char> *left, ll_node<char> *right,
				auto succ) {
			if (left && !right)
				return false;
			if (!left && right)
				return true;
			if (!left && !right)
				return false;
			if (left->value < right->value)
				return true;
			if (left->value > right->value)
				return false;
			return succ(left->prev(), right->prev(), succ);
		};

		auto l_ptr = value_.tail();
		auto r_ptr = other.value_.tail();

		assert(a_neg == b_neg);
		if (a_neg)
			// (-a) < (-b) <=> b < a
			std::swap(l_ptr, r_ptr);

		return do_compare(l_ptr, r_ptr, do_compare);
	}

	ll_item add(ll_item &&other) {
		ll_item out;
		bool a_neg = is_negative(), b_neg = other.is_negative();

		make_absolute();
		other.make_absolute();

		if (numerically_zero())
			a_neg = false;
		if (other.numerically_zero())
			b_neg = false;

		auto plus  = +[] (int a, int b) { return a + b; };
		auto minus = +[] (int a, int b) { return a - b; };

		auto l_ptr = value_.head();
		auto r_ptr = other.value_.head();
		auto op = plus;
		bool negate_result = false;

		if (a_neg != b_neg) {
			// (-a) + b => b - a dla a < b; -(b - a) dla b < a
			// a + (-b) => b - a dla b < a; -(b - a) dla a < b
			std::swap(l_ptr, r_ptr);
			if (copy().numerically_lesser(other.copy()))
				negate_result = b_neg;
			else
				negate_result = !a_neg;
			op = minus;
		} else if (a_neg && b_neg) {
			// (-a) + (-b) <=> -(a + b)
			negate_result = true;
		}

		auto do_digit = [&out] (ll_node<char> *left, ll_node<char> *right,
				int carry, auto op, auto succ) {
			if (!left && !right)
				return carry;

			auto l_digit = left ? left->value - '0' : 0;
			auto r_digit = right ? right->value - '0' : 0;

			auto answer = op(l_digit, r_digit) + carry;
			auto a_digit = answer % 10;
			auto a_carry = answer / 10;

			if (a_digit < 0) {
				a_carry = -1;
				a_digit = 10 + a_digit;
			}

			out.append(a_digit + '0');

			return succ(
				left ? left->next() : left,
				right ? right->next() : right,
				a_carry, op, succ);
		};

		auto final_carry = do_digit(l_ptr, r_ptr, 0, op, do_digit);

		if (final_carry < 0) {
			negate_result = !negate_result;
			final_carry = -final_carry;
		}

		if (final_carry != 0) {
			out.append((final_carry % 10) + '0');
		}

		out.trim_zeros();

		if (negate_result && out.is_truthy())
			out.negate();

		return out;
	}

private:
	linked_list<char> value_;
};


struct cpu {
	explicit cpu(const char *program)
	: program_{program} { }

	void dump_stack() {
		auto do_dump = [] (ll_node<ll_item> *cur, size_t depth, auto succ) {
			if (!depth) return;

			printf("%zu: ", depth - 1);
			cur->value.print();
			printf("\n");

			succ(cur->next(), depth - 1, succ);
		};

		do_dump(stack_.underlying_list().head(), stack_.depth(), do_dump);
	}

	const char *single_step(const char *pc);

	void run(const char *pc) {
		auto next_pc = single_step(pc);
		if (!next_pc)
			return;
		run(next_pc);
	}

	void run() {
		run(program_);
	}

private:
	ll_stack<ll_item> stack_;
	const char *program_;
};

const char *cpu::single_step(const char *pc) {
	auto insn = *pc;
	if (!insn)
		return nullptr;

	auto next_pc = pc + 1;

	switch(insn) {
		case '\'':
			stack_.push(ll_item{});
			break;
		case ',':
			stack_.pop();
			break;
		case ':':
			stack_.push(stack_.peek().copy());
			break;
		case ';': {
			auto a = stack_.pop();
			auto b = stack_.pop();
			stack_.push(std::move(a));
			stack_.push(std::move(b));
			break;
		}
		case '@': {
			auto idx = stack_.pop().into_number<size_t>();
			stack_.push(stack_.peek(idx).copy());
			break;
		}
		case '.':
			stack_.peek().prepend(getchar());
			break;
		case '>': {
			auto chr = stack_.pop().detach_first();
			printf("%c", chr);
			break;
		}
		case '!': {
			auto v = stack_.pop();
			stack_.push(ll_item::from_number<int>(!v.is_truthy()));
			break;
		}
		case '<': {
			auto a = stack_.pop();
			auto b = stack_.pop();
			stack_.push(ll_item::from_number<int>(b.numerically_lesser(std::move(a))));
			break;
		}
		case '=': {
			auto a = stack_.pop();
			auto b = stack_.pop();
			stack_.push(ll_item::from_number<int>(a.numerically_equal(std::move(b))));
			break;
		}
		case '~':
			stack_.push(ll_item::from_number(pc - program_));
			break;
		case '?': {
			auto t = stack_.pop();
			auto w = stack_.pop();
			if (w.is_truthy())
				next_pc = program_ + t.into_number<size_t>();
			break;
		}
		case '-':
			stack_.peek().negate();
			break;
		case '^':
			stack_.peek().make_absolute();
			break;
		case '$': {
			auto chr = stack_.peek().detach_first();
			stack_.push(ll_item::from_char(chr));
			break;
		}
		case '#': {
			auto lst = stack_.pop();
			stack_.peek().underlying_list().splice_in(lst.underlying_list());
			break;
		}
		case '+': {
			auto a = stack_.pop();
			auto b = stack_.pop();
			stack_.push(a.add(std::move(b)));
			break;
		}
		case '&':
			dump_stack();
			break;
		case ']': {
			auto ord = stack_.pop().into_number<int>();
			stack_.push(ll_item::from_char(ord));
			break;
		}
		case '[': {
			auto chr = stack_.pop().detach_first();
			stack_.push(ll_item::from_number<int>(static_cast<unsigned char>(chr)));
			break;
		}
		default:
			stack_.peek().prepend(insn);
	}

	return next_pc;
}


int main() {
	char program[20000 + 1 + 1];
	fgets(program, 20000 + 1, stdin);

	auto program_size = strlen(program);
	if (program_size && program[program_size - 1] == '\n')
		program[program_size - 1] = '\0';

	cpu cpu_{program};
	cpu_.run();
}
