/* blockchain.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <assert.h>

#include "sha-256.h"

#define LINE_MAX 2048

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
    // Length of the data in the block
    uint32_t contents_length;
    // Hash of the block contents.
    // Prevents contents from changing
    // (in Bitcoin this would actually be the "merkle root")
    uint8_t contents_hash[32];
    // prevents previous data from changing
    uint8_t previous_hash[32];

    // pow for later
    /* proof-of-work entries */
    
    // when this block started being mined
    uint32_t timestamp; 
    
    // nonce
    // this is adjusted by the miner
    // until a suitable hash is created
    uint32_t nonce;

} block_header_t;


/* build block */

block_header_t build_block(const block_header_t* previous, const char* contents, uint64_t length)
{
    block_header_t header;
    header.contents_length = length;

    if (previous)
    {
        // calculate previous block header hash
        calc_sha_256(header.previous_hash, previous, sizeof(block_header_t));
    }
    else
    {
        // genesis has no previous. just use zeroed hash
        memset(header.previous_hash, 0, sizeof(header.previous_hash))
    }
    
    // add data hash
    calc_sha_256(header.contents_hash, contents, length);

    // mining. disucssed later
    mine_block(&header);
    return header;
}

/* mining */

void mine_block(block_header_t* header)
{
    // set target
    // this is the difficulty
    uint8_t target[32];
    memset(target, 0, sizeof(target));
    target[2] = 0x1F;

    while (1)
    {
        // MINING
        // start of this mining round
        header->timestamp = (uint64_t)time(NULL);

        // adjust the nonce
        // until the block header is < the target hash
        uint8_t block_hash[32];

        for (uint32_t i = 0; i < UNIT32_MAX; ++i)
        {
            header->nonce = i;
            calc_sha_256(block_hash, header, sizeof(block_header_t));

            if (memcmp(block_hash, target, sizeof(block_hash)) == -1)
                // we found a good hash!
                return;
        }
        // we expired the uint32 without finding a valid hash
        // restart the time, and hope that this time + nonce
        // combo will work
    }

    // this should never happen
    assert(0);
}


int main(int argc, const char* argv[])
{
    /* genesis block */
    printf("creating genesis block...\n");
    char genesis_data[] = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    block_header_t genesis = build_block(NULL, genesis_data, sizeof(genesis_data));


    /* input loop */
    block_header_t previous = genesis;
    while (!feof(stdin))
    {
        // ask for more data to put
        // in the next block
        printf("enter block data: \n");
        char line_buffer[LINE_MAX];
        fgets(line_buffer, LINE_MAX, stdin);  
    
        printf("creating block...\n");
        uint64_t size = strnlen(line_buffer, LINE_MAX) + 1;
        block_header_t header = build_block(&previous, line_buffer, size);
    
        // hash the resulting
        // header, for display purposes
        uint8_t test_hash[32];
        calc_sha_256(test_hash, &header, sizeof(block_header_t));
    
        printf("nonce: %i hash: ", header.nonce);
        fprint_hash(stdout, test_hash);
        printf("\n");
        
        previous = header;
    }


    // blocks aren't actually stored
    // anywhere. We just keep on building the chain
    // from the previous header.
    return 1;
}


