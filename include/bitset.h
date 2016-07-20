#ifndef BITSET_H_
# define BITSET_H_

# include <stdlib.h>

/*
 *
 * Bitset(size_t nbits):
 *   Returns the newly created bitset, filled with zeros, with the size given as parameter.
 *
 * resize_bitset(bitset set, size_t size):
 *   Resize the bitset from its previous size to size.
 *   If size is bigger than its previous size, the new bits will be initialized to 0.
 *   Else, the bitset is truncated and overlying bits are lost.
 *
 * del_bitset(bitset set):
 *   Free the memory used by the bitset set.
 *
 * copy_bitset(bitset set):
 *   Return a new bitset strictly identic to the one given as parameter.
 *
 * set_subset(bitset set, size_t from, size_t to, bool state):
 *   Set all the bits from the from position to the to position, included, at the state given (can be true or false).
 *   Overlying bits are simply ignored.
 *
 * shift_bitset(bitset set, int width):
 *   Proceed to a binary shift of the bitset, to width bits to the left (in a big endianness, so proceed to a shift
 *   to the most significant bit).
 *   You can call it with a negative width to shift bits to the right.
 *   Newly inserted bits are always set to false.
 *
 * compare_bitsets(bitset s1, bitset s2):
 *   Compares s1 and s2 and return true if bitsets are equals (when bits at the same position are equals).
 *   If their sizes arent equals, it doesnt matter as the overlying bits from the biggest bitset are all false; else
 *   returns false.
 *
 * print_bitset(bitset set):
 *   Print the bitset to the standard output, with on space each 4 bits, and two spaces each 8 bits.
 *   Useful when you need to have a look at your bitset, in debug.
 *   You need to define the _DEBUG macro before.
 *
 * bitwise_not(bitset set):
 *   Returns a newly copied bitset, from set, after had proceed to a not logical operation to all bits.
 *
 * SET_BIT(bitset set, size_t index, bool state):
 *   Set the bit at position index to the state given as parameter.
 *
 * GET_BIT(bitset set, size_t index):
 *   Returns true if the bit at the pos index is set, false otherwise.
 *   You can't modify the bit's state from this.
 *
 * BITSET_SIZE(bitset set):
 *   Returns the number of bits of the bitset.
 *
 * TODO:
 *   bitwise_and(bitset s1, bitset s2):
 *     Returns a new bitset created from the and binary operation between s1 and s2.
 *
 *   bitwise_or(bitset s1, bitset s2);
 *     Returns a new bitset created from the or binary operation between s1 and s2.
 *
 *   bitwise_xor(bitset s1, bitset s2);
 *     Returns a new bitset created from the xor binary operation between s1 and s2.
 *
 *   bitwise_nand(bitset s1, bitset s2);
 *     Returns a new bitset created from the nand binary operation between s1 and s2.
 *
 *   bitwise_nor(bitset s1, bitset s2);
 *     Returns a new bitset created from the nor binary operation between s1 and s2.
 *
 *   bitwise_xnor(bitset s1, bitset s2);
 *     Returns a new bitset created from the xnor binary operation between s1 and s2.
 *
 *   convert_from(void *from, size_t nbits);
 *     Returns a newly created bitset from the nbits bits from the from pointer.
 *
 *   convert_to(bitset set, void *to, size_t nbits);
 *     Put nbits firsts bits of the bitset into the to pointer.
 *     Please be sure that the to pointer can embed enough bits.
 *
 */

typedef __uint128_t _word_t;
typedef unsigned int u_int;
typedef _word_t * bitset;
typedef enum bool bool;

enum bool {
	false = 0,
	true = 1
};

# define _WORD_NBYTES               (sizeof(_word_t))
# define _WORD_SIZE                 (_WORD_NBYTES * 8)
# define _B_INDEX(index)            ((index) / _WORD_SIZE)
# define _B_OFFSET(index)           ((index) % _WORD_SIZE)

# define _MIN(A, B)                 (((A) < (B)) ? A : B)
# define _MAX(A, B)                 (((A) > (B)) ? A : B)
# define _SWAP(A, B)                ({ typeof(A) _t; _t = A; A = B; B = _t; })

# define _SET_INDEX(set, index)     ((set)[(index)] = ~((_word_t) 0))
# define _RESET_INDEX(set, index)   ((set)[(index)] = 0)

# define SET_BIT(set, index, state) ({ \
		if (state) (set)[_B_INDEX(index)] |= (_word_t) 1 << _B_OFFSET(index); \
		else (set)[_B_INDEX(index)] &= ~((_word_t) 1 << _B_OFFSET(index)); \
	})

# define GET_BIT(set, index)        ((bool) !!((set)[_B_INDEX(index)] & ((_word_t) 1 << _B_OFFSET(index))))

