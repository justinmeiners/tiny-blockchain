#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <assert.h>

#include "sha-256.h"

#define LINE_MAX 4096

void fprint_hash(FILE* f, uint8_t* hash)
{
    fprintf(f, "0x");
    for (int i = 0; i < 32; ++i)
        fprintf(f, "%02x", hash[i]);
}
typedef struct
{
    /* Length of the data in the block */
    uint32_t contents_length;
    /* Hash of the block contents. */
    /* 32 is the number of bytes in a sha256 hash */
    uint8_t contents_hash[32];
    uint8_t previous_hash[32];

    /* when this block started being mined */
    uint32_t timestamp; 

    /* This is adjusted to make the hash of this header fall in the valid range. */
    uint32_t nonce;
} block_header_t;
void mine_block(block_header_t* header, const uint8_t* target)
{
    while (1)
    {
        /* MINING: start of the mining round */
        header->timestamp = (uint64_t)time(NULL); 

        /* adjust the nonce until the block header is < the target hash */
        uint8_t block_hash[32];
        
        for (uint32_t i = 0; i < UINT32_MAX; ++i)
        {
            header->nonce = i;
            calc_sha_256(block_hash, header, sizeof(block_header_t));
        
            if (memcmp(block_hash, target, sizeof(block_hash)) < 0)
                /* we found a good hash */
                return;
        }
 
        /* The uint32 expired without finding a valid hash.
           Restart the time, and hope that this time + nonce combo works. */
    }

    /* this should never happen */
    assert(0);
}
block_header_t build_block(const block_header_t* previous, const char* contents, uint64_t length)
{
    block_header_t header;
    header.contents_length = length;

    if (previous)
    {
        /* calculate previous block header hash */
        calc_sha_256(header.previous_hash, previous, sizeof(block_header_t));
    }
    else
    {
        /* genesis has no previous. just use zeroed hash */
        memset(header.previous_hash, 0, sizeof(header.previous_hash));
    }
    
    /* add data hash */
    calc_sha_256(header.contents_hash, contents, length);
    return header;
}

/* this controls the difficulty.
   I chose this target because it works well on my computer.
   Feel free to try out others. */

uint8_t target[32];

int main(int argc, const char* argv[])
{
    memset(target, 0, sizeof(target));

    /* too hard?: try target[2] = 0xFF
       too easy?: try target[2] = 0x01 */
    target[2] = 0x0F;

    
    int block_no = 0;
    block_header_t previous;
    
    while (!feof(stdin))
    {
        /* read data to put in the block */
        char line_buffer[LINE_MAX];
        fgets(line_buffer, LINE_MAX, stdin);  
        uint64_t size = strnlen(line_buffer, LINE_MAX) + 1;
    
    
        block_header_t* previous_ptr = block_no == 0 ? NULL : &previous;
        fprintf(stderr, "creating block %i: ", block_no);
        fprintf(stderr, "%s\n", line_buffer);
         
        block_header_t header = build_block(previous_ptr, line_buffer, size);
        mine_block(&header, target);
        previous = header;
        ++block_no;
    
        /* hash the solved header. (only for display purposes) */
        printf("previous: ");
        fprint_hash(stdout, previous.previous_hash);
        printf("\n");
    
        printf("contents: ");
        fprint_hash(stdout, previous.contents_hash);
        printf("\n");
    
        printf("timestamp: %d\n", previous.timestamp);
        printf("nonce: %d\n", previous.nonce);
    
        uint8_t test_hash[32];
        calc_sha_256(test_hash, &previous, sizeof(block_header_t));
        printf("hash: ");
        fprint_hash(stdout, test_hash);
        printf("\n\n\n");
    
    }
    return 1;
}
