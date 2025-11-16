#include "lcptools/lps.h"
#include "lcptools/core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define UBLOCK_BIT_SIZE 32

// Helper function: Convert binary string to 32-bit unsigned integer
static uint32_t binary_string_to_int(const char *binary_str) {
    uint32_t result = 0;
    int len = strlen(binary_str);
    
    // If string is longer than 32 bits, only process the last 32 bits
    int start_index = 0;
    if (len > 32) {
        start_index = len - 32;
    }
    
    for (int i = start_index; i < len; i++) {
        if (binary_str[i] == '1') {
            result = (result << 1) | 1;
        } else if (binary_str[i] == '0') {
            result = result << 1;
        }
    }
    
    return result;
}

// Helper function: Extract binary string representation from a core
static int core_to_binary_string(const struct core *cr, char *buffer, size_t buffer_size) {
    // We need at least 33 bytes (32 bits + null terminator) for a 32-bit string
    // But we'll accept any buffer that can hold the actual bit_size
    if (buffer_size < (size_t)(cr->bit_size + 1)) {
        // If buffer is too small but can hold at least 33 bytes, extract only last 32 bits (LSB)
        if (buffer_size >= 33 && cr->bit_size > 32) {
            uint64_t block_number = (cr->bit_size - 1) / UBLOCK_BIT_SIZE + 1;
            int buffer_index = 0;
            
            // Extract only the last 32 bits (least significant bits: indices 0-31)
            // These bits are stored in the last block of bit_rep array
            // We extract from index 31 down to 0 to maintain MSB-first order in the string
            for (int index = 31; index >= 0; index--) {
                int bit = (cr->bit_rep[block_number - index / UBLOCK_BIT_SIZE - 1] >> (index % UBLOCK_BIT_SIZE)) & 1;
                buffer[buffer_index++] = bit ? '1' : '0';
            }
            
            buffer[buffer_index] = '\0';
            return buffer_index;
        }
        return -1; // Buffer too small
    }
    
    uint64_t block_number = (cr->bit_size - 1) / UBLOCK_BIT_SIZE + 1;
    int buffer_index = 0;
    
    for (int index = cr->bit_size - 1; 0 <= index; index--) {
        int bit = (cr->bit_rep[block_number - index / UBLOCK_BIT_SIZE - 1] >> (index % UBLOCK_BIT_SIZE)) & 1;
        buffer[buffer_index++] = bit ? '1' : '0';
    }
    
    buffer[buffer_index] = '\0';
    return buffer_index;
}

// Super method: Convert all cores in LPS structure to integers and write to file
int convert_lps_cores_to_integers_file(const struct lps *lps_ptr, const char *filename) {
    // Open output file for writing integers
    FILE *output_file = fopen(filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error: Could not open output file '%s' for writing.\n", filename);
        return 1;
    }

    // Allocate buffer for binary string (32 bits + null terminator)
    const size_t max_binary_length = 33;  // 32 bits + '\0'
    char *binary_buffer = (char *)malloc(max_binary_length);
    if (binary_buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(output_file);
        return 1;
    }

    // Convert each core's binary representation to integer and write to file
    for (int i = 0; i < lps_ptr->size; i++) {
        // Extract binary string from core
        // If bit_size > 32, core_to_binary_string will extract only last 32 bits
        // Otherwise, it extracts the full string
        int binary_len = core_to_binary_string(&(lps_ptr->cores[i]), binary_buffer, max_binary_length);
        
        if (binary_len < 0) {
            fprintf(stderr, "Warning: Failed to extract binary string for core %d, skipping.\n", i);
            continue;
        }
        
        // Convert binary string to 32-bit unsigned integer
        // If string is longer than 32 bits, only last 32 bits are used
        uint32_t integer_value = binary_string_to_int(binary_buffer);
        
        // Write integer to file
        fprintf(output_file, "%u ", integer_value);
    }

    // Clean up
    free(binary_buffer);
    fclose(output_file);

    return 0;
}

int main(int argc, char *argv[]) {

    const char *input_file = NULL;
    int deepen_level = 1;  // Default deepen level

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -i flag requires an input file\n");
                fprintf(stderr, "Usage: %s -i <input_file> [-d <deepen_level>]\n", argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                deepen_level = atoi(argv[++i]);
                if (deepen_level < 0) {
                    fprintf(stderr, "Error: deepen level must be non-negative\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -d flag requires a deepen level\n");
                fprintf(stderr, "Usage: %s -i <input_file> [-d <deepen_level>]\n", argv[0]);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            fprintf(stderr, "Usage: %s -i <input_file> [-d <deepen_level>]\n", argv[0]);
            return 1;
        }
    }

    // Check if input file was provided
    if (input_file == NULL) {
        fprintf(stderr, "Error: Input file is required\n");
        fprintf(stderr, "Usage: %s -i <input_file> [-d <deepen_level>]\n", argv[0]);
        return 1;
    }

    // Initialize alphabet coefficients
    LCP_INIT();

    // Read input file
    FILE *file = fopen(input_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", input_file);
        return 1;
    }

    // Read entire file into buffer
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = (char *)malloc(file_size + 1);
    if (!file_content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(file_content, 1, file_size, file);
    file_content[bytes_read] = '\0';
    fclose(file);

    // Remove all whitespace (spaces, tabs, newlines, etc.)
    char *cleaned_str = (char *)malloc(bytes_read + 1);
    if (!cleaned_str) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(file_content);
        return 1;
    }

    size_t cleaned_len = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (!isspace((unsigned char)file_content[i])) {
            cleaned_str[cleaned_len++] = file_content[i];
        }
    }
    cleaned_str[cleaned_len] = '\0';

    // Free the original file content
    free(file_content);

    // Create LCP string object
    struct lps lcp_str;
    init_lps(&lcp_str, cleaned_str, cleaned_len);

    // Deepen the LCP analysis
    lps_deepen(&lcp_str, deepen_level);

    // Generate output filename: <input_file>-level<deepen_level>.txt
    char output_filename[512];
    snprintf(output_filename, sizeof(output_filename), "%s-level%d.txt", input_file, deepen_level);

    // Convert cores to integers and save to file
    convert_lps_cores_to_integers_file(&lcp_str, output_filename);

    // Clean up
    free_lps(&lcp_str);
    free(cleaned_str);

    return 0;
}
