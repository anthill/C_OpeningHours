#ifndef BITSET_H_
# define BITSET_H_

# include <stdlib.h>

# ifdef _DEBUG
/* Include stdio, needed to define printf calls in printf_bitset(). */
#  include <stdio.h>
# endif /* !_DEBUG */
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
 * bitwise_and(bitset s1, bitset s2):
 *   Returns a new bitset created from the and binary operation between s1 and s2.
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
# define _SWAP(A, B)                ({ __auto_type _t = A; A = B; B = _t; })

# define _SET_INDEX(set, index)     ((set)[(index)] = ~((_word_t) 0))
# define _RESET_INDEX(set, index)   ((set)[(index)] = 0)

# define SET_BIT(set, index, state) ({                                                                                          \
	if (state) (set)[_B_INDEX(index)] |= (_word_t) 1 << _B_OFFSET(index);                                                   \
	else (set)[_B_INDEX(index)] &= ~((_word_t) 1 << _B_OFFSET(index));                                                      \
})

# define GET_BIT(set, index)        ((bool) !!((set)[_B_INDEX(index)] & ((_word_t) 1 << _B_OFFSET(index))))

# define BITSET_SIZE(set)           ((size_t) *(set - 1))

# define Bitset(nbits) ({                                                                                                       \
	bitset _set = ((bitset) calloc(_B_INDEX(nbits) + !!_B_OFFSET(nbits) + 1, sizeof(_word_t))) + 1;                         \
	*(_set - 1) = nbits;                                                                                                    \
	_set;                                                                                                                   \
})

# define resize_bitset(set, size) ({                                                                                            \
	set = (bitset) realloc(set - 1, size + sizeof(_word_t)) + 1;                                                            \
	set_subset(set, BITSET_SIZE(set), size, 0);                                                                             \
	*(set - 1) = size;                                                                                                      \
})

# define del_bitset(set) ({ free(set - 1); })

# define copy_bitset(original) ({                                                                                               \
	size_t _i = BITSET_SIZE(original);                                                                                      \
	bitset _copy = Bitset(_i);                                                                                              \
                                                                                                                                \
	_i = _B_INDEX(_i) + !!_B_OFFSET(_i);                                                                                    \
	while (_i--)                                                                                                            \
		_copy[_i] = original[_i];                                                                                       \
	_copy;                                                                                                                  \
})

# define set_subset(set, from, to, state) ({                                                                                    \
	u_int _from = from, _to = to, _state = state;                                                                           \
	if (_to > (BITSET_SIZE(set) - 1))                                                                                       \
		_to = BITSET_SIZE(set), --_to;                                                                                  \
	++_to;                                                                                                                  \
	if (_from < _to) {                                                                                                      \
		if (_B_INDEX(_from) == _B_INDEX(_to)) {                                                                         \
			if (_state)                                                                                             \
				set[_B_INDEX(_from)] |=                                                                         \
 					  ~(~ (_word_t) 0 << (_B_OFFSET(_to) - _B_OFFSET(_from))) << _B_OFFSET(_from);          \
			else                                                                                                    \
				set[_B_INDEX(_from)] &=                                                                         \
					~(~(~ (_word_t) 0 << (_B_OFFSET(_to) - _B_OFFSET(_from))) << _B_OFFSET(_from));         \
		} else {                                                                                                        \
			u_int i = _B_INDEX(_from);                                                                              \
			if (_state) {                                                                                           \
				set[_B_INDEX(_from)] |=    (~ (_word_t) 0 << _B_OFFSET(_from));                                 \
				if (_B_OFFSET(_to))                                                                             \
					set[_B_INDEX(_to)]   |=   ~(~ (_word_t) 0 << _B_OFFSET(_to));                           \
				while (++i < _B_INDEX(_to))                                                                     \
					_SET_INDEX(set, i);                                                                     \
			} else {                                                                                                \
				set[_B_INDEX(_from)] &=   ~(~ (_word_t) 0 << _B_OFFSET(_from));                                 \
				set[_B_INDEX(_to)]   &=    (~ (_word_t) 0 << _B_OFFSET(_to));                                   \
				while (++i < _B_INDEX(_to))                                                                     \
					_RESET_INDEX(set, i);                                                                   \
			}                                                                                                       \
		}                                                                                                               \
	}                                                                                                                       \
})

