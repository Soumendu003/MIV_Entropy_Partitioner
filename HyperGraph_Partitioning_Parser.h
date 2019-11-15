#ifndef HYPERGRAPH_PARTITIONING_PARSER_H
#define HYPERGRAPH_PARTITIONING_PARSER_H

#include "Tokenizer.h"
#include "HyperGraph.h"
#include "Header.h"
#include<iostream>
#include<vector>
#include<string>

using namespace std;

struct block_node
{
    uint32_t id;
    float height;
    float width;
};

class HyperGraph_Partitioning_Parser{
    private:
        const char* delimeters = "{},\t ()\n\r";
        char end_char = '\n';
        const char* blocks_file;
        const char* nets_file;
        uint8_t benchmark_type;
        Tokenizer* Tok = 0;
        HyperGraph* HG = 0;
        vector<block_node> *Block_Nodes = 0;
        vector<string> *Node_id_to_Name = 0;

    public:
        explicit HyperGraph_Partitioning_Parser(){}
        ~HyperGraph_Partitioning_Parser();
        HyperGraph_Partitioning_Parser(const char* blocks_file, const char* nets_file, const char* benchmark);
        void build_HyperGraph();
        HyperGraph* copy_HyperGraph(){
            return this->HG;
        }
        vector<block_node>* copy_Block_Nodes(){
            return this->Block_Nodes;
        }
        vector<string>* copy_Node_id_to_Name(){
            return this->Node_id_to_Name;
        }
        [[deprecated]] void print_net_connections();
        [[deprecated]] void print_block_adjacency();

    private:
        void parse_opencore_soft_blocks();
        void parse_opencore_soft_nets();
        void parse_opencore_hard_blocks();
        void parse_opencore_hard_nets();
        void parse_ispd_blocks();
        void parse_ispd_nets();
    
};

#endif