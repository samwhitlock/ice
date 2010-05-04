/*
 *
 *  Created by Sam Whitlock on 5/4/10.
 *
 */

inline unsigned int num_bits(uint32_t xor_str)
{
    return __builtin_popcount((unsigned int) xor_str);
}