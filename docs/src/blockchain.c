/* blockchain.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <assert.h>

#include "sha-256.h"

#define LINE_MAX 4096

/* print hashes */
void fprint_hash(FILE* f, uint8_t* hash)
{
    fprintf(f, "0x");
    for (int i = 0; i < 32; ++i)
        fprintf(f, "%02x", hash[i]);
}

/* block header */
typedef struct
{
    /* Length of the data in the block */
    uint32_t contents_length;
    /* Hash of the block contents.
       Prevents contents from changing
       (in Bitcoin this would actually be the "merkle root" */
    uint8_t contents_hash[32];
    /* prevents previous data from changing */
    uint8_t previous_hash[32];

    /* proof-of-work entries */
    /* when this block started being mined */
    uint32_t timestamp; 
    
    /* nonce.
       this is adjusted by the miner,
       until a suitable hash is found */
    uint32_t nonce;

} block_header_t;


/* mining */
void mine_block(block_header_t* header)
{
    /* target */
    /* this controls the difficulty.
       I chose this target because it works well on my computer.
       feel free to try out others. */
    
    uint8_t target[32];
    memset(target, 0, sizeof(target));
    target[2] = 0x0F;
    
    /* too hard?: try target[2] = 0xFF
       too easy?: try target[2] = 0x01 */


    while (1)
    {
        /* MINING: start of the mining round */
        header->timestamp = (uint64_t)time(NULL); 

        /* nonce search */
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

/* build block */
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

    /* mining. disucssed later */
    mine_block(&header);
    return header;
}


int main(int argc, const char* argv[])
{
    FILE* output_file = fopen("chain.bin", "wb");

    /* input loop */
    /* genesis block */
    printf("creating genesis block...\n");
    char genesis_data[] = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    block_header_t genesis = build_block(NULL, genesis_data, sizeof(genesis_data));

    
    int block_no = 0;
    block_header_t previous = genesis;
    while (!feof(stdin))
    {
        /* hash the solved header. (only for display purposes) */
        uint8_t test_hash[32];
        calc_sha_256(test_hash, &previous, sizeof(block_header_t));
        printf("done. nonce: %i hash: ", previous.nonce);
        fprint_hash(stdout, test_hash);
        printf("\n");
    
        /* dump header to a file */
        fwrite(&previous, sizeof(block_header_t), 1, output_file);
     
        /* read data to put in the block */
        char line_buffer[LINE_MAX];
        fgets(line_buffer, LINE_MAX, stdin);  
    
        printf("creating block %i: ", block_no);
        printf("%s\n", line_buffer);
        uint64_t size = strnlen(line_buffer, LINE_MAX) + 1;
        block_header_t header = build_block(&previous, line_buffer, size);
      
        previous = header;
        ++block_no;
    }


    fclose(output_file);
    return 1;
}