# define shift_bitset(set, width) ({                                                                                            \
	size_t _len = _B_INDEX(BITSET_SIZE(set)),                                                                               \
	       _i;                                                                                                              \
	_word_t _shift[2] = {0, 0};                                                                                             \
                                                                                                                                \
	if (width < 0) {                                                                                                        \
		width = -width;                                                                                                 \
		_i = _len;                                                                                                      \
		_shift[1] = (set[_i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(set))))) >> width;                 \
		while (_i--) {                                                                                                  \
			_shift[0] = set[_i];                                                                                    \
			_shift[1] |= _shift[0] >> (_WORD_SIZE - width);                                                         \
			set[_i + 1] = _shift[1];                                                                                \
			_shift[1] = _shift[0] >> width;                                                                         \
		}                                                                                                               \
		set[_i + 1] = _shift[1];                                                                                        \
	} else if (width) {                                                                                                     \
		_i = 0;                                                                                                         \
		do {                                                                                                            \
			_shift[0] = set[_i];                                                                                    \
			_shift[1] |= _shift[0] << (_WORD_SIZE - width);                                                         \
			if (_i)                                                                                                 \
				set[_i - 1] = _shift[1];                                                                        \
			_shift[1] = _shift[0] << width;                                                                         \
		} while (++_i < len);                                                                                           \
		set[_i - 1] = _shift[1];                                                                                        \
	}                                                                                                                       \
})

# define compare_bitsets(s1, s2) ({                                                                                             \
	size_t _min_len = _MIN(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))),                                           \
	       _max_len = _MAX(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))),                                           \
	       _i = -1;                                                                                                         \
	bitset _s1 = s1,                                                                                                        \
	       _s2 = s2;                                                                                                        \
	bool valid = true;                                                                                                      \
                                                                                                                                \
	if (_min_len != _B_INDEX(BITSET_SIZE(_s1)))                                                                             \
		_SWAP(_s1, _s2);                                                                                                \
	while (valid && ++_i < _min_len)                                                                                        \
		if (_s1[_i] != _s2[_i])                                                                                         \
			valid = false;                                                                                          \
	if (valid && (_s1[_i] & ~(~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(_s1)))))                                 \
			!= (_s2[_i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(_s1)))))                            \
			|| (_min_len == _max_len                                                                                \
				&& (_s2[_i] & (~ (_word_t) 0 >> (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(_s2))                       \
							+ _B_OFFSET(BITSET_SIZE(_s1))) << _B_OFFSET(BITSET_SIZE(_s1)))))        \
			|| (_min_len != _max_len                                                                                \
				&& (_s2[_i] & (~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(_s1)))))))                  \
		valid = false;                                                                                                  \
	while (valid && ++_i < _max_len)                                                                                        \
		if (_s2[_i])                                                                                                    \
			valid = false;                                                                                          \
	valid;                                                                                                                  \
})

# define bitwise_not(set) ({                                                                                                    \
	bitset _set_not = copy_bitset(set);                                                                                     \
	size_t _len = _B_INDEX(BITSET_SIZE(set)),                                                                               \
	       _i = -1;                                                                                                         \
                                                                                                                                \
	while (++_i < _len)                                                                                                     \
		_set_not[_i] = ~_set_not[_i];                                                                                   \
	_set_not[i] = ~(_set_not[_i] & ~(~ (_word_t) 0 << (_WORD_SIZE - _B_OFFSET(BITSET_SIZE(_set_not)))));                    \
	_set_not;                                                                                                               \
})

# define bitwise_and(s1, s2) ({                                                                                                 \
	size_t _min_len = _MIN(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))),                                           \
	       _i = -1;                                                                                                         \
	bitset _s1 = s1,                                                                                                        \
	       _s2 = s2,                                                                                                        \
	       _set_and = Bitset(_MAX(_B_INDEX(BITSET_SIZE(s1)), _B_INDEX(BITSET_SIZE(s2))));                                   \
                                                                                                                                \
	if (_min_len != _B_INDEX(BITSET_SIZE(_s1)))                                                                             \
		_SWAP(_s1, _s2);                                                                                                \
	while (++_i <= _min_len)                                                                                                \
		_set_and[_i] = _s1[_i] & _s2[_i];                                                                               \
	_set_and;                                                                                                               \
})

# ifdef _DEBUG

#  define print_bitset(set) ({                                                                                                  \
	size_t _len = BITSET_SIZE(set),                                                                                         \
	       _i = 0;                                                                                                          \
                                                                                                                                \
	while (_i < _len) {                                                                                                     \
		printf("%s%s%d", !(_i % 4) && _i ? " " : "", !(_i % 8) && _i ? " " : "", GET_BIT(_set, _i));                    \
		++_i;                                                                                                           \
	}                                                                                                                       \
	printf("\n");                                                                                                           \
	BITSET_SIZE(_set);                                                                                                      \
})

# endif /* !_DEBUG */

#endif /* !BITSET_H_ */
