#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <assert.h>

#include "sha-256.h"

typedef struct
{
    // hash of block contents
    // in BitCoin this would
    // be the Merkle root.
    // this is generalized to
    // any data (not transactions)
    uint8_t data_hash[32];
    // prevents previous data from changing
    uint8_t previous_hash[32];
    // when this block started being mined
    uint32_t timestamp;
    // length of the data
    uint32_t length;
    // nonce
    // this is adjusted by the miner
    // until a suitable hash is created
    uint32_t nonce;
} BlockHeader;

void fprint_hash(FILE* f, uint8_t* hash)
{
    fprintf(f, "0x");
    for (int i = 0; i < 32; ++i)
    {
        fprintf(f, "%02x", hash[i]);
    }
}

BlockHeader build_block(BlockHeader* previous, const char* data, uint64_t length)
{
    BlockHeader header;
    header.length = length;

    if (previous)
    {
        // calculate previous block header hash
        calc_sha_256(header.previous_hash, previous, sizeof(BlockHeader));
    }
    else
    {
        memset(header.previous_hash, 0, sizeof(header.previous_hash))
    }
    

    // add data hash
    calc_sha_256(header.data_hash, data, length);

    // set target
    // this is the difficulty
    uint8_t target[32];
    memset(target, 0, sizeof(target));
    target[2] = 0x1F;

    // MINING
    while (1)
    {
        // start of this mining round
        header.timestamp = (uint64_t)time(NULL);

        // adjust the nonce
        // until the block header is < the target hash

        uint8_t block_hash[32];

        uint32_t i = 0;
        while (i < UINT32_MAX)
        {
            header.nonce = i;
            calc_sha_256(block_hash, &header, sizeof(BlockHeader));

            if (memcmp(block_hash, target, sizeof(block_hash)) == -1)
            {
                // we found a good hash!
                return header;
            }
            ++i;
        }
        // we expired the uint32 without finding
        // a valid hash
        // restart the time, and hope that this time + nonce
        // combo will work
    }
    
    // this should never happen
    assert(0);
    return header;
}

#define LINE_MAX 2048

int main(int argc, const char* argv[])
{
    printf("creating genesis block...\n");
    char genesis_data[] = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    BlockHeader genesis = build_block(NULL, genesis_data, sizeof(genesis_data));

    BlockHeader previous = genesis;
    while (!feof(stdin))
    {
        // ask for more data to put
        // in the next block
        printf("enter block data: \n");
        char line_buffer[LINE_MAX];
        fgets(line_buffer, LINE_MAX, stdin);  
 
        printf("creating block...\n");
        uint64_t size = strnlen(line_buffer, LINE_MAX) + 1;
        BlockHeader header = build_block(&previous, line_buffer, size);

        // hash the resulting
        // header, for display purposes
        uint8_t test_hash[32];
        calc_sha_256(test_hash, &header, sizeof(BlockHeader));

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