# define BITSET_SIZE(set)           ((size_t) *(set - 1))

# define Bitset(nbits) ({ \
		bitset _set = ((bitset) calloc(_B_INDEX(nbits) + !!_B_OFFSET(nbits) + 1, sizeof(_word_t))) + 1; \
		*(_set - 1) = nbits; \
		_set; \
	})

# define resize_bitset(set, size) ({ \
		set = (bitset) realloc(set - 1, size + sizeof(_word_t)) + 1; \
		set_subset(set, BITSET_SIZE(set), size, 0); \
		*(set - 1) = size; \
	})

# define del_bitset(set) ({ free(set - 1); })

static bitset copy_bitset(bitset original) {
	size_t i = BITSET_SIZE(original);
	bitset copy = Bitset(i);

	i = _B_INDEX(i) + !!_B_OFFSET(i);
	while (i--)
		copy[i] = original[i];
	return (copy);
}

static void set_subset(bitset set, size_t from, size_t to, bool state) {
	state = !!state;
	if (to > BITSET_SIZE(set))
		to = BITSET_SIZE(set);
	else
		++to;
	if (from >= to)
		return;
	if (_B_INDEX(from) == _B_INDEX(to)) {
		if (state)
			set[_B_INDEX(from)] |=   ~(~ (_word_t) 0 << (_B_OFFSET(to) - _B_OFFSET(from))) << _B_OFFSET(from);
		else
			set[_B_INDEX(from)] &= ~(~(~ (_word_t) 0 << (_B_OFFSET(to) - _B_OFFSET(from))) << _B_OFFSET(from));
	} else {
		u_int i = _B_INDEX(from);
		if (state) {
			set[_B_INDEX(from)] |=    (~ (_word_t) 0 << _B_OFFSET(from));
			set[_B_INDEX(to)]   |=   ~(~ (_word_t) 0 << _B_OFFSET(to));
			while (++i < _B_INDEX(to))
				_SET_INDEX(set, i);
		} else {
			set[_B_INDEX(from)] &=   ~(~ (_word_t) 0 << _B_OFFSET(from));
			set[_B_INDEX(to)]   &=    (~ (_word_t) 0 << _B_OFFSET(to));
			while (++i < _B_INDEX(to))
				_RESET_INDEX(set, i);
		}
	}
}

static void shift_bitset(bitset set, int width) {
	size_t len = _B_INDEX(BITSET_SIZE(set)),
	       i;
	_word_t shift[2] = {0, 0};

	if (width < 0) {
		width = -width;
		i = len;
		shift[1] = (set[i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(set))))) >> width;
		while (i--) {
			shift[0] = set[i];
			shift[1] |= shift[0] >> (_WORD_SIZE - width);
			set[i + 1] = shift[1];
			shift[1] = shift[0] >> width;
		}
		set[i + 1] = shift[1];
	} else if (width) {
		i = 0;
		do {
			shift[0] = set[i];
			shift[1] |= shift[0] << (_WORD_SIZE - width);
			if (i)
				set[i - 1] = shift[1];
			shift[1] = shift[0] << width;
		} while (++i < len);
		set[i - 1] = shift[1];
	}
}

static bool compare_bitsets(bitset s1, bitset s2) {
	size_t min_len = _MIN(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))),
	       max_len = _MAX(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))),
	       i = -1;

	if (min_len != _B_INDEX(BITSET_SIZE(s1)))
		_SWAP(s1, s2);
	while (++i < min_len)
		if (s1[i] != s2[i])
			return (false);
	if ((s1[i] & ~(~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(s1)))))
			!= (s2[i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(s1)))))
			|| (min_len == max_len
				&& (s2[i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(s2)) + _B_OFFSET(BITSET_SIZE(s1))) << _B_OFFSET(BITSET_SIZE(s1)))))
			|| (min_len != max_len
				&& (s2[i] & (~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(s1)))))))
		return (false);
	while (++i < max_len)
		if (s2[i])
			return (false);
	return (true);
}

static bitset bitwise_not(bitset set) {
	bitset set_not = copy_bitset(set);
	size_t len = _B_INDEX(BITSET_SIZE(set)),
	       i = -1;

	while (++i < len)
		set_not[i] = ~set_not[i];
	set_not[i] = ~(set_not[i] & ~(~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(set_not)))));
	return (set_not);
}

# ifdef _DEBUG

int print_bitset(bitset set) {
	size_t len = BITSET_SIZE(set),
	       i = 0;

	while (i < len) {
		printf("%s%s%d", !(i % 4) && i ? " " : "", !(i % 8) && i ? " " : "", GET_BIT(set, i));
		++i;
	}
	printf("\n");
	return (BITSET_SIZE(set));
}

# endif /* !_DEBUG */

#endif /* !BITSET_H_ */
