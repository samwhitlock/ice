/*
 *
 *  Created by Sam Whitlock on 5/4/10.
 *
 */

inline unsigned int num_bits(uint32_t xor_str)
{
    unsigned int total_bits_set; // store the total here
    static const int S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
    static const int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
    
    total_bits_set = xor_str - ((xor_str >> 1) & B[0]);
    total_bits_set = ((total_bits_set >> S[1]) & B[1]) + (total_bits_set & B[1]);
    total_bits_set = ((total_bits_set >> S[2]) + total_bits_set) & B[2];
    total_bits_set = ((total_bits_set >> S[3]) + total_bits_set) & B[3];
    total_bits_set = ((total_bits_set >> S[4]) + total_bits_set) & B[4];
    
    return total_bits_set;
}